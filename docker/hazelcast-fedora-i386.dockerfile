FROM fedora:latest

RUN dnf groups install -y "Development Tools"
RUN dnf install -y gcc-c++ gdb compat-openssl10-devel.i686 cmake valgrind rsync passwd openssh-server ninja-build java-1.8.0-openjdk

#install 32-bit libraries
RUN dnf -y install glibc-devel.i686 glibc-devel libstdc++.i686

# needed for test
RUN dnf install -y maven net-tools gcovr
RUN dnf install -y fedora-repos-rawhide
RUN dnf --disablerepo=* --enablerepo=rawhide --nogpg install -y thrift-devel.i686

RUN dnf install -y wget bzip2
RUN dnf install -y wget bzip2 && wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.bz2 && tar xjf boost_1_75_0.tar.bz2 && rm boost_1_75_0.tar.bz2 && cd boost_1_75_0 && ./bootstrap.sh && ./b2 address-model=32 --with-thread --with-chrono install && cd .. && rm -rf boost_1_75_0


