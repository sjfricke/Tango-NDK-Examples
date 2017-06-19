#include "UI_Interface.h"

// We can set a minimum version of tango for our application
constexpr int kTangoCoreMinimumVersion = 9377;

double* lastPoseData;
long* lastFrameImage;
int lastPointCloud;

void onPoseAvailable(void*, const TangoPoseData* pose)
{
  memcpy(lastPoseData, &(pose->translation), 3*sizeof(double));
  memcpy(lastPoseData + 3, &(pose->orientation), 4*sizeof(double));
}

void OnPointCloudAvailable(void*, const TangoPointCloud* point_cloud)
{
  LOGI("Point count: %d",point_cloud->num_points);
  lastPointCloud = point_cloud->num_points;
}

void OnFrameAvailable(void*, TangoCameraId, const TangoImageBuffer* buffer)
{
  LOGI("Height: %d", buffer->height);
  *(lastFrameImage)     = (long)(buffer->height);
  *(lastFrameImage + 1) = (long)(buffer->width);
  *(lastFrameImage + 2) = (long)(buffer->stride);
  *(lastFrameImage + 3) = (long)(buffer->exposure_duration_ns);
  *(lastFrameImage + 4) = (long)(buffer->frame_number);
}

double* GetPosition() { return lastPoseData; }

int GetPointCloud() { return lastPointCloud; }

long* GetFrameImage() { return lastFrameImage; }


namespace UI {

  void UI_Interface::OnCreate(JNIEnv* env, jobject caller_activity)
  {
    // Check the installed version of the TangoCore.  If it is too old, then
    // it will not support the most up to date features.
    int version = 0;
    TangoErrorType err = TangoSupport_GetTangoVersion(env, caller_activity, &version);

    LOGI("Current Tango Core Version: %d", version);

    if (TANGO_SUCCESS != err || version < kTangoCoreMinimumVersion) {
      LOGE("UI_Interface::CheckVersion, Tango Core version is out of date.");
      std::exit(EXIT_SUCCESS);
    }

    lastPoseData = (double*)malloc(7 * sizeof(double));
    lastFrameImage = (long*)malloc(5 * sizeof(long));

  } //OnCreate

  void UI_Interface::OnTangoServiceConnected(JNIEnv* env, jobject iBinder)
  {
    // First thing is to set the iBinder with the Tango Service
    if (TangoService_setBinder(env, iBinder) != TANGO_SUCCESS) {
      LOGE("OnTangoServiceConnected, TangoService_setBinder error");
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ////// Configure Tango Services Wanted /////
    ////////////////////////////////////////////

    // TANGO_CONFIG_DEFAULT is enabling Motion Tracking and disabling Depth
    // Perception.
    tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
    if (nullptr == tango_config_) {
      LOGE("OnTangoServiceConnected, TangoService_getConfig error.");
      std::exit(EXIT_SUCCESS);
    }

    // Enable color camera from config.
    err = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
    if (TANGO_SUCCESS != err) {
      LOGE( "OnTangoServiceConnected config_enable_color_camera() failed with error code: %d", err);
      std::exit(EXIT_SUCCESS);
    }

    // Enable Depth Perception.
    err = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
    if (TANGO_SUCCESS != err) {
      LOGE("UI_Interface::ConnectTango ,config_enable_depth() failed with error code: %d.", err);
      std::exit(EXIT_SUCCESS);
    }

    // Need to specify the depth_mode as XYZC.
    err = TangoConfig_setInt32(tango_config_, "config_depth_mode",  TANGO_POINTCLOUD_XYZC);
    if (TANGO_SUCCESS != err) {
      LOGE( "Failed to set 'depth_mode' configuration flag with error code: %d", err);
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ///////// Create Callbacks for data ////////
    ////////////////////////////////////////////

    // TangoCoordinateFramePair is used to tell Tango Service about the frame of
    // references that the applicaion would like to listen to.
    TangoCoordinateFramePair pair;
    pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
    pair.target = TANGO_COORDINATE_FRAME_DEVICE;
    err = TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailable);
    if (TANGO_SUCCESS != err) {
      LOGE("OnTangoServiceConnected, connectOnPoseAvailable error code: %d", err);
      std::exit(EXIT_SUCCESS);
    }

    err = TangoService_connectOnPointCloudAvailable(OnPointCloudAvailable);
    if (TANGO_SUCCESS != err) {
      LOGE("OnTangoServiceConnected, connectOnPointCloudAvailable error code: %d", err);
      std::exit(EXIT_SUCCESS);
    }

    err = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this, OnFrameAvailable);
    if (TANGO_SUCCESS != err) {
      LOGE( "OnTangoServiceConnected, Error connecting color frame %d", err);
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ///////////// Time to connect! /////////////
    ////////////////////////////////////////////

    if (TANGO_SUCCESS != TangoService_connect(this, tango_config_)) {
      LOGE("UI_Interface::ConnectTango, TangoService_connect error.");
      std::exit(EXIT_SUCCESS);
    }

    // Initialize TangoSupport context.
    TangoSupport_initializeLibrary();

  } // OnTangoServiceConnected

  void UI_Interface::OnPause() {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
    TangoService_disconnect();
    free(lastPoseData);
    free(lastFrameImage);
  } //OnPause

}  // namespace UI
