#include "RGBA_Data.h"

namespace {
// We can set a minimum version of tango for our application
constexpr int kTangoCoreMinimumVersion = 9377;

void OnFrameAvailableRouter(void *context, TangoCameraId,
                            const TangoImageBuffer *buffer) {
  rgba::RGBAData *app = static_cast<rgba::RGBAData *>(context);
  app->OnFrameAvailable(buffer);
}
} //namespace

namespace rgba {
  void RGBAData::OnCreate(JNIEnv* env, jobject caller_activity)
  {
    // Check the installed version of the TangoCore.  If it is too old, then
    // it will not support the most up to date features.
    int version = 0;
    TangoErrorType err = TangoSupport_GetTangoVersion(env, caller_activity, &version);

    LOGI("Current Tango Core Version: %d", version);

    if (TANGO_SUCCESS != err || version < kTangoCoreMinimumVersion) {
      LOGE("Tango Core version is out of date.");
      std::exit(EXIT_SUCCESS);
    }

  } //OnCreate


  void RGBAData::OnTangoServiceConnected(JNIEnv* env, jobject iBinder)
  {
    // First thing is to set the iBinder with the Tango Service
    if (TangoService_setBinder(env, iBinder) != TANGO_SUCCESS) {
      LOGE("TangoService_setBinder error");
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ////// Configure Tango Services Wanted /////
    ////////////////////////////////////////////

    tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
    if (tango_config_ == nullptr) {
      LOGE("Failed to get TANGO_CONFIG_DEFAULT");
      std::exit(EXIT_SUCCESS);
    }

    // Enable color camera from config.
    int err = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
    if (err != TANGO_SUCCESS) {
      LOGE("config_enable_color_camera() failed with error code: %d", err);
      std::exit(EXIT_SUCCESS);
    }

    err = TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR, &color_camera_intrinsics_);
    if (err != TANGO_SUCCESS) {
      LOGE("Failed to get the intrinsics for the color camera.");
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ///////// Create Callbacks for data ////////
    ////////////////////////////////////////////

     err = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this, OnFrameAvailableRouter);
    if (err != TANGO_SUCCESS) {
      LOGE("Error connecting color frame %d", err);
      std::exit(EXIT_SUCCESS);
    }

    // The image_buffer_manager_ contains the image data and allows reading and
    // writing of data in a thread safe way.
    if (!image_buffer_manager_) {
      err = TangoSupport_createImageBufferManager(
          TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP, color_camera_intrinsics_.width,
          color_camera_intrinsics_.height, &image_buffer_manager_);

      if (err != TANGO_SUCCESS) {
        LOGE("Failed to create image buffer manager");
        std::exit(EXIT_SUCCESS);
      }
    }

    ////////////////////////////////////////////
    ///////////// Time to connect! /////////////
    ////////////////////////////////////////////

    // Connect to Tango Service, service will start running, and
    // pose can be queried.
    err = TangoService_connect(this, tango_config_);
    if (err != TANGO_SUCCESS) {
      LOGE("Failed to connect to the Tango service with error code: %d", err);
      std::exit(EXIT_SUCCESS);
    }

    // Initialize TangoSupport context.
    TangoSupport_initializeLibrary();

    // Height * Width * 4 (4 is since its rgba)
    // need an extra width allocated for conversion algorithm
    rgba_buffer = (uint8_t*)calloc(sizeof(uint8_t), (color_camera_intrinsics_.width+1) * color_camera_intrinsics_.height * 4);

  } // OnTangoServiceConnected

  void RGBAData::OnPause() {
    if (tango_config_ != nullptr) {
      TangoConfig_free(tango_config_);
      tango_config_ = nullptr;
    }
    TangoService_disconnect();
  } //OnPause

  void RGBAData::OnFrameAvailable(const TangoImageBuffer* buffer) {
    TangoSupport_updateImageBuffer(image_buffer_manager_, buffer);
  }

  const char* RGBAData::SavePNG(const char* directory_path) {
    // Get the latest color image
    TangoImageBuffer* image = nullptr;
    TangoSupport_getLatestImageBuffer(image_buffer_manager_, &image);

    // need to convert to RGBA color space to use easier
    nv21_to_rgba(image->data, rgba_buffer, image->width, image->height);

    static char file_path[256];
    char fileName[16];
    sprintf(fileName, "/frame%ld.png", (long)image->frame_number);

    strcpy(file_path, directory_path);
    strcat(file_path, fileName);

    // This is a single line lodepng library function
    // will take rgba buffer and turn to png file and save to path
    unsigned error = lodepng_encode32_file(file_path, rgba_buffer, image->width, image->height);

    return file_path;
  }

  int RGBAData::SendPNG(const char* serverIP) {

    // TODO
    // Get the latest color image
    TangoImageBuffer* image = nullptr;
    TangoSupport_getLatestImageBuffer(image_buffer_manager_, &image);

    // need to convert to RGBA color space to use easier
    nv21_to_rgba(image->data, rgba_buffer, image->width, image->height);

    unsigned char* png;
    size_t pngsize;

    // This is a single line lodepng library function
    // will take rgba buffer and turn to png file, doesn't write it out
    unsigned error = lodepng_encode32(&png, &pngsize, rgba_buffer, image->width, image->height);

    // TODO
    // Add support to send to server

    return 0;
  }

  static int yuv_tbl_ready_flag=0;
  static int loop_times_once=2;
  static int y1192_tbl[256];
  static int v1634_tbl[256];
  static int v833_tbl[256];
  static int u400_tbl[256];
  static int u2066_tbl[256];
  static unsigned char uchar_range_tbl[1000];

  int RGBAData::nv21_to_rgba(unsigned char *src_buf,unsigned char *rgb_buf,int width,int height)
  {
    int frame_size =width*height;
    unsigned long *src_y,*src_vu;
    int i,j;

    if((!rgb_buf)||(!src_buf)){
      return -1;
    }

    src_y = (unsigned long *)src_buf;
    src_vu = (unsigned long *)(src_buf + frame_size);

    int ii;
    if(0 == yuv_tbl_ready_flag){
      for(ii=0; ii<256; ii++){
        y1192_tbl[ii] = 1192*(ii-16);
        if(y1192_tbl[ii] < 0){
          y1192_tbl[ii] = 0;
        }
        v1634_tbl[ii] = 1634*(ii-128);
        v833_tbl[ii] = 833*(ii-128);
        u400_tbl[ii] = 400*(ii-128);
        u2066_tbl[ii] = 2066*(ii-128);
      }
      //-276~534,set tbl offset to 300
      for(ii=0; ii<1000; ii++){
        if(ii<300)
          uchar_range_tbl[ii] = 0;
        else if(ii>555)
          uchar_range_tbl[ii] = 255;
        else
          uchar_range_tbl[ii] = ii-300;
        //printf("%d,",uchar_range_tbl[i]);
      }
      yuv_tbl_ready_flag = 1;
    }

    int cnt_long = 0;
    int longs_per_line = width/SYSTEM_BITS_IN_BYTES;
    unsigned long y1 = *(src_y+cnt_long);
    unsigned long y2 = *(src_y+cnt_long+longs_per_line);
    unsigned long vu = *(src_vu+cnt_long);

    unsigned char *rgb1 = rgb_buf;
    unsigned char *rgb2 = rgb1+4*width;

    //process 2 lines X 1 long size once
    for(i=0; i<frame_size; i+=2*SYSTEM_BITS_IN_BYTES){
      cnt_long++;
      //if reached the end of the line
      if(cnt_long >= longs_per_line)
      {
        //src buf skip to next two line
        src_y += 2*longs_per_line;
        src_vu += longs_per_line;

        //dst buf skip to next two line
        rgb1 += 4*width;
        rgb2 = rgb1+4*width;

        cnt_long = 0;
      }
      //read src buf per long size
      y1 = *(src_y+cnt_long);
      y2 = *(src_y+cnt_long+longs_per_line);
      vu = *(src_vu+cnt_long);

      //convert one long size per loop
      for(j=0; j<LOOPS_PER_LONG; j+=1){
        unsigned char y11,y12,y21,y22,u,v;
        int k = j<<4;
        y11 = (y1>>k)&0xff;
        y12 = (y1>>(k+8))&0xff;
        y21 = (y2>>k)&0xff;;
        y22 = (y2>>(k+8))&0xff;

        u = (vu>>k)&0xff;
        v = (vu>>(k+8))&0xff;

        int y1192_11=y1192_tbl[y11];
        int b11 = (y1192_11 + v1634_tbl[v])>>10;
        int g11 = (y1192_11 - v833_tbl[v] - u400_tbl[u])>>10;
        int r11 = (y1192_11 + u2066_tbl[u])>>10;

        int y1192_12=y1192_tbl[y12];
        int b12 = (y1192_12 + v1634_tbl[v])>>10;
        int g12 = (y1192_12 - v833_tbl[v] - u400_tbl[u])>>10;
        int r12 = (y1192_12 + u2066_tbl[u])>>10;

        int y1192_21=y1192_tbl[y21];
        int b21 = (y1192_21 + v1634_tbl[v])>>10;
        int g21 = (y1192_21 - v833_tbl[v] - u400_tbl[u])>>10;
        int r21 = (y1192_21 + u2066_tbl[u])>>10;

        int y1192_22=y1192_tbl[y22];
        int b22 = (y1192_22 + v1634_tbl[v])>>10;
        int g22 = (y1192_22 - v833_tbl[v] - u400_tbl[u])>>10;
        int r22 = (y1192_22 + u2066_tbl[u])>>10;

        *rgb1++ = UCHAR_RANGE(r11);
        *rgb1++ = UCHAR_RANGE(g11);
        *rgb1++ = UCHAR_RANGE(b11);
        *rgb1++ = 0xff;

        *rgb1++ = UCHAR_RANGE(r12);
        *rgb1++ = UCHAR_RANGE(g12);
        *rgb1++ = UCHAR_RANGE(b12);
        *rgb1++ = 0xff;

        *rgb2++ = UCHAR_RANGE(r21);
        *rgb2++ = UCHAR_RANGE(g21);
        *rgb2++ = UCHAR_RANGE(b21);
        *rgb2++ = 0xff;

        *rgb2++ = UCHAR_RANGE(r22);
        *rgb2++ = UCHAR_RANGE(g22);
        *rgb2++ = UCHAR_RANGE(b22);
        *rgb2++ = 0xff;
      }
    }
    return 0;
  }


}