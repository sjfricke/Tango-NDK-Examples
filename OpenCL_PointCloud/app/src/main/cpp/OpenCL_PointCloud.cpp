#include "OpenCL_PointCloud.h"

// We can set a minimum version of tango for our application
constexpr int kTangoCoreMinimumVersion = 9377;

// prints to logcat the current cloud point data
void onPoseAvailable(void*, const TangoPoseData* pose) {
  LOGI("TEST: %f\t%f\%f", pose->translation[0], pose->translation[1], pose->translation[2]);
}


namespace PC {

  void OpenCL_PointCloud::OnCreate(JNIEnv* env, jobject caller_activity) {
    // Check the installed version of the TangoCore.  If it is too old, then
    // it will not support the most up to date features.
    int version = 0;
    TangoErrorType err =
        TangoSupport_GetTangoVersion(env, caller_activity, &version);

    LOGI("Our Tango Core Version: %d", version );

    if (err != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
      LOGE("UI_Interface::CheckVersion, Tango Core version is out of date.");
      std::exit(EXIT_SUCCESS);
    }
  }

  void OpenCL_PointCloud::OnTangoServiceConnected(JNIEnv* env, jobject iBinder) {
    if (TangoService_setBinder(env, iBinder) != TANGO_SUCCESS) {
      LOGE("UI_Interface::ConnectTango, TangoService_setBinder error");
      std::exit(EXIT_SUCCESS);
    }

    // TANGO_CONFIG_DEFAULT is enabling Motion Tracking and disabling Depth
    // Perception.
    tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
    if (tango_config_ == nullptr) {
      LOGE("UI_Interface::ConnectTango, TangoService_getConfig error.");
      std::exit(EXIT_SUCCESS);
    }

    // TangoCoordinateFramePair is used to tell Tango Service about the frame of
    // references that the applicaion would like to listen to.
    TangoCoordinateFramePair pair;
    pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
    pair.target = TANGO_COORDINATE_FRAME_DEVICE;
    if (TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailable) !=
        TANGO_SUCCESS) {
      LOGE("UI_Interface::ConnectTango, connectOnPoseAvailable error.");
      std::exit(EXIT_SUCCESS);
    }

    if (TangoService_connect(nullptr, tango_config_) != TANGO_SUCCESS) {
      LOGE("UI_Interface::ConnectTango, TangoService_connect error.");
      std::exit(EXIT_SUCCESS);
    }
  }

  void OpenCL_PointCloud::OnPause() {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
    TangoService_disconnect();
  }

  void OpenCL_PointCloud::runOpenCL() {
    const size_t N = 1 << 20;
    try {
      // Get list of OpenCL platforms.
      std::vector<cl::Platform> platform;
      cl::Platform::get(&platform);

      if (platform.empty()) {
        LOGE("OpenCL platforms not found.");
        return;
      }

      // Get first available GPU device which supports double precision.
      cl::Context context;
      std::vector<cl::Device> device;
      for(auto p = platform.begin(); device.empty() && p != platform.end(); p++) {
        std::vector<cl::Device> pldev;

        try {
          p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

          for(auto d = pldev.begin(); device.empty() && d != pldev.end(); d++) {
            if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;

            std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();

            device.push_back(*d);
            context = cl::Context(device);
          }
        } catch(...) {
          device.clear();
        }
      }

      if (device.empty()) {
        LOGE("OpenCL GPUs with double precision not found.");
      }

      LOGI("OpenCL Device: %s", device[0].getInfo<CL_DEVICE_NAME>());

      // Create command queue.
      cl::CommandQueue queue(context, device[0]);

      // Compile OpenCL program for found device.
      cl::Program program(context, cl::Program::Sources(
          1, std::make_pair(source, strlen(source))
      ));

      try {
        program.build(device);
      } catch (const cl::Error&) {
        LOGE("OpenCL compilation error\n %s",program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0]));
      }

      cl::Kernel add(program, "add");

      // Prepare input data.
      std::vector<double> a(N, 1);
      std::vector<double> b(N, 2);
      std::vector<double> c(N);

      // Allocate device buffers and transfer input data to device.
      cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   a.size() * sizeof(double), a.data());

      cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                   b.size() * sizeof(double), b.data());

      cl::Buffer C(context, CL_MEM_READ_WRITE,
                   c.size() * sizeof(double));

      // Set kernel parameters.
      add.setArg(0, static_cast<cl_ulong>(N));
      add.setArg(1, A);
      add.setArg(2, B);
      add.setArg(3, C);

      // Launch kernel on the compute device.
      queue.enqueueNDRangeKernel(add, cl::NullRange, N, cl::NullRange);

      // Get result back to host.
      queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());

      // Should get '3' here.
      LOGI("OpenCL Should be 3: %f", c[42]);
    } catch (const cl::Error &err) {
      LOGE("OpenCL error: %s\n%s", err.what() , err.err());
    }
  }

}  // namespace PC