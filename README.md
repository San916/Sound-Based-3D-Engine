# Sound-Based-3D-Engine
3D engine where the user, instead of using light and colors to see, uses raytracing to mimic sound-based methods.

# Setup
* Install Vulkan (1.4.328.0), glfw3 (3.4.1), glm (1.02), jolt (5.5.0) with vcpkg
```
vcpkg install vulkan glfw3 glm joltphysics
```
* Create a `/build` directory in the project folder and `cd` into it
* Run:
```
cmake ..
cmake --build .
```
* To run engine
```
debug\engine
```
* To run without validation layers:
```
cmake --build . --config Release

release\engine
```
# Demo
(This demo contains slight artifacting)
<img src="assets/github_demo/demo.gif">
<img src="assets/github_demo/physics_demo.gif">
