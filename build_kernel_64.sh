#!/bin/bash
# MSM8998 kernel build script v0.1

ARCH=arm64
BUILD_WHERE=$(pwd)
BUILD_KERNEL_OUT_DIR=$BUILD_WHERE/out
BUILD_CROSS_COMPILE=$BUILD_WHERE/../PLATFORM/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
BUILD_JOB_NUMBER=`grep processor /proc/cpuinfo|wc -l`
ANDROID_MAJOR_VERSION=p
PLATFORM_VERSION=9.0.0

BUILD_COMMAND=$1
CLEAN_OPTION=$2

if [ "$BUILD_COMMAND" == "dream2qlte_chn_open" ]; then
	PRODUCT_NAME=dream2qltechn
    SIGN_MODEL=SM-G9550_CHN_CHC_CHN0
elif [ "$BUILD_COMMAND" == "dreamqlte_chn_open" ]; then
	PRODUCT_NAME=dreamqltechn
    SIGN_MODEL=SM-G9500_CHN_CHC_CHN0
elif [ "$BUILD_COMMAND" == "greatqlte_chn_open" ]; then
	PRODUCT_NAME=greatqltechn
    SIGN_MODEL=SM-N9500_CHN_CHC_CHN0
elif [ "$BUILD_COMMAND" == "gts4llte_eur_open" ]; then
	PRODUCT_NAME=gts4lltexx
    SIGN_MODEL=SM-T835_EUR_XX_EUR0
elif [ "$BUILD_COMMAND" == "gts4lwifi_eur_open" ]; then
    PRODUCT_NAME=gts4lwifixx
    SIGN_MODEL=SM-N9500_EUR_XX_EUR0
else
	#default product
    PRODUCT_NAME=dream2qltechn
    SIGN_MODEL=SM-G9550_CHN_CHC_CHN0
fi

# Default Python version is 2.7
mkdir -p bin
ln -sf /usr/bin/python2.7 ./bin/python
export PATH=$(pwd)/bin:$PATH

while getopts "w:t:" flag; do
	case $flag in
		w)
			BUILD_OPTION_HW_REVISION=$OPTARG
			echo "-w : "$BUILD_OPTION_HW_REVISION""
			;;
		t)
			TARGET_BUILD_VARIANT=$OPTARG
			echo "-t : "$TARGET_BUILD_VARIANT""
			;;
		*)
			echo "wrong 2nd param : "$OPTARG""
			exit -1
			;;
	esac
done

shift $((OPTIND-1))

PROJECT_NAME=${BUILD_COMMAND%%_*}
TEMP=${BUILD_COMMAND#*_}
REGION=${TEMP%%_*}
CARRIER=${TEMP##*_}

if [ "$BUILD_OPTION_HW_REVISION" == "" ] ; then
    KERNEL_DEFCONFIG=${BUILD_COMMAND}_defconfig
else
    KERNEL_DEFCONFIG=${BUILD_COMMAND}_rev${BUILD_OPTION_HW_REVISION}_defconfig
fi

FUNC_CLEAN_DTB()
{
	if ! [ -d $BUILD_KERNEL_OUT_DIR/arch/arm64/boot/dts/samsung ] ; then
		echo "no directory : "$BUILD_KERNEL_OUT_DIR/arch/arm64/boot/dts/samsung""
	else
		echo "rm files in : "$BUILD_KERNEL_OUT_DIR/arch/arm64/boot/dts/samsung/*.dtb""
		rm $BUILD_KERNEL_OUT_DIR/arch/arm64/boot/dts/samsung/*.dtb
	fi
}

FUNC_BUILD_CLEAN()
{
    make clean
    make mrproper
}

FUNC_BUILD_KERNEL()
{
    # Checking parameters
    if [ "$BUILD_COMMAND" == "" ]; then
        SECFUNC_PRINT_HELP;
        exit -1;
    fi

    if [ "$CLEAN_OPTION" == "-c" ]; then
        FUNC_BUILD_CLEAN;
    fi

	echo ""
	echo "=============================================="
	echo "START : FUNC_BUILD_KERNEL"
	echo "=============================================="
	echo ""
	echo "build project="$PROJECT_NAME""
    echo "build model="$SIGN_MODEL""
	echo "build common config="$KERNEL_DEFCONFIG""

    if [ ! -d "$BUILD_KERNEL_OUT_DIR" ]; then
        mkdir $BUILD_KERNEL_OUT_DIR
        chmod -R 777 $BUILD_KERNEL_OUT_DIR
    fi

	FUNC_CLEAN_DTB

    make -C $BUILD_WHERE O=$BUILD_KERNEL_OUT_DIR -j$BUILD_JOB_NUMBER ARCH=$ARCH CROSS_COMPILE=$BUILD_CROSS_COMPILE KCFLAGS=-mno-android  $KERNEL_DEFCONFIG
    make -C $BUILD_WHERE O=$BUILD_KERNEL_OUT_DIR -j$BUILD_JOB_NUMBER ARCH=$ARCH CROSS_COMPILE=$BUILD_CROSS_COMPILE KCFLAGS=-mno-android

	echo ""
	echo "================================="
	echo "END   : FUNC_BUILD_KERNEL"
	echo "================================="
	echo ""
}

SECFUNC_PRINT_HELP()
{
	echo -e '\E[33m'
	echo "Help"
	echo "$0 \$1 \$2 \$3"
	echo "  \$1 : "
	echo "      dreamqlte_chn_open"
	echo "      dream2qlte_chn_open"
	echo "      greatqlte_chn_open"
	echo "      gts4llte_eur_open"
	echo "      gts4lwifi_eur_open"
	echo "  \$2 : "
	echo "      -c or Nothing  (-c : clean work dir)"
	echo -e '\E[0m'
}

# MAIN FUNCTION
rm -rf ./build.log
(
	START_TIME=`date +%s`

	FUNC_BUILD_KERNEL

	#FUNC_MKBOOTIMG

	END_TIME=`date +%s`

	let "ELAPSED_TIME=$END_TIME-$START_TIME"
	echo "Total compile time is $ELAPSED_TIME seconds"
) 2>&1	 | tee -a ./build.log