FROM gcc:5.1
RUN apt-get update && \
    apt-get install -y libssl-dev wget openssh-server
# install CMake
RUN wget --quiet https://github.com/Kitware/CMake/releases/download/v3.19.0/cmake-3.19.0-Linux-x86_64.sh && \
    chmod +x ./cmake-3.19.0-Linux-x86_64.sh && \
    ./cmake-3.19.0-Linux-x86_64.sh --prefix=/usr/local/ --exclude-subdir && \
    rm cmake-3.19.0-Linux-x86_64.sh
# install Boost
ADD https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz .
RUN tar -xzf boost_1_72_0.tar.gz && \
    rm boost_1_72_0.tar.gz && \
    cd boost_1_72_0 && \
    ./bootstrap.sh && \
    ./b2 --with-thread --with-chrono --with-regex install && \
    cd .. && \
    rm -rf boost_1_72_0

RUN useradd -m user && yes password | passwd user
RUN service ssh start