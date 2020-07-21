#!/bin/bash
make clean
make mrproper

export ARCH=arm64
export BUILD_CROSS_COMPILE=$(pwd)/../PLATFORM/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
export BUILD_JOB_NUMBER=`grep processor /proc/cpuinfo|wc -l`
export ANDROID_MAJOR_VERSION=p
export ANDROID_VERSION=90000
export PLATFORM_VERSION=9.0.0

mkdir out 
chmod -R 777 out

make -C $(pwd) O=$(pwd)/out -j$BUILD_JOB_NUMBER ARCH=$ARCH CROSS_COMPILE=$BUILD_CROSS_COMPILE KCFLAGS=-mno-android  dream2qlte_chn_open_defconfig
make -C $(pwd) O=$(pwd)/out -j$BUILD_JOB_NUMBER ARCH=$ARCH CROSS_COMPILE=$BUILD_CROSS_COMPILE KCFLAGS=-mno-android

#out/arch/arm64/boot/
