# Clover game engine
Migration in progress!

This is what this does:
[Tech Preview](https://www.youtube.com/watch?v=-tum9KOXC2o)

## Getting started
Note that the project hasn't been built or tested on multiple systems, so it's very likely that some problems will occur.

### On Windows
1.  Install Mingw-w64-\<TODO check working version and add link\> compiler tool set
2.  Download [this repository](https://github.com/crafn/clover/archive/master.zip) and extract it somewhere
    - If you plan to contribute, use `git clone --recursive https://github.com/crafn/clover.git` instead
3.  Install [clover build system](http://github.com/crafn/clbs)
4.  Run `code/clbs/build_dev.bat` to build the engine

After succesfull build an executable is located at `builds/win*`

### On Linux
1.  Install dependencies; required packages are (on lubuntu with nvidia gpu):
    - mesa-common-dev (for gl.h)
    - libx11-dev (for glfw)
    - libxxf86vm-dev (for glfw)
    - libxrandr-dev (for glfw)
    - libxi-dev (for glfw)
    - libglu1-mesa-dev (for glew)
    - nvidia-opencl-dev (for clover::hardware)
    - portaudio19-dev (for clover::hardware)
2.  Clone repository and submodules

		git clone --recursive https://github.com/crafn/clover.git
3.  Install [clover build system](http://github.com/crafn/clbs)
4.  Build the engine

		cd clover/code/clbs
		clbs -j4

After succesfull build an executable is located at `builds/linux*`

### TODO
- provide data package somehow
