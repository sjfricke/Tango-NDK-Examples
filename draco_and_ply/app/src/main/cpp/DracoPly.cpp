#include "DracoPly.h"

// We can set a minimum version of tango for our application
constexpr int kTangoCoreMinimumVersion = 9377;

int lastPointCloud;

void OnPointCloudAvailable(void*, const TangoPointCloud* point_cloud)
{
  LOGI("TEST: %d", point_cloud->num_points);
  lastPointCloud = point_cloud->num_points;
}

int GetPointCloud() { return lastPointCloud; }

namespace draco {

  void DracoPly::OnCreate(JNIEnv* env, jobject caller_activity)
  {
    // Check the installed version of the TangoCore.  If it is too old, then
    // it will not support the most up to date features.
    int version = 0;
    TangoErrorType err = TangoSupport_GetTangoVersion(env, caller_activity, &version);

    LOGI("Current Tango Core Version: %d", version);

    if (TANGO_SUCCESS != err || version < kTangoCoreMinimumVersion) {
      LOGE("DracoPly::CheckVersion, Tango Core version is out of date.");
      std::exit(EXIT_SUCCESS);
    }

  } //OnCreate

  void DracoPly::OnTangoServiceConnected(JNIEnv* env, jobject iBinder)
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

    // Enable Depth Perception.
    err = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
    if (TANGO_SUCCESS != err) {
      LOGE("OnTangoServiceConnected ,config_enable_depth() failed with error code: %d.", err);
      std::exit(EXIT_SUCCESS);
    }

    // Need to specify the depth_mode as XYZC.
    err = TangoConfig_setInt32(tango_config_, "config_depth_mode",  TANGO_POINTCLOUD_XYZC);
    if (TANGO_SUCCESS != err) {
      LOGE( "OnTangoServiceConnected, 'depth_mode' configuration flag with error code: %d", err);
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ///////// Create Callbacks for data ////////
    ////////////////////////////////////////////

    err = TangoService_connectOnPointCloudAvailable(OnPointCloudAvailable);
    if (TANGO_SUCCESS != err) {
      LOGE("OnTangoServiceConnected, connectOnPointCloudAvailable error code: %d", err);
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

  void DracoPly::OnPause() {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
    TangoService_disconnect();
  } //OnPause

}  // namespace draco
