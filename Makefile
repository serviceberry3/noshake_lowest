INC=-I/home/nodog/docs/files/DroneJob/noshake_lowest/include
#-I/home/nodog/Android/Sdk/ndk/android-ndk-r21d-linux-x86_64/android-ndk-r21d/sysroot/usr/include/android 

clang:
	armv7a-linux-androideabi16-clang accel_read.c -o accel_read $(INC) -w android-21/libandroid.so
clang64:
	aarch64-linux-android21-clang accel_read.c drm_low.c -o out/noshake_lowest $(INC) -w android-21/libandroid.so
gcc:
	arm-linux-gnueabi-gcc -march=armv7-a accel_read.c -o accel_read $(INC) -w android-19/libandroid.so
gcc64:
	aarch64-linux-gnu-gcc -march=armv8-a accel_read.c -o accel_read $(INC) -w android-21/libandroid.so
push:
	adb push out/accel_read scripts/copy_libs.sh /system/files
test:
	armv7a-linux-androideabi21-clang -static -march=armv7-a accel_read.c -o accel_read $(INC) -w android-21/libandroid.so
drm64:
	aarch64-linux-gnu-gcc -static -march=armv8-a drm_low.c -o drm_64 $(INC) -w
clean:
	rm accel_read drm_low