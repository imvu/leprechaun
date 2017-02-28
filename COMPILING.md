These instructions were put together in June of 2015.

# Ubuntu 14.04

## Download the compiled CEF binary for Linux 64bit or build CEF version 3.2924.1571.gcdcdfa9 from source

The pre-compiled binary distribution of CEF is not compatible with libstdc++ available on Ubuntu Precise so you will either want to build CEF from source or manually upgrade libstdc++.

## Install required dev packages

    sudo apt-get install libgtk2.0-dev libgtkglext1-dev ninja-build

### To build from source:

    wget https://bitbucket.org/chromiumembedded/cef/raw/master/tools/automate/automate-git.py
    python automate-git.py --download-dir=<some_temp_location> --branch=2357 --no-debug-build
    cp -r <some_temp_location>/chromium/src/cef/binary_distrib/cef_binary_3.2357.1280.<some_hash>.linux64/ <somewhere>

### To use the pre-compiled binary:

[Linux 64bit CEF 3.2357.1276](http://opensource.spotify.com/cefbuilds/index.html)

Once downloaded, unpack it:

    cd <somewhere>
    tar xvf /path/to/cef_binary_3.2924.1571.gcdcdfa9_linux64.tar.bz2

## Install cmake 3.x

Building the CEF binaries requires cmake >= 2.8.12.2 which is not available as a debian package on Ubuntu Precise.

    cd <somewhere>
    wget http://www.cmake.org/files/v3.2/cmake-3.2.3.tar.gz
    tar -xf cmake-3.2.3.tar.gz
    cd cmake-3.2.3/
    ./bootstrap
    make
    sudo make install

## Build the pre-built CEF binaries

The libcef.so file is pre-built and included in the pre-built CEF binary package. However, the
libcef_dll_wrapper library is not pre-built. It needs to be built manually along with the two
sample CEF applications.

    cd /path/to/cef_binary_3.2357.1280_<some_hash>_linux64/

The cmake configuration includes a compiler flag that is incompatible with the version of gcc that
is available as a debian package on Ubuntu Precise. Either upgrade your gcc compiler to 4.7 or else
manually edit the CMakeLists.txt file in the root of the pre-built CEF binary folder and change all
instances of "-std=gnu++11" to "-std=c++0x".

    mkdir build
    cd build
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
    ninja libcef_dll_wrapper

## Install scons

This can be done either by installing the scons debian package or by downloading and installing the source.

### Install scons debian package:

    sudo apt-get install scons

### Install scons from source:

    wget http://prdownloads.sourceforge.net/scons/scons-src-2.3.4.tar.gz
    tar -xf scons-src-2.3.4.tar.gz
    cd scons-src-2.3.4/
    python bootstrap.py build/scons
    cd build/scons
    sudo python setup.py install

## Get Leprechaun

    cd <somewhere>
    git clone git@github.com:imvu/leprechaun.git

## Build Leprechaun

    cd leprechaun
    scons CEFDIR=/path/to/extracted/cef

The leprechaun binaries will be under leprechaun-binaries/linux/

# OSX 10.9 XCode 5.1, SDK 10.6

## Install TheUnarchiver

    Go to http://unarchiver.c3.cx/unarchiver or http://wakaba.c3.cx/s/apps/unarchiver.html and download

## Download the 64-bit compiled binary CEF library

[Mac 64bit CEF 3.2357.1276.gd4b589c](https://cefbuilds.com/)

Extract the downloaded archive file by navigating to it using Finder and double clicking on it.

## Install cmake

[cmake-3.2.3-Darwin-x86_64.dmg](http://www.cmake.org/files/v3.2/cmake-3.2.3-Darwin-x86_64.dmg)

## Clone the depot\_tools git repository:

    cd <somewhere>
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    export PATH=$PATH:`pwd`/depot_tools

## Apply some patches to CEF

/path/to/cef_binary_3.2357.1276.gd4b589c_macosx64/include/base/cef_build.h:141

    -#define OVERRIDE override
    +#define OVERRIDE

/path/to/cef_binary_3.2924.1571.gcdcdfa9_macosx64/include/internal/cef_types_wrappers.h:985

    -s->range = { 0, 0 };
    +s->range.from = 0;
    +s->range.to = 0;

## Build the pre-built CEF binaries

The libcef.so file is pre-built and included in the pre-built CEF binary package. However, the
libcef_dll_wrapper library is not pre-built. It needs to be built manually along with the two
sample CEF applications.

    cd /path/to/cef_binary_3.2357.1276.gd4b589c_macosx64
    mkdir build
    cd build
    /Applications/CMake.app/Contents/bin/cmake -G "Ninja" -DPROJECT_ARCH="x86_64" -DCMAKE_BUILD_TYPE=Release ..
    ninja cefclient cefsimple

### Install scons:

    wget http://prdownloads.sourceforge.net/scons/scons-src-2.3.4.tar.gz
    tar -xf scons-src-2.3.4.tar.gz
    cd scons-src-2.3.4/
    python bootstrap.py build/scons
    cd build/scons
    sudo python setup.py install

## Get Leprechaun

    cd <somewhere>
    git clone git@github.com:imvu/leprechaun.git

## Build Leprechaun

    cd leprechaun
    scons CEFDIR=/path/to/cef_binary_3.2357.1276.gd4b589c_macosx64

# Windows with Visual Studio 2010 SP1, Windows SDK v7.0A

## Install [depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)

    cd <somewhere>
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    PATH=%PATH%;<somewhere>\depot_tools

## Install [7zip](http://sourceforge.net/projects/sevenzip/files/latest/download)

## Install [cmake](http://www.cmake.org/files/v3.2/cmake-3.2.3-win32-x86.exe)

## Download the Windows [compiled CEF binary](https://cefbuilds.com/)

Choose Windows 32bit CEF 3.2357.1280.geba024d

Browse to the downloaded cef_binary_3.2357.1280.geba024d_windows32.7z file in explorer, right click, select 7zip, select Extract... and browse to a location on your hard drive.

## Apply some patches to CEF

These may not be necessary if using Visual Studio 2013 update 4, Windows SDK 8.1 as reported required by [CEF Branching and Building](https://bitbucket.org/chromiumembedded/cef/wiki/BranchesAndBuilding) under "Windows Build Requirements" for release 2357.

C:\path\to\cef_binary_3.2357.1280.geba024d_windows32\include\base\cef_build.h:141

    -#define OVERRIDE override
    +#define OVERRIDE

C:\path\to\cef_binary_3.2357.1280.geba024d_windows32\CMakeLists.txt:391

       # /wd"4127"             = Ignore "conditional expression is constant" warning
    +  # /wd"4189"             = Ignore "local variable is initialized but not referenced" warning
       # /wd"4244"             = Ignore "conversion possible loss of data" warning
    +  # /wd"4355"             = Ignore "'this' : used in base member initializer list" warning
    +  # /wd"4510"             = Ignore "default constructor could not be generated" warning
       # /wd"4512"             = Ignore "assignment operator could not be generated" warning
    +  # /wd"4610"             = Ignore "class XXX can never be instantiated - user defined constructor required" warning
       # /wd"4701"             = Ignore "potentially uninitialized local variable" warning
       # /wd"4702"             = Ignore "unreachable code" warning
       # /wd"4996"             = Ignore "function or variable may be unsafe" warning
    -  set(CEF_COMPILER_FLAGS          "/MP /Gy /GR- /Zi /W4 /WX /wd\"4100\" /wd\"4127\" /wd\"4244\" /wd\"4512\" /wd\"4701\" /wd\"4702\" /wd\"4996\"")
    +  set(CEF_COMPILER_FLAGS          "/MP /Gy /GR- /Zi /W4 /WX /wd\"4100\" /wd\"4127\" /wd\"4189\" /wd\"4244\" /wd\"4355\" /wd\"4510\" /wd\"4512\" /wd\"4610\" /wd\"4701\" /wd\"4702\" /wd\"4996\"")
       # /MTd                  = Multithreaded debug runtime

## Build the pre-built CEF binaries

The libcef.dll file is pre-built and included in the pre-built CEF binary package. However, the
libcef_dll_wrapper library is not pre-built. It needs to be built manually along with the two
sample CEF applications.

    cd /D C:\path\to\cef_binary_3.2357.1280.geba024d_windows32
    md build
    cd build
    "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
    ninja cefclient cefsimple

## Get Leprechaun

    cd <somewhere>
    git clone git@github.com:imvu/leprechaun.git

## Build Leprechaun

    cd leprechaun
    scons CEFDIR=C:\path\to\cef_binary_3.2357.1280.geba024d_windows32
