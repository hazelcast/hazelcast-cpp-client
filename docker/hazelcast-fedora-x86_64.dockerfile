FROM fedora:latest

RUN dnf groups install -y "Development Tools"
RUN dnf install -y gcc-c++ gdb compat-openssl10-devel.x86_64 cmake valgrind

# needed for test
RUN dnf install -y maven net-tools gcovr
RUN dnf install -y fedora-repos-rawhide
RUN dnf --disablerepo=* --enablerepo=rawhide --nogpg install -y thrift-devel

# install boost 1.72.0
RUN dnf install -y wget bzip2 && wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.bz2 && tar xjf boost_1_72_0.tar.bz2 && rm boost_1_72_0.tar.bz2 && cd boost_1_72_0 && ./bootstrap.sh && ./b2 address-model=64 --with-thread --with-chrono install && cd .. && rm -rf boost_1_72_0

