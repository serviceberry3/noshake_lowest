#!/bin/sh

LIBS_PATH=/system/lib64

cp /apex/com.android.os.statsd/lib64/{libstatssocket.so,libstatspull.so} $LIBS_PATH
cp /apex/com.android.art/lib64/{libicui18n.so,libicuuc.so,libnativehelper.so,libandroidicu.so} $LIBS_PATH


#cp /apex/com.android.art/lib64/libnativehelper.so /system/lib64
#cp /apex/com.android.art/lib64/libandroidicu.so /system/lib64
#cp /apex/com.android.os.statsd/lib64/libstatspull.so /system/lib64
#cp /apex/com.android.os.statsd/lib64/libstatssocket.so /system/lib64
#cp /apex/com.android.art/lib64/libicuuc.so /system/lib64
#cp /apex/com.android.art/lib64/libicui18n.so /system/lib64
