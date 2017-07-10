#ifndef RGBA_DATA_RGBA_DATA_H
#define RGBA_DATA_RGBA_DATA_H

#include <memory>
#include <vector>
#include <cstdlib>
#include <stdlib.h>

#include <tango_client_api.h>  // NOLINT
#include <tango_support_api.h>

#include <jni.h>
#include <android/log.h>

// used to get logcat outputs which can be regex filtered by the LOG_TAG we give
// So in Logcat you can filter this example by putting Tutorial_TAG
#define LOG_TAG "RGBAData"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define SYSTEM_BITS_IN_BYTES  (sizeof(void*))
#define LOOPS_PER_LONG	(SYSTEM_BITS_IN_BYTES/2)
#define UCHAR_RANGE(x)	((unsigned long)uchar_range_tbl[x+300])

namespace rgba {

// TangoHandler provides functionality to communicate with the Tango Service.
class RGBAData {
public:
  RGBAData() : tango_config_(nullptr) {}

  RGBAData(const RGBAData& other) = delete;
  RGBAData& operator=(const RGBAData& other) = delete;

  ~RGBAData() {
    if (tango_config_ != nullptr) {
      TangoConfig_free(tango_config_);
      tango_config_ = nullptr;
    }

    if (rgba_buffer != nullptr) {
      free(rgba_buffer);
      rgba_buffer = nullptr;
    }

    TangoSupport_freeImageBufferManager(image_buffer_manager_);
    image_buffer_manager_ = nullptr;

  }

  // Check if the Tango Core version is compatible with this app.
  // If not, the application will exit.
  //
  // @param env, java environment parameter CheckVersion is being called.
  // @param caller_activity, caller of this function.
  void OnCreate(JNIEnv* env, jobject caller_activity);

  // Called when the Tango service is connect. We set the binder object to Tango
  // Service in this function.
  //
  // @param env, java environment parameter.
  // @param iBinder, the native binder object.
  void OnTangoServiceConnected(JNIEnv* env, jobject iBinder);

  // Disconnect and stop Tango service.
  void OnPause();

  void OnFrameAvailable(const TangoImageBuffer* buffer);

  const char* SavePNG(const char* directory_path);

  int SendPNG(const char* serverIP);

  int nv21_to_rgba(unsigned char *src_buf,unsigned char *rgb_buf,int width,int height);

private:
  TangoConfig tango_config_;

  TangoCameraIntrinsics color_camera_intrinsics_;

  TangoErrorType err;

  // Image data manager.
  TangoSupportImageBufferManager* image_buffer_manager_;

  uint8_t* rgba_buffer;
};
} //NAMESPACE rgba

#endif // RGBA_DATA_RGBA_DATA_H
