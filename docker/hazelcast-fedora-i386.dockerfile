FROM fedora:33

RUN dnf groups install -y "Development Tools" && \
    dnf install -y gcc-c++ gdb compat-openssl10-devel.i686 cmake valgrind rsync passwd openssh-server ninja-build \
                   java-1.8.0-openjdk glibc-devel.i686 glibc-devel libstdc++.i686 maven net-tools gcovr \
                   boost-devel.i686 thrift-devel.i686
