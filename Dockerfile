FROM ubuntu:latest

LABEL description="Ledger Development Environment"
LABEL author="LaunchBadge"
LABEL version="1.0"

ARG USER_ID
ARG GROUP_ID

ARG CLANG_VERSION=4.0.0

RUN apt-get update && \
    apt-get -y install gcc-multilib g++-multilib \
                       wget xz-utils \
                       python3-dev python3 \
                       python3-pip python3-protobuf \
                       libudev-dev \
                       libusb-1.0-0-dev \
                       libtinfo5 \
                       clang-tidy \
                       clang-format \
                       protobuf-compiler

RUN mkdir -p /opt/ledger/env

RUN wget https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q1-update/+download/gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 && \
    tar xf gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 && \
    rm gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 && \
    cp -r gcc-arm-none-eabi-5_3-2016q1 /opt/ledger/env/gcc-arm-none-eabi-5_3-2016q1 && \
    rm -rf gcc-arm-none-eabi-5_3-2016q1

RUN wget http://releases.llvm.org/${CLANG_VERSION}/clang+llvm-${CLANG_VERSION}-x86_64-linux-gnu-ubuntu-16.04.tar.xz -O clang+llvm.tar.xz && \
    tar xf clang+llvm.tar.xz && \
    rm clang+llvm.tar.xz && \
    mv clang+llvm* /opt/ledger/env/clang-arm-fropi

RUN pip3 install protobuf
RUN pip3 install ledgerblue
RUN pip3 install Pillow

RUN rm -f /usr/bin/python && ln -s /usr/bin/python3 /usr/bin/python

COPY x.py /opt/ledger/x.py

RUN if [ ${USER_ID:-0} -ne 0 ] && [ ${GROUP_ID:-0} -ne 0 ]; \
    then \
        groupadd -g ${GROUP_ID} ledgerboi && \
        useradd -l -u ${USER_ID} -g ledgerboi ledgerboi && \
        install -d -m 0755 -o ledgerboi -g ledgerboi /home/ledgerboi && \
        chown --no-dereference --recursive \
        ${USER_ID}:${GROUP_ID} /home/ledgerboi /opt/ledger; \
    fi

USER ledgerboi
ENTRYPOINT [ "/opt/ledger/x.py" ]
WORKDIR /workspace
