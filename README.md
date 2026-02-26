# Echolocation-Game-Engine
3D engine where the user, instead of using light to see, uses sonar.

# Setup
* Install Vulkan (1.4.328.0), glfw3 (3.4.1), glm (1.02) with vcpkg
```
vcpkg install vulkan glfw3 glm
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