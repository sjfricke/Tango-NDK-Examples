#include <jni.h>

#include "OpenCL_PointCloud.h"

static PC::OpenCL_PointCloud app;

#ifdef __cplusplus
extern "C" {
#endif

/* Format:
 JNIEXPORT < Return_Type > JNICALL
 < Package_Name + Java_Class_Name + Function_Name > (
    JNIEnv* env, jobject obj, < Parameter_of_Native_Function > , ...
 )
*/

JNIEXPORT void JNICALL
Java_com_spencerfricke_opencl_1pointcloud_TangoJniNative_onCreate(
    JNIEnv* env, jobject, jobject caller_activity) {
  app.OnCreate(env, caller_activity);
}

JNIEXPORT void JNICALL
Java_com_spencerfricke_opencl_1pointcloud_TangoJniNative_onTangoServiceConnected(
    JNIEnv* env, jobject, jobject iBinder) {
  app.OnTangoServiceConnected(env, iBinder);
  runOpenCL();
}

JNIEXPORT void JNICALL
Java_com_spencerfricke_opencl_1pointcloud_TangoJniNative_onPause(
    JNIEnv*, jobject) {
  app.OnPause();
}

#ifdef __cplusplus
}
#endif
