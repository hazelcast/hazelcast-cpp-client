FROM fedora:22

RUN dnf groups install -y "Development Tools"
RUN dnf install -y gcc-c++ openssl-devel cmake tar wget bzip2 java-17-openjdk

# install boost
RUN wget --quiet https://boostorg.jfrog.io/artifactory/main/release/1.72.0/source/boost_1_72_0.tar.bz2 && tar xjf boost_1_72_0.tar.bz2 && rm boost_1_72_0.tar.bz2 && cd boost_1_72_0 && ./bootstrap.sh && ./b2 address-model=64 --with-thread --with-chrono install && cd .. && rm -rf boost_1_72_0

RUN wget --quiet https://github.com/Kitware/CMake/releases/download/v3.19.0/cmake-3.19.0-Linux-x86_64.sh && \
    chmod +x ./cmake-3.19.0-Linux-x86_64.sh && \
    ./cmake-3.19.0-Linux-x86_64.sh --prefix=/usr/local/ --exclude-subdir && \
    rm cmake-3.19.0-Linux-x86_64.sh
