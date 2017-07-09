#include "DracoPly.h"

// We can set a minimum version of tango for our application
constexpr int kTangoCoreMinimumVersion = 9377;

// Need a router since the callback is out of scope of the class instances
void OnPointCloudAvailableRouter(void* context, const TangoPointCloud* point_cloud) {
  static_cast<draco::DracoPly*>(context)->OnPointCloudAvailable(point_cloud);
}

namespace draco {

  void DracoPly::OnCreate(JNIEnv* env, jobject caller_activity)
  {
    // Check the installed version of the TangoCore.  If it is too old, then
    // it will not support the most up to date features.
    int version = 0;
    err = TangoSupport_GetTangoVersion(env, caller_activity, &version);

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

    // Setup Point Cloud Manager
    if (point_cloud_manager_ == nullptr) {
      int32_t max_point_cloud_elements;
      err = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements", &max_point_cloud_elements);
      if (err != TANGO_SUCCESS) {
        LOGE("Failed to query maximum number of point cloud elements.");
        std::exit(EXIT_SUCCESS);
      }
      LOGI("Max Point Cloud elements supportd: %d", max_point_cloud_elements);

      err = TangoSupport_createPointCloudManager(max_point_cloud_elements, &point_cloud_manager_);
      if (err != TANGO_SUCCESS) {
        std::exit(EXIT_SUCCESS);
      }
    }

    ////////////////////////////////////////////
    ///////// Create Callbacks for data ////////
    ////////////////////////////////////////////

    err = TangoService_connectOnPointCloudAvailable(OnPointCloudAvailableRouter);
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

  void DracoPly::OnPointCloudAvailable(const TangoPointCloud* point_cloud)
  {
    TangoSupport_updatePointCloud(point_cloud_manager_, point_cloud);
    if (err != TANGO_SUCCESS) {
      LOGE("TangoSupport_updatePointCloud - Error: %d", err);
      std::exit(EXIT_SUCCESS);
    }

  }

  const char* DracoPly::SavePointCloudToPly(int frames, const char* directory) {

    static char file_path[256];
    strcpy(file_path, directory);
    strcat(file_path,"/tango.ply");
    LOGI("file_path = %s", file_path);

    point_cloud_vector.resize(frames);
    GetPointCloud(point_cloud_vector.at(0));

    uint32_t totalSize = 0;
    err = TangoSupport_getLatestPointCloud(point_cloud_manager_, &point_cloud_vector.at(0));
    if (TANGO_SUCCESS != err) {
      LOGE("TangoSupport_getLatestPointCloud - error code: %d", err);
    }
    totalSize += point_cloud_vector.at(0)->num_points;
//    for (auto point_cloud : point_cloud_vector) {
//      totalSize += point_cloud->num_points;
//    }

    if( access( file_path, F_OK ) != -1 ) {
      // file exists so clear it first since we will want appending for actual writes
      fclose(fopen(file_path, "w"));
    }

    FILE* file = fopen(file_path, "a");
    if (NULL == file) {
      static char errorString[128];
      sprintf(errorString, "Could Not Open File at %s", file_path);
      return errorString;
    }

    fprintf(file, "ply\n"
        "format ascii 1.0\n"
        "element vertex %u\n"
        "property float32 x\n"
        "property float32 y\n"
        "property float32 z\n"
        "end_header\n", totalSize);

    // time to write in the body
    for (auto point_cloud : point_cloud_vector) {
      for (uint32_t i = 0; i < point_cloud->num_points; i++) {
        if (point_cloud->points[i][3] > 0.5f) {
          fprintf(file, "%.3f %.3f %.3f\n",
                  point_cloud->points[i][0],
                  point_cloud->points[i][1],
                  point_cloud->points[i][2]);
        }
      }
    }

    fclose(file);
    return file_path;
  }

  void DracoPly::GetPointCloud(TangoPointCloud* point_cloud) {


  }

}  // namespace draco
