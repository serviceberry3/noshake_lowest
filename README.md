This is a version of my text_stabilization repo that doesn't do any rerendering. It just uses a DRM dumb buffer to draw some sample text once, and then slides the buffer around on the screen based on incoming accelerometer readings. Uses a spring-mass model as done in text_stabilization.

# Requirements #
 - Device: Google Pixel 4
 - Android NDK: preferably version android-ndk-r11c-linux-x86_64 or similar
 - AOSP build: aosp_flame-userdebug 11 RQ1A.201205.008.A1 (for some reason only AOSP around this time seems to have the required libs to access the accelerometer)
 - DRM driver modifications (custom kernel) as outlined in [this repo](https://github.com/serviceberry3/android_drm_dumb)

# Instructions #  

Modify scripts/path_setup.sh appropriately to match the path to your Android NDK toolchains bin. To add that bin to your current PATH, run  
```
source scripts/path_setup.sh
```

Then run ```make clang64```. Make sure you've first run   
```
adb root
adb shell mount -o rw; remount /system; reboot
```
Run ```make push``` and make sure the executable from your now-created out/ folder and the lib copying script are sent to the device successfully. Then, in the Android device shell, call ```chmod a+x``` on the ```noshake_lowest``` executable and ```copy_libs.sh``` script. Run the copying script to copy necessary libs into /system/lib64. Then run the noshake executable.
