#!/bin/bash
make clean
make mrproper

export ARCH=arm64
export BUILD_CROSS_COMPILE=$(pwd)/../PLATFORM/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
export BUILD_JOB_NUMBER=`grep processor /proc/cpuinfo|wc -l`
export ANDROID_MAJOR_VERSION=p
export PLATFORM_VERSION=9.0.0

mkdir out 
chmod -R 777 out

make -C $(pwd) O=$(pwd)/out -j$BUILD_JOB_NUMBER ARCH=$ARCH CROSS_COMPILE=$BUILD_CROSS_COMPILE KCFLAGS=-mno-android  dream2qlte_chn_open_defconfig
make -C $(pwd) O=$(pwd)/out -j$BUILD_JOB_NUMBER ARCH=$ARCH CROSS_COMPILE=$BUILD_CROSS_COMPILE KCFLAGS=-mno-android

# build boot.img by aik tool
rm -rf $(pwd)/build/aik/ramdisk $(pwd)/build/aik/split_img
cp $(pwd)/build/boot/dream2qltechn/* $(pwd)/build/aik/
mv $(pwd)/out/arch/arm64/boot/Image-gz-dtb $(pwd)/build/aik/split_img/boot.img-zImage

if [ `whoami` = "root" ];then
    bash $(pwd)/build/aik/cleanup.sh
    bash $(pwd)/build/aik/repackimg.sh
else
    sudo bash $(pwd)/build/aik/cleanup.sh
    sudo bash $(pwd)/build/aik/repackimg.sh
fi

cp $(pwd)/build/aik/image-new.img $(pwd)/build/zip/