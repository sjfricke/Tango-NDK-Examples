#ifndef DRACO_PLY_DRACO_PLY_H
#define DRACO_PLY_DRACO_PLY_H

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
#define LOG_TAG "DracoPly"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void OnPointCloudAvailable(void*, const TangoPointCloud* point_cloud);

namespace draco {

  // TangoHandler provides functionality to communicate with the Tango Service.
  class DracoPly {
  public:
    DracoPly() : tango_config_(nullptr) {}

    DracoPly(const DracoPly& other) = delete;

    DracoPly& operator=(const DracoPly& other) = delete;

    ~DracoPly() {
      if (tango_config_ != nullptr) {
        TangoConfig_free(tango_config_);
        tango_config_ = nullptr;
      }
      if (point_cloud_manager_ != nullptr) {
        TangoSupport_freePointCloudManager(point_cloud_manager_);
        point_cloud_manager_ = nullptr;
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

    // Tango Service point cloud callback function for depth data.
    // Called when new new point cloud data is available from the Tango Service.
    void OnPointCloudAvailable(const TangoPointCloud* point_cloud);

    const char* SavePointCloudToPly(int frames, const char* directory);

    // used to get PointCloud file
    void GetPointCloud(TangoPointCloud* point_cloud);

  private:
    TangoConfig tango_config_;

    TangoErrorType err;

    // Point data manager to hold state
    TangoSupportPointCloudManager* point_cloud_manager_;

    std::vector<TangoPointCloud*> point_cloud_vector;
  };
}  // namespace draco

#endif //DRACO_PLY_DRACO_PLY_H
