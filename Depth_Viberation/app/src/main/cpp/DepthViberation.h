#ifndef DEPTH_VIBERATION_DEPTH_VIBERATION_H
#define DEPTH_VIBERATION_DEPTH_VIBERATION_H

#include <android/log.h>
#include <jni.h>

#include "tango_client_api.h"   // NOLINT
#include "tango_support_api.h"  // NOLINT

#include <cstdlib>
#include <unistd.h>

#include <string>
#include <vector>

// used to get logcat outputs which can be regex filtered by the LOG_TAG we give
// So in Logcat you can filter this example by putting Tutorial_TAG
#define LOG_TAG "DepthViberation"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void OnPointCloudAvailableRouter(void*, const TangoPointCloud* point_cloud);

// TangoHandler provides functionality to communicate with the Tango Service.
class DepthViberation {
public:
  DepthViberation() : tango_config_(nullptr) {}

  DepthViberation(const DepthViberation& other) = delete;

  DepthViberation& operator=(const DepthViberation& other) = delete;

  ~DepthViberation() {
    if (tango_config_ != nullptr) {
      TangoConfig_free(tango_config_);
      tango_config_ = nullptr;
    }
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

  void OnDestroy();

  // Cache the Java VM
  //
  // @JavaVM java_vm: the Java VM is using from the Java layer.
  void SetJavaVM(JavaVM* java_vm) { java_vm_ = java_vm; }

  // Tango Service point cloud callback function for depth data.
  // Called when new new point cloud data is available from the Tango Service.
  void OnPointCloudAvailable(const TangoPointCloud* point_cloud);

private:
  TangoConfig tango_config_;

  TangoErrorType err;

  // Cached Java VM, caller activity object
  JavaVM* java_vm_;
  jobject calling_activity_obj_;
  jmethodID on_demand_method_;

  int callback_delay_count = 0;

};
#endif //DEPTH_VIBERATION_DEPTH_VIBERATION_H
