## OpenSWF
[![Build Status](https://travis-ci.org/shawnscode/openswf.svg?branch=master)](https://travis-ci.org/shawnscode/openswf)

This project is used to enable ease swf integration into video games. Its far from a stable version literally, but i'll keep on developing. If u have any suggestion, feel free to make a pull request or just open a issue to let me know.

oammix [at] gmail [dot] com

### Make
I'm working on MacOs currently, if so do u, u can compile openswf by following below:

    premake5 gmake
	cd build/3rd
	make
	cd -
	make
	cd test
	make

Then u could find unit-test and some simple examples in `path-to-openswf/bin`. If u prefer a ide based environment, like xcode, please install [premake5](http://premake.github.io), and generate xcode project files with `premake5 xcode4`.

This prject has not much platform specified code right now, so it should be easy to port to other platforms with big-endian bit order.

### Dependencies

1. glfw3
1. glew
1. glm
1. zlib

