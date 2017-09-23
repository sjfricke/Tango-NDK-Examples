# Tango-NDK-Examples
This is a set of examples projects of how to use Google Tango NDK in various ways. These are not ment to be production level examples, but more of a guiding step to get off the ground adding features into your application.

* Need some help getting up and running with Tango or NDK? Check out [Tangotutorial.org](http://tangotutorial.org)!
* Want to see a different type of example? Add your own? Fix a flaw in these? Please feel free to make an issue or pull request.
  * If some code didn't make sense and after you figured out, add a comment in the code and push a pull request so the next person doesn't have to get stuck. Its hard to know what I didn't know when I first started with Tango or NDK sometimes.

## How to run/install/use examples
* All examples are self-contained within their own respected folders. If you want the easiest thing is just clone the repo and open each folder in Android Studios and it should build without any issues.
* The **ONLY** thing I have seperated from each project is the Tango API library folders since it would be wasteful to copy and paste those around.
  * Either have the `tango_client_api` and `tango_support_api` in the same directory as the parent folder of the project (as it is currently in the repo) or read [Loading Tango API Libraries](https://github.com/sjfricke/Tango-C-NDK-Tutorial/blob/master/Section_03_Tango/Tutorials/Chapter_03.md) chapter from tutorial repo.
* Here are the current system enviroments to shown to work with examples, please add to list if you find success or report an issue if crap breaks!
  * OS
	* Ubunut 16.04
	* Windows 10
  * Android Studios
	* 2.3.3
  * NDK
	* r15
  * Android API
	* 23

----------

## List of demos

### [UI interface](./UI_interface)
* The idea behind this is to show how to get a UI interfacing with your logic in the NDK layer.
* This really is good example if trying to understand how JNI works and sending data back and forth.

### [OpenCL PointCloud](./OpenCL_PointCloud)
* This demo is to help get OpenCL intergrated with Tango.
* Use point cloud data as an example, but can apply OpenCL kernels to way more ideas
* OpenCL is tricky since its not a native support library, more info in the project README on subject

### [Depth Viberation](./Depth_Viberation)
* This demo shows how to have your phone viberation by how close your phone is to an object
* Also shows how to have an async callback function from the C++ layer to Java
** Since Viberation is only an SDK feature we need to be able to call the Java based function from C++ good anytime

### [Draco and Ply](./draco_and_ply)
* **NOT DONE**
** Currently have ply saving done, need to figure out how to incorperate draco to ndk
* A demo to show how to save a file into a .ply or .draco file and transfer it to a server

### [RGBA Data](./RGBA_Data)
* **NOT DONE** (But almost, just needs polishing)
** A demo to show how to get an RGBA pixel buffer from Tango Frame
** Might suggest using the Camera2 API as well
