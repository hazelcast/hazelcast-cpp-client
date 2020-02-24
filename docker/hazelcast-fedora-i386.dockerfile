FROM fedora:latest

RUN dnf groups install -y "Development Tools"
RUN dnf install -y gcc-c++ gdb compat-openssl10-devel.i686 cmake

#install 32-bit libraries
RUN dnf -y install glibc-devel.i686 glibc-devel libstdc++.i686

# needed for test
RUN dnf install -y maven net-tools gcovr
RUN dnf install -y fedora-repos-rawhide
RUN dnf --disablerepo=* --enablerepo=rawhide --nogpg install -y thrift-devel.i686


