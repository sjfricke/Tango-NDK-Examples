#ifndef JOB_SCHEDULER_JOB_SCHEDULER_H
#define JOB_SCHEDULER_JOB_SCHEDULER_H

#include <android/log.h>
#include <jni.h>

#include "tango_client_api.h"   // NOLINT
#include "tango_support_api.h"  // NOLINT

#include <cstdlib>
#include <string>

// used to get logcat outputs which can be regex filtered by the LOG_TAG we give
// So in Logcat you can filter this example by putting Tutorial_TAG
#define LOG_TAG "JobScheduler"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void onPoseAvailable(void*, const TangoPoseData* pose);

void OnPointCloudAvailable(void*, const TangoPointCloud* point_cloud);

// used to get last Pose data
double* GetPosition();

// used to get lase PointCloud count
int GetPointCloud();

namespace job {

  // TangoHandler provides functionality to communicate with the Tango Service.
  class JobScheduler {
  public:
    JobScheduler() : tango_config_(nullptr) {}

    JobScheduler(const JobScheduler& other) = delete;

    JobScheduler& operator=(const JobScheduler& other) = delete;

    ~JobScheduler() {
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

  private:
    TangoConfig tango_config_;

    TangoErrorType err;
  };
}  // namespace job

#endif //JOB_SCHEDULER_JOB_SCHEDULER_H
