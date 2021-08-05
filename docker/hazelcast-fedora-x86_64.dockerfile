FROM fedora:33

RUN dnf groups install -y "Development Tools" && \
    dnf install -y gcc-c++ gdb compat-openssl10-devel.x86_64 cmake java-1.8.0-openjdk.x86_64 rsync passwd \
                   openssh-server ninja-build maven net-tools gcovr boost-devel thrift-devel

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
