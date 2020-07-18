FROM ubuntu:18.04

RUN apt update -y > /dev/null 2> /dev/null

RUN apt install python make bc git gcc ccache -y

WORKDIR /home/kernel

RUN mkdir -p ../PLATFORM/prebuilts/gcc/linux-x86/aarch64

RUN git clone  https://github.com/djb77/aarch64-linux-android-4.9 ../PLATFORM/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9

COPY ./ ./

CMD ["bash"]

