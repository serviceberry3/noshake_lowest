This is a version of my text_stabilization repo that doesn't do any rerendering. It just uses a DRM dumb buffer to draw some sample text once, and then slides the buffer around on the screen based on incoming accelerometer readings. Uses a spring-mass model as done in text_stabilization.

Push copy_libs.sh to the Android device and run it to copy necessary libs into /system/lib64. Then run ```make```. Make sure you've run   
```
adb root
adb shell mount -o rw; remount /system; reboot
```

Run ```make push``` and make sure the executable is sent to the device successfully. Then, in the Android device shell, call ```chmod a+x``` on the executable, and run it.
