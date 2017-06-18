#ifndef OPENCL_POINTCLOUD_OPENCL_POINTCLOUD_H
#define OPENCL_POINTCLOUD_OPENCL_POINTCLOUD_H

#include <android/log.h>
#include <jni.h>

#include "tango_client_api.h"   // NOLINT
#include "tango_support_api.h"  // NOLINT
#include <CL/cl.hpp>

#include <cstdlib>
#include <vector>
#include <string>

// used to get logcat outputs which can be regex filtered by the LOG_TAG we give
// So in Logcat you can filter this example by putting Tutorial_TAG
#define LOG_TAG "OpenCL_PC"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define __CL_ENABLE_EXCEPTIONS

namespace PC {

  // TangoHandler provides functionality to communicate with the Tango Service.
  class OpenCL_PointCloud {
  public:
    OpenCL_PointCloud() : tango_config_(nullptr) {}

    OpenCL_PointCloud(const OpenCL_PointCloud& other) = delete;

    OpenCL_PointCloud& operator=(const OpenCL_PointCloud& other) = delete;

    ~OpenCL_PointCloud() {
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

    void runOpenCL();

  private:
    TangoConfig tango_config_;

    // Compute c = a + b.
    static const char source[] =
            "kernel void add(\n"
            "       ulong n,\n"
            "       global const double *a,\n"
            "       global const double *b,\n"
            "       global double *c\n"
            "       )\n"
            "{\n"
            "    size_t i = get_global_id(0);\n"
            "    if (i < n) {\n"
            "       c[i] = a[i] + b[i];\n"
            "    }\n"
            "}\n";
  };

}  // namespace PC


#endif //OPENCL_POINTCLOUD_OPENCL_POINTCLOUD_H
