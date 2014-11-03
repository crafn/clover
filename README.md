# Clover game engine
Migration in progress!

This is what this does:
[Tech Preview](https://www.youtube.com/watch?v=-tum9KOXC2o)

## Getting started
Building is done by a simple tool which requires Python 2.7: [Clover Build System](http://github.com/crafn/clbs) 

### Linux
Required packages when using a raw installation of lubuntu with nvidia gpu:
- libboost-all-dev (TODO: should use in-repo)
- libx11-dev
- mesa-common-dev (for gl.h)
- libxxf86vm-dev (for glfw)
- libxrandr-dev (for glfw)
- libxi-dev (for glfw)
- libglu1-mesa-dev (for glew)
- libvorbis-dev (for clover::audio)
- nvidia-opencl-dev (for clover::util)
- libfreetype6-dev (for clover::visual)
- portaudio19-dev (for clover::audio)

Building

    cd ./code/clbs
    clbs -j4

After a succesfull build an executable is located at builds/linux64 or builds/linux32. Note that the project hasn't been built or tested on multiple systems, so it's very likely that some problems will occur.

### Windows
TODO

### TODO
- add build support for windows
- provide data package somehow, maybe building should download it automatically!
