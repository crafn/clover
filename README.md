# Clover game engine
Migration in progress!

This is what this does:
[Tech Preview](https://www.youtube.com/watch?v=-tum9KOXC2o)

## Getting started
Building is done by a [simple tool](http://github.com/crafn/clbs). 
Download sources

	git clone --recursive https://github.com/crafn/clover.git

### Linux
Required packages when using a clean installation of lubuntu with nvidia gpu:
- mesa-common-dev (for gl.h)
- libx11-dev (for glfw)
- libxxf86vm-dev (for glfw)
- libxrandr-dev (for glfw)
- libxi-dev (for glfw)
- libglu1-mesa-dev (for glew)
- nvidia-opencl-dev (for clover::hardware)
- portaudio19-dev (for clover::hardware)

Building

    cd clover/code/clbs
    clbs -j4

After a succesfull build an executable is located at builds/linux64 or builds/linux32. Note that the project hasn't been built or tested on multiple systems, so it's very likely that some problems will occur.

### Windows
TODO

### TODO
- add build support for windows
- provide data package somehow, maybe building should download it automatically!
