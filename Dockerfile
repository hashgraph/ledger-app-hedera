# Dockerfile intended to be used as a build 
# environment for Ledger applications

FROM ubuntu:19.10

# Cross compilation headers are required
RUN apt-get update && \
    apt-get -y install gcc-multilib g++-multilib \
                       wget xz-utils \
                       python3-dev python3 python3-pip \
                       libudev-dev \
                       libusb-1.0-0-dev \
                       libtinfo5

RUN mkdir -p /opt/ledger/nanox && \
    mkdir -p /opt/ledger/others

RUN wget https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q1-update/+download/gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 && \
    tar xf gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 && \
    rm gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 && \
    cp -r gcc-arm-none-eabi-5_3-2016q1 /opt/ledger/nanox/gcc-arm-none-eabi-5_3-2016q1 && \
    mv gcc-arm-none-eabi-5_3-2016q1 /opt/ledger/others/gcc-arm-none-eabi-5_3-2016q1

RUN wget http://releases.llvm.org/4.0.0/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.10.tar.xz -O clang+llvm.tar.xz && \
    tar xf clang+llvm.tar.xz && \
    rm clang+llvm.tar.xz && \
    mv clang+llvm* /opt/ledger/others/clang-arm-fropi

RUN wget http://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz -O clang+llvm.tar.xz && \
    tar xf clang+llvm.tar.xz && \
    rm clang+llvm.tar.xz && \
    mv clang+llvm* /opt/ledger/nanox/clang-arm-fropi

RUN pip3 install ledgerblue

COPY x.py /opt/x.py

ENTRYPOINT [ "/opt/x.py" ]
WORKDIR /workspace
