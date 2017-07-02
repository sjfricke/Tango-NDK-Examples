#include <jni.h>
#include "JobScheduler.h"

static job::JobScheduler app;

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
Java_com_spencerfricke_jobscheduler_TangoJniNative_onCreate(
    JNIEnv* env, jobject, jobject caller_activity) {
  app.OnCreate(env, caller_activity);
}

JNIEXPORT void JNICALL
Java_com_spencerfricke_jobscheduler_TangoJniNative_onTangoServiceConnected(
    JNIEnv* env, jobject, jobject iBinder) {
  app.OnTangoServiceConnected(env, iBinder);
}

JNIEXPORT void JNICALL
Java_com_spencerfricke_jobscheduler_TangoJniNative_onPause(
    JNIEnv*, jobject) {
  app.OnPause();
}

JNIEXPORT jdoubleArray JNICALL
Java_com_spencerfricke_jobscheduler_TangoJniNative_getPosition(
    JNIEnv* env, jobject) {

  jdoubleArray result;
  result = env->NewDoubleArray(7);
  if (result == NULL) {
    return NULL; /* out of memory error thrown */
  }

  double* poseData = GetPosition();

  // fill a temp structure to use to populate the java double array
  jdouble fill[7] = {
      poseData[0],
      poseData[1],
      poseData[2],
      poseData[3],
      poseData[4],
      poseData[5],
      poseData[6],
  };

  // move from the temp structure to the java structure
  env->SetDoubleArrayRegion(result, 0, 7, fill);
  return result;
}

JNIEXPORT jint JNICALL
Java_com_spencerfricke_jobscheduler_TangoJniNative_getPointCloud(
    JNIEnv* env, jobject) {

  jint result = GetPointCloud();
  return result;
}

#ifdef __cplusplus
}
#endif
