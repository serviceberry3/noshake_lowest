#!/bin/sh

#remove extra libs that we copied to /system/lib64
rm /system/lib64/{libstatssocket.so,libstatspull.so,libicui18n.so,libicuuc.so,libnativehelper.so,libandroidicu.so}