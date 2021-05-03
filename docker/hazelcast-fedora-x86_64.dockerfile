FROM fedora:latest

RUN dnf groups install -y "Development Tools"
RUN dnf install -y gcc-c++ gdb compat-openssl10-devel.x86_64 cmake valgrind java-1.8.0-openjdk.x86_64 rsync passwd openssh-server ninja-build

# needed for test
RUN dnf install -y maven net-tools gcovr
RUN dnf install -y fedora-repos-rawhide
RUN dnf --disablerepo=* --enablerepo=rawhide --nogpg install -y thrift-devel

# install boost
RUN dnf install -y wget bzip2 && wget --quiet https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.bz2 && tar xjf boost_1_76_0.tar.bz2 && rm boost_1_76_0.tar.bz2 && cd boost_1_76_0 && ./bootstrap.sh && ./b2 address-model=64 --with-thread --with-chrono install && cd .. && rm -rf boost_1_76_0

RUN ssh-keygen -A

RUN ( \
    echo 'LogLevel DEBUG2'; \
    echo 'PermitRootLogin yes'; \
    echo 'PasswordAuthentication yes'; \
    echo 'Subsystem sftp /usr/libexec/openssh/sftp-server'; \
  ) > /etc/ssh/sshd_config_test_clion

RUN useradd -m user \
  && yes password | passwd user

CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_test_clion"]
