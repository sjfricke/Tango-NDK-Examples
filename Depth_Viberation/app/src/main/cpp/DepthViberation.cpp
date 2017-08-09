#include "DepthViberation.h"

// We can set a minimum version of tango for our application
constexpr int kTangoCoreMinimumVersion = 9377;

// Need a router since the callback is out of scope of the class instances
void OnPointCloudAvailableRouter(void* context, const TangoPointCloud* point_cloud) {
  static_cast<DepthViberation*>(context)->OnPointCloudAvailable(point_cloud);
}

void DepthViberation::OnCreate(JNIEnv* env, jobject caller_activity)
{
  // Check the installed version of the TangoCore.  If it is too old, then
  // it will not support the most up to date features.
  int version = 0;
  err = TangoSupport_GetTangoVersion(env, caller_activity, &version);

  LOGI("Current Tango Core Version: %d", version);

  if (TANGO_SUCCESS != err || version < kTangoCoreMinimumVersion) {
    LOGE("Tango Core version is out of date.");
    std::exit(EXIT_SUCCESS);
  }

  // Need to create an instance of the Java activity
  calling_activity_obj_ = env->NewGlobalRef(caller_activity);

  // Need to enter package and class to find Java class
  // If this is the same class as the calling_activity then you can just use
  // jclass handlerClass = env->GetObjectClass(caller_activity);
  jclass handlerClass = env->FindClass("com/spencerfricke/depth_viberation/MainActivity");

  // Here you need to name the function and the JNI argument/parameter type
  on_demand_method_ = env->GetMethodID(handlerClass, "pulse", "(I)V");

} //OnCreate

void DepthViberation::OnTangoServiceConnected(JNIEnv* env, jobject iBinder)
{
  // First thing is to set the iBinder with the Tango Service
  if (TangoService_setBinder(env, iBinder) != TANGO_SUCCESS) {
    LOGE("TangoService_setBinder error");
    std::exit(EXIT_SUCCESS);
  }

  ////////////////////////////////////////////
  ////// Configure Tango Services Wanted /////
  ////////////////////////////////////////////

  // TANGO_CONFIG_DEFAULT is enabling Motion Tracking and disabling Depth
  // Perception.
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (nullptr == tango_config_) {
    LOGE("TangoService_getConfig error.");
    std::exit(EXIT_SUCCESS);
  }

  // Enable Depth Perception.
  err = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (TANGO_SUCCESS != err) {
    LOGE("config_enable_depth() configuration - failed with error code: %d.", err);
    std::exit(EXIT_SUCCESS);
  }

  // Need to specify the depth_mode as XYZC.
  err = TangoConfig_setInt32(tango_config_, "config_depth_mode",  TANGO_POINTCLOUD_XYZC);
  if (TANGO_SUCCESS != err) {
    LOGE( "'depth_mode' configuration - failed with error code: %d", err);
    std::exit(EXIT_SUCCESS);
  }

  ////////////////////////////////////////////
  ///////// Create Callbacks for data ////////
  ////////////////////////////////////////////

  err = TangoService_connectOnPointCloudAvailable(OnPointCloudAvailableRouter);
  if (TANGO_SUCCESS != err) {
    LOGE("connectOnPointCloudAvailable error code: %d", err);
    std::exit(EXIT_SUCCESS);
  }

  ////////////////////////////////////////////
  ///////////// Time to connect! /////////////
  ////////////////////////////////////////////

  if (TANGO_SUCCESS != TangoService_connect(this, tango_config_)) {
    LOGE("TangoService_connect error.");
    std::exit(EXIT_SUCCESS);
  }

  // Initialize TangoSupport context.
  TangoSupport_initializeLibrary();

} // OnTangoServiceConnected

void DepthViberation::OnPause() {
  TangoConfig_free(tango_config_);
  tango_config_ = nullptr;
  TangoService_disconnect();
} //OnPause

void DepthViberation::OnDestroy() {
  JNIEnv* env;
  java_vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  env->DeleteGlobalRef(calling_activity_obj_);

  calling_activity_obj_ = nullptr;
  on_demand_method_ = nullptr;
}

void DepthViberation::OnPointCloudAvailable(const TangoPointCloud* point_cloud)
{
  // Calculate the average depth.
  float average_depth = 0;
  int count = 0;
  float confidence_threshold = 0.4f;

  // scans each point and sums up those that are in confidence range
  for (size_t i = 0; i < point_cloud->num_points; ++i) {
    if (point_cloud->points[i][3] > confidence_threshold) {
      average_depth += point_cloud->points[i][2];
      count++;
    }
  }

  if (point_cloud->num_points) {
    average_depth /= count;
  }

  // Log the number of points and average depth.
  LOGI("Point count: %d (count: %d). Average depth (m): %.3f",
       point_cloud->num_points, count, average_depth);

  // Need to set a delay in how often we send a callback to JNI since
  // it will start the viberation too often
  if (callback_delay_count > 5) {
    callback_delay_count = 0;

    if (calling_activity_obj_ == nullptr || on_demand_method_ == nullptr) {
      LOGE("Can not reference Activity to request pulsing");
      return;
    }

    jint viberation_rate = static_cast<jint>((average_depth * 400) - 50);
    if (viberation_rate < 0) { viberation_rate = 0; };

    // Here, we notify the Java activity that we'd like it to trigger a callback.
    JNIEnv *env;
    java_vm_->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    env->CallVoidMethod(calling_activity_obj_, on_demand_method_, viberation_rate);

  } else {
    callback_delay_count++;
  }
}
