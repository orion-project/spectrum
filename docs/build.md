# Building from sources

## Prepare build environment

### Qt

[Qt for Open Source](https://www.qt.io/download-open-source) must be installed. The target Qt version for all platforms is 5.15.2 currently. For Windows select the "MSVC 2019 64-bit" flavour. QtCreator is the default IDE used for the project.

### Visual Studio

Microsoft C++ compiler is used on Windows, so [Visual Studio Community 2022](https://visualstudio.microsoft.com/en/vs/community/) must be installed separately. The only required target is the "Desktop development with C++".

Visual Studio can also be used as the IDE for Qt projects with the [Qt Visual Studio Tools](https://doc.qt.io/qtvstools-2) extension. In VS go to "Extensions -> Manage Extensions", search for the extension and install it, then go to "Extensions -> Qt VS Tools -> Qt Versions" and register a Qt installation path.

### CMake

CMake could be installed separately or as part of Qt installation.

### vcpkg

[vcpkg](https://vcpkg.io/) is an open source package manager for C++. It must be installed in a directory separated from the project for preparing third party dependencies, such as Python:

```bash
git clone https://github.com/microsoft/vcpkg.git
```

Then run `bootstrap-vcpkg.sh` or `bootstrap-vcpkg.bat` depending on the platform, add the vcpkg directory to the `PATH`, and create the `VCPKG_ROOT` variable containing the installation path.

Download and compile dependencies in the project directory:

```bash
vcpkg install
```

When configuring project in QtCreator, add additional option in the CMake Initial Configuration tab and reconfigure. It must pointing to the installed vcpkg toolchain file location, e.g.:

```
-DCMAKE_TOOLCHAIN_FILE:FILEPATH=/home/user/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Qt quirks

**[Ubuntu] Platform plugin**

If QtCreator refuses to run on Ubuntu because of

```log
qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in "" even though it was found.
```

try to run 

```bash
sudo apt-get install --reinstall libxcb-xinerama0
```

**[Ubuntu] GL lib**

First time when building on Ubuntu it says

```log
../Qt/5.15.2/gcc_64/include/QtGui/qopengl.h:141:22: fatal error: GL/gl.h: No such file or directory
```

additional dev libs must be installed:

```bash
sudo apt-get install build-essential libgl1-mesa-dev
```

These also mentioned in QCustomPlot [instructions](https://www.qcustomplot.com/index.php/tutorials/settingup) so probably will be needed:

```bash
sudo apt-get install mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev
```

## Prepare source code

```bash
git clone https://github.com/orion-project/spectrum
cd spectrum
git submodule init
git submodule update
```

## Dev build

Use *Qt Creator IDE* to do dev builds. Just open project file `CMakeLists.txt` in the IDE and configure it to use the installed Qt-kit. The target Qt version for all platforms is 5.15.2 currently. Any of newer versions should also fir but probably will require code adjustments.

Target file is `bin/spectrum` (Linux), `bin/spectrum.app` (MacOS), or `bin\spectrum.exe` (Windows). 
