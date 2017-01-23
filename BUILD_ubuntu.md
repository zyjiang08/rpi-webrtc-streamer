
# Building RPi-WebRTC-Streamer in Ubuntu

## Branch Notice
  You need WebRTC Branch55 to compile this branch version

## Cross Compile Environment Setup
Cross compile is an essential task in order to compile WebRTC native-code package and RWS(Rpi WebRTC Streamer). In fact, we can not download the WebRTC native-code package with the Raspberry PI.


#### Cross-compiler installation

Cross compiler (gcc-4.9.4 for raspberry pi) [rpi_rootfs](https://github.com/kclyu/rpi_rootfs.git) repo have cross compiler, You don't need install cross compiler. 
Add /opt/rpi_rootfs/tools/arm-linux-gnueabihf/bin in your PATH environment variable to use cross compiler.


#### Raspberry PI sysroot 
Once the cross-compiler setup is completed, you must create a sysroot for the cross compile environment.  please refer to this [rpi_rootfs](https://github.com/kclyu/rpi_rootfs.git) repo.

## Notes before download source code
First, it seems better to describe the location of the directory that developer use before you download the source code. There may be a location is fixed in current code and script itself, if possible, it is recommended to start at the same directory location during compilation. (After confirming that there is no fixed location issue, this section will be removed.)

|package|directory|
|----------------|-----------------|
|WebRTC native-code|$(HOME)/Workspace/webrtc|
|Rpi-Webrtc-Streamer|$(HOME)/Workspace/rpi-webrtc-streamer|
|Rpi sysroot|$(HOME)/Workspace/rpi_rootfs|


## Building WebRTC native-code package
#### Install Prerequisite Software tool
To build the WebRTC native-code package, [Prerequisite Software tool](https://webrtc.org/native-code/development/prerequisite-sw/)  must be installed at first.

#### Download WebRTC native-code package 
To download webrtc source code please use the following command: 

```
mkdir -p ~/Workspace/webrtc
cd ~/Workspace/webrtc
fetch --nohooks webrtc
cd src
gclient sync
git checkout -b rel55 branch-heads/55
gclient sync
```
When the syncing is complete, in order to verify, re-enter the following command **gclient sync** . check the following message comes out. 

```
$ gclient sync
Syncing projects: 100% (2/2), done.                      

________ running '/usr/bin/python -c import os,sys;script = os.path.join("trunk","check_root_dir.py");_ = os.system("%s %s" % (sys.executable,script)) if os.path.exists(script) else 0' in '/home/kclyu/Workspace/webrtc'

________ running '/usr/bin/python -u src/sync_chromium.py --target-revision cede888c27275835e5aaadf5dac49379eb3ac106' in '/home/kclyu/Workspace/webrtc'
Chromium already up to date:  cede888c27275835e5aaadf5dac49379eb3ac106

________ running '/usr/bin/python src/setup_links.py' in '/home/kclyu/Workspace/webrtc'

________ running '/usr/bin/python src/build/landmines.py --landmine-scripts src/webrtc/build/get_landmines.py --src-dir src' in '/home/kclyu/Workspace/webrtc'

________ running '/usr/bin/python src/third_party/instrumented_libraries/scripts/download_binaries.py' in '/home/kclyu/Workspace/webrtc'

________ running 'download_from_google_storage --directory --recursive --num_threads=10 --no_auth --quiet --bucket chromium-webrtc-resources src/resources' in '/home/kclyu/Workspace/webrtc'
```
## Building WebRTC with 
#### Building WebRTC native-code package

1. generate ninja build 
  
```
cd ~/Workspace/webrtc/src
gn gen arm_build
```

2. change build setting 

the below command will open the vi editor.
```
gn args arm_build
```
update the contents with the following.(You can find lastest args.gn file in misc/args.gn, 
        You need to replace '/home/kclyu/' with your account's home path.)
```
# Build arguments go here. Examples:
#   is_component_build = true
#   is_debug = false
# See "gn args <out_dir> --list" for available build arguments.

is_component_build=false
#is_official_build=true
#is_debug=false # Relese Version

host_toolchain="//build/toolchain/linux:arm"
target_os="linux"
target_cpu="arm"
arm_float_abi="hard"
arm_use_neon=true
arm_tune="cortex-a7"
is_clang=false
use_sysroot=true
target_sysroot="/home/kclyu/Workspace/rpi_rootfs"
system_libdir="/home/kclyu/Workspace/rpi_rootfs/usr/lib/arm-linux-gnueabihf"
gold_path = "/home/kclyu/tools/rpi_tools/arm-linux-gnueabihf/bin"

#
#
use_ozone=true
is_desktop_linux=false
rtc_desktop_capture_supported=false

#
#
rtc_build_with_neon=true
enable_nacl=false
enable_pdf=false
enable_webvr=false
rtc_use_h264=true
use_openh264=true
rtc_include_tests=false
rtc_desktop_capture_supported=false
rtc_include_pulse_audio=false
rtc_enable_protobuf=false
treat_warnings_as_errors=false

```
3. building WebRTC library

the below command will start to build.
```
ninja -C arm_build
```
After compilation is finished without an error, go to the next step to compile rws.

## Building RWS(rpi-webrtc-streamer)
  
*  Setup Makefile and cross_*.mk 
 
|Item|file|description|
|----------------|-----------------|-----|
|Builder Type|Makefile|cross_gn.mk for GN builder, cross_gyp.mk for GYP builder|
|SYSROOT|cross_*.mk|sysroot for raspberry pi |
|WEBRTCROOT|cross_*.mk|WebRTC root directory path|
|WEBRTCOUTPUT|cross_*.mk|WebRTC build output path|

   
*  build rws
 ```
cd ~/Workspace/
git clone https://github.com/kclyu/rpi-webrtc-streamer.git
cd rpi-webrtc-streamer/src
sh ../../scripts/config_h264bitstream.sh 
cd ..
make
```
## Branch55 Notice
This branch is a version separated from rws commit '754b309e5ad8ed0b3f6285d7792769c81d936f09', so it does not contain any patches that were later patched.


## Version History
 * 2017/01/20 v0.56:
   - WebRTC branch55 branch
 * 2016/09/20 v0.56 : Initial Version


