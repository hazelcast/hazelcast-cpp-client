FROM fedora:latest

# needed the following line since with earlier docker versions the build gives the following error:
# Error: Transaction test error:
#     file /usr/share/gcc-10/python/libstdcxx/__pycache__/__init__.cpython-39.opt-1.pyc from install of libstdc++-10.2.1-9.fc33.i686 conflicts with file from package libstdc++-10.2.1-6.fc33.x86_64
RUN dnf clean packages
RUN dnf groups install -y "Development Tools"
RUN dnf install -y gcc-c++ gdb compat-openssl10-devel.i686 cmake valgrind rsync passwd openssh-server ninja-build java-1.8.0-openjdk

#install 32-bit libraries
RUN dnf -y install glibc-devel.i686 glibc-devel libstdc++.i686

# needed for test
RUN dnf install -y maven net-tools gcovr
RUN dnf install -y fedora-repos-rawhide
RUN dnf --disablerepo=* --enablerepo=rawhide --nogpg install -y thrift-devel.i686

RUN dnf install -y wget bzip2
RUN dnf install -y wget bzip2 && wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.bz2 && tar xjf boost_1_72_0.tar.bz2 && rm boost_1_72_0.tar.bz2 && cd boost_1_72_0 && ./bootstrap.sh && ./b2 address-model=32 --with-thread --with-chrono install && cd .. && rm -rf boost_1_72_0


