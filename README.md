# Clover game engine

This is what this does:
[Tech Preview](https://www.youtube.com/watch?v=-tum9KOXC2o)

## Getting started
Note that the project hasn't been built or tested on multiple systems, so it's very likely that some problems will occur. Also, the engine has many game specific features and is not production quality.

### On Windows
_As of 2015-01-28 worrying for windows build has been temporarily ended. It's better to get the game finished on single platform (linux) first than to constantly struggle with MinGW and not get the game finished. Contributions are welcome though._

1.  Install [64bit win-builds MinGW](http://win-builds.org/download.html) compiler toolset. Be sure to add the `bin` directory inside your MinGW installation to your `PATH` environment variable so that `g++` can be called from the command line.
2.  Install an OpenCL library and headers. For NVidia GPUs you have to install the [CUDA Toolkit](https://developer.nvidia.com/getcuda). <sub><sup>This sucks and shouldn't be necessary in the future.</sup></sub>
3.  Use [git](http://git-scm.com/) to download this repository

        git clone --recursive https://github.com/crafn/clover.git
    Don't use the github zip package because it lacks submodules.
4.  Install [clover build system](http://github.com/crafn/clbs)
5.  Run `code/clbs/build_dev.bat` to build the engine

After succesful build an executable is located at `builds/windows*`

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

After succesful build an executable is located at `builds/linux*`

### Running the engine with Clover game data
Download the [resource archive](http://crafn.kapsi.fi/data/clover/resources.zip) and extract it to the repo root. `resources` directory should be next to `code` directory, and executable should be launched in the directory containing it. The game data is currently licensed under the strictest creative commons license (Attribution-NonCommercial-NoDerivatives), but that might change in the future.

Further info about the game project on [my personal website](http://crafn.kapsi.fi/?path=games/project_clover).
