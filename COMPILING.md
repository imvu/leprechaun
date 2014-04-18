# Ubuntu 10.4

These instructions were put together in April of 2014.
They are my best guess as to the steps that led to a complete build.

I did not start with a fresh machine, so there may be additional packages need that are not listed in this file.

This will require at least 13G of space.

# Grab Chromium

    svn co http://src.chromium.org/svn/trunk/tools/depot_tools

    export PATH=$PATH:<path to>/depot_tools/

    mkdir <path to>/chromium
    cd <chromium path>

    gclient config http://src.chromium.org/svn/releases/21.0.1180.91

edit 21.0.1180.91/DEPS
change gsutil source to https://github.com/GoogleCloudPlatform/gsutil.git@158f32f

    cd src
    ./build/install-build-deps.sh
    cd ..
    gclient sync --jobs 8 --force

I had to run the sync several times. It seemed to get a new error each time I ran it until it finally passed.

# Grab CEF

    cd src
    svn co http://chromiumembedded.googlecode.com/svn/branches/1180/cef3 cef

    cd cef

The next few steps may be unneccessary, but they worked for me:
    ./cef_create_projects.sh
        (that had an error for me)
    edit 21.0.1180.91/DEPS to move trace-view from r168 to r55
    ./cef_create_projects.sh
        (this will break again)
    edit 21.0.1180.91/DEPS back to r168
    cd ../../
    gclient sync --jobs 8 --force
        (this will fail now too, this time on gyp errors. Ignore it)
    cd src/cef/
    ./cef_create_projects.sh
That seems to get me to a successful run.

After that works, it told me to rerun gclient sync:
    gclient sync --jobs 8 --force
        (this will fail again, also on gyp errors. Ignore it again)

# Make cefclient

## Code massage

edit net/third_party/nss/ssl/ssl3ecc.c  line 986
comment out the line that reads
    case SEC_OID_PKCS1_SHA224_WITH_RSA_ENCRYPTION:

edit base/debug/trace_event.h
here is the diff:

    Index: base/debug/trace_event.h
    diff --git a/base/debug/trace_event.h b/base/debug/trace_event.h
    index a82b4a86699651ee4485015a22ba0bae700096d4..d73bb0a4011496930fe3f604ff1420ff6a712075 100644
    --- a/base/debug/trace_event.h
    +++ b/base/debug/trace_event.h
    @@ -714,10 +714,12 @@ class TraceStringWithCopy {
    }

    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned long long, TRACE_VALUE_TYPE_UINT)
    +INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned long, TRACE_VALUE_TYPE_UINT)
    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned int, TRACE_VALUE_TYPE_UINT)
    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned short, TRACE_VALUE_TYPE_UINT)
    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned char, TRACE_VALUE_TYPE_UINT)
    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(long long, TRACE_VALUE_TYPE_INT)
    +INTERNAL_DECLARE_SET_TRACE_VALUE_INT(long, TRACE_VALUE_TYPE_INT)
    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(int, TRACE_VALUE_TYPE_INT)
    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(short, TRACE_VALUE_TYPE_INT)
    INTERNAL_DECLARE_SET_TRACE_VALUE_INT(signed char, TRACE_VALUE_TYPE_INT)


    svn up -r 1180 cef/libcef/browser/browser_host_impl_gtk.cc

edit libcef/browser/browser_host_impl.cc
    1757: 
    - cef_file_dialog_mode_t mode;
    + cef_file_dialog_mode_t mode = FILE_DIALOG_OPEN;

edit cef/libcef/browser/browser_host_impl_gtk.cc
add the folling line:
    cef/libcef/browser/browser_host_impl_gtk.cc:110:
    "All Files");

## Actual building

    cd <chromium path>/src/cef/tools
    ./build_projects.sh Release

I had to run the build command several times as I kept getting further each time I ran it. Eventually it passed.

# Leprechaun

## Install Scons
    sudo apt-get install scons

## Build leprechaun
    cd <place where you have leprechaun checked out>
    scons CEFDIR=<chromium path>

# DONE!
