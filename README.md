This is a version of my text_stabilization repo that doesn't do any rerendering. It just uses a DRM dumb buffer to draw some sample text once, and then slides the buffer around on the screen based on incoming accelerometer readings. Uses a spring-mass model as done in text_stabilization.

INSTRUCTIONS:  

Modify scripts/path_setup.sh appropriately to match the path to your Android NDK toolchains bin. To add that bin to your current PATH, run  
```
source scripts/path_setup.sh
```

Then run ```make clang64```. Make sure you've first run   
```
adb root
adb shell mount -o rw; remount /system; reboot
```
Run ```make push``` and make sure the executable and lib copying script is sent to the device successfully. Then, in the Android device shell, call ```chmod a+x``` on the ```noshake_lowest``` executable and ```copy_libs.sh``` script. Run the copying script to copy necessary libs into /system/lib64. Then run the noshake executable.
