all:
	aarch64-linux-android21-clang accel_read.c -o accel_read -w android-21/libandroid.so
nodk:
	arm-linux-gnueabi-gcc -static -march=armv7-a accel_read.c -o accel_read -w libandroid.so
push:
	adb push accel_read /system/files
