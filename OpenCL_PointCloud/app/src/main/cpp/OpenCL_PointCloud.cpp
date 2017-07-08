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



}  // namespace PC

void runOpenCL() {
    // Length of vectors
    unsigned int n = 100000;

    static constexpr char source[] =
            "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
                    "__kernel void vecAdd(  __global double *a,\n"
                    "                       __global double *b,\n"
                    "                       __global double *c,\n"
                    "                       const unsigned int n)\n"
                    "{\n"
                    "    int id = get_global_id(0);\n"
                    "    if (id < n) {\n"
                    "        c[id] = a[id] + b[id];\n"
                    "    }\n"
                    "}\n";
    // Host input vectors
    double *h_a;
    double *h_b;
    // Host output vector
    double *h_c;

    // Device input buffers
    cl_mem d_a;
    cl_mem d_b;
    // Device output buffer
    cl_mem d_c;

    cl_platform_id cpPlatform;        // OpenCL platform
    cl_device_id device_id;           // device ID
    cl_context context;               // context
    cl_command_queue queue;           // command queue
    cl_program program;               // program
    cl_kernel kernel;                 // kernel

    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(double);

    // Allocate memory for each vector on host
    h_a = (double*)malloc(bytes);
    h_b = (double*)malloc(bytes);
    h_c = (double*)malloc(bytes);

    // Initialize vectors on host
    int i;
    for( i = 0; i < n; i++ )
    {
        h_a[i] = 0 + i;
        h_b[i] = 0 - i;
    }

    size_t globalSize, localSize;
    cl_int err;

    // Number of work items in each local work group
    localSize = 64;

    // Number of total work items - localSize must be devisor
    globalSize = 100032; // hardcoded to prevent needing math.h

    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);

    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

    // Create a context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

    // Create a command queue
    queue = clCreateCommandQueue(context, device_id, 0, &err);

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,
                                        (const char **) &source, NULL, &err);

    // Build the program executable
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "vecAdd", &err);

    // Create the input and output arrays in device memory for our calculation
    d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);

    // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0,
                               bytes, h_a, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0,
                                bytes, h_b, 0, NULL, NULL);

    // Set the arguments to our compute kernel
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
    err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &n);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize,
                                 0, NULL, NULL);

    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);

    // Read the results from the device
    clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0,
                        bytes, h_c, 0, NULL, NULL );

    //Sum up vector c and print result divided by n, this should equal 0 within error
    double sum = 0;
    for(i=0; i<n; i++)
        sum += h_c[i];
    LOGI("final result: %f\n", sum/(double)n);

    // release OpenCL resources
    clReleaseMemObject(d_a);
    clReleaseMemObject(d_b);
    clReleaseMemObject(d_c);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    //release host memory
    free(h_a);
    free(h_b);
    free(h_c);

    return;
}