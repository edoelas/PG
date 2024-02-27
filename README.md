# PGUPV (Programación Gráfica UPV)

PGUPV is a C++ library for learning modern OpenGL for Windows and Linux.

It provides many tools for quickly building OpenGL applications, without having to deal with tedious code like setting VAO and VBO, reading image files, reading 3D model files, etc.

It is *not* a game engine. It has no support for audio, physics, AI and so on. 

It is designed for teaching OpenGL courses by providing the scaffolding, but also not hiding too much GL details from the student.

The library comes with examples, heavily commented (in Spanish, sorry), but they should be easy to understand.

## What do I need to try it?

The library supports Windows and Linux (since Apple abandoned OpenGL long ago).

### Windows
Install a recent Visual Studio (any edition will do, Code, Pro, Enterprise...). You need to install C++ support. Also requires CMake (https://cmake.org/download/).

1. Get the code, either using git or getting the ZIP file from this page.
2. If you use the CMake GUI, keep reading. If you prefer the console, read the Linux instructions below.
  * select the project's root folder (the one that contains the file ```README.md```) in the box "Where is the source code"
  * write a non-existing folder (for example, called ```build``` inside of the previous folder) in "Where to build the binaries"
  * click on ```Configure```. It will ask which compiler to use. Select your preferred version of Visual Studio
  * click on ```Generate```
  * click on Open Project. If everything worked correctly, it will open Visual Studio with the solution
3. You can select any project inside of the examples folder (right click on the project, ```Set as startup project```)
4. Hit F5 and have fun! Open the main.cpp and study the code, run it step by step, etc.

### Linux

First, you will need to install some dependencies. In Ubuntu:

```
sudo apt install build-essential git cmake libfreeimage-dev libfreetype-dev pkg-config libgtk-3-dev libglu1-mesa-dev ffmpeg libavcodec-dev libavdevice-dev libavformat-dev libassimp-dev libsdl2-dev libsdl2-ttf-dev libglew-dev
```

1. Get the code, either using git or getting the ZIP file from this page.
2. If you use Visual Studio Code, install the C++ and CMake support and open the root folder (the one that contains the file README.md). If you use the console:
```
git clone https://github.com/fjabad/pgupv.git
cd pgupv
mkdir build
cd build
cmake ..
make -j4
```
   3. Go to the ```bin``` directory and try the examples.




