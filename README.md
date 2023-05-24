# meh_IMGUI

An Immediate Mode GUI and windowing library for a game engine that I'm working on. It supports windows and linux(wayland only).
This library is intended to be used a static library in windows, similar to glfw.\
This library is written in c++20. It uses clang-cl in windows as g++ does not natively support windows platform and clang++ in linux because this library is developed to be used inside a game engine where performance is of top priority. The build system is entirely managed by Tup.

## How to build?
If you wish to build this repository you are expected to have [Tup](https://gittup.org/tup/) installed. If not, you can download Tup by clicking [right here](https://gittup.org/tup/win32/tup-latest.zip), extract it and place the executable somewhere in your system path. Additionally, since this repository uses clang-cl and clang++, you need them as well.

This repository additionally uses `vulkan` and `ft_to_atlas`, both of which are included in the source code.

Clone this repository with recursize flag
```git 
git clone --recursive https://github.com/FullGardenStudent/meh_IMGUI
```
### Windows
Since this repository requires clang-cl, you can get a pre-build binary from [here](https://releases.llvm.org/download.html) and install it if not installed already. Once you are done installing it, you can build the project with tup. This will generate a `meh_IMGUI.lib` static library, which you can use along with `meh_IMGUI.hh` and `meh_IMGUI_windows.hh` header files. These three files are all you'll need to use this library.
```tup
tup init  # initialize tup database
tup       # run tup
```
### Linux
(not yet added. I will add it soon)
You need tup and clang++. Fetch them from your distro's package manager. Build commands are same as windows.
```tup
tup init  # initialize tup database
tup       # run tup
```
## Development how?
You can generate a `compile_commands.json` file using tup by running `tup compiledb`(of course, this should be run after running `tup init` or else it won't work) and chill with lsp and the text editor of your choise.\
There is also a `meh_IMGUI.sln` visual studio solution file for developing in windows using visual studio😵
