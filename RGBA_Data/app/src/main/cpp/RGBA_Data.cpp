#include <tango_support_api.h>
#include <cstdlib>

#include "RGBA_Data.h"

namespace {
// We can set a minimum version of tango for our application
constexpr int kTangoCoreMinimumVersion = 9377;

void OnFrameAvailableRouter(void *context, TangoCameraId,
                            const TangoImageBuffer *buffer) {
  rgba::RGBAData *app = static_cast<rgba::RGBAData *>(context);
  app->OnFrameAvailable(buffer);
}
} //namespace

namespace rgba {
  void RGBAData::OnCreate(JNIEnv* env, jobject caller_activity)
  {
    // Check the installed version of the TangoCore.  If it is too old, then
    // it will not support the most up to date features.
    int version = 0;
    TangoErrorType err = TangoSupport_GetTangoVersion(env, caller_activity, &version);

    LOGI("Current Tango Core Version: %d", version);

    if (TANGO_SUCCESS != err || version < kTangoCoreMinimumVersion) {
      LOGE("RGBAData::CheckVersion, Tango Core version is out of date.");
      std::exit(EXIT_SUCCESS);
    }

  } //OnCreate


  void RGBAData::OnTangoServiceConnected(JNIEnv* env, jobject iBinder)
  {
    // First thing is to set the iBinder with the Tango Service
    if (TangoService_setBinder(env, iBinder) != TANGO_SUCCESS) {
      LOGE("OnTangoServiceConnected, TangoService_setBinder error");
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ////// Configure Tango Services Wanted /////
    ////////////////////////////////////////////

    tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
    if (tango_config_ == nullptr) {
      LOGE(
          "HelloVideoApp::OnTangoServiceConnected,"
              "Failed to get default config form");
      std::exit(EXIT_SUCCESS);
    }

    // Enable color camera from config.
    int ret =
        TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "HelloVideoApp::OnTangoServiceConnected,"
              "config_enable_color_camera() failed with error code: %d",
          ret);
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ///////// Create Callbacks for data ////////
    ////////////////////////////////////////////
    ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                               OnFrameAvailableRouter);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "HelloVideoApp::OnTangoServiceConnected,"
              "Error connecting color frame %d",
          ret);
      std::exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////////
    ///////////// Time to connect! /////////////
    ////////////////////////////////////////////

    // Connect to Tango Service, service will start running, and
    // pose can be queried.
    ret = TangoService_connect(this, tango_config_);
    if (ret != TANGO_SUCCESS) {
      LOGE(
          "HelloVideoApp::OnTangoServiceConnected,"
              "Failed to connect to the Tango service with error code: %d",
          ret);
      std::exit(EXIT_SUCCESS);
    }

    // Initialize TangoSupport context.
    TangoSupport_initializeLibrary();

  } // OnTangoServiceConnected

  void RGBAData::OnPause() {
    if (tango_config_ != nullptr) {
      TangoConfig_free(tango_config_);
      tango_config_ = nullptr;
    }
    TangoService_disconnect();
  } //OnPause

  void RGBAData::OnFrameAvailable(const TangoImageBuffer* buffer) {
    LOGI("GETTING FRAME: %d (Frame: %ld)", buffer->height, (long)buffer->frame_number);
  }

}