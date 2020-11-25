FROM gcc:5
RUN dpkg --add-architecture i386

RUN apt-get update
RUN apt-get install -y libssl-dev:i386 libstdc++5:i386
# install CMake
ADD https://github.com/Kitware/CMake/releases/download/v3.19.0/cmake-3.19.0-Linux-x86_64.sh .
RUN chmod +x ./cmake-3.19.0-Linux-x86_64.sh && \
    ./cmake-3.19.0-Linux-x86_64.sh --prefix=/usr/local/ --exclude-subdir && \
    rm cmake-3.19.0-Linux-x86_64.sh
# install Boost
ADD https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz .
RUN tar -xzf boost_1_72_0.tar.gz && \
    rm boost_1_72_0.tar.gz && \
    cd boost_1_72_0 && \
    ./bootstrap.sh && \
    ./b2 address-model=32 --with-thread --with-chrono install && \
    cd .. && \
    rm -rf boost_1_72_0
