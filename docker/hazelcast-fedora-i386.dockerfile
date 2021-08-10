FROM fedora:34

RUN dnf groups install -y "Development Tools" && \
    dnf install -y gcc-c++ gdb openssl-devel cmake valgrind rsync passwd openssh-server ninja-build \
                   java-1.8.0-openjdk glibc-devel.i686 glibc-devel libstdc++.i686 maven net-tools gcovr \
                   boost-devel.i686 thrift-devel.i686
