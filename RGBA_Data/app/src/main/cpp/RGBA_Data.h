#ifndef RGBA_DATA_RGBA_DATA_H
#define RGBA_DATA_RGBA_DATA_H

#include <atomic>
#include <jni.h>
#include <memory>
#include <mutex>
#include <vector>

#include <tango_client_api.h>  // NOLINT


#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>

#include <tango_support_api.h>

// used to get logcat outputs which can be regex filtered by the LOG_TAG we give
// So in Logcat you can filter this example by putting Tutorial_TAG
#define LOG_TAG "RGBAData"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace rgba {

// TangoHandler provides functionality to communicate with the Tango Service.
class RGBAData {
public:
//  RGBAData() : tango_config_(nullptr) {}
//
//  RGBAData(const RGBAData& other) = delete;
//
//  RGBAData& operator=(const RGBAData& other) = delete;
//
//  ~RGBAData() {
//    if (tango_config_ != nullptr) {
//      TangoConfig_free(tango_config_);
//      tango_config_ = nullptr;
//    }
//  }

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


private:
  TangoConfig tango_config_;

  TangoErrorType err;
};
} //NAMESPACE rgba

#endif // RGBA_DATA_RGBA_DATA_H
