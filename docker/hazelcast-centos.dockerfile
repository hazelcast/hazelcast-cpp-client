FROM centos:latest

RUN yum groups install -y "Development Tools"
RUN yum install -y cmake
RUN yum install -y maven

RUN yum install -y wget
RUN wget https://www.openssl.org/source/openssl-1.0.2-latest.tar.gz
RUN tar xvfz openssl-1.0.2-latest.tar.gz
RUN cd openssl-1.0.2u
RUN ./config
RUN make
RUN make install

RUN yum install -y thrift-devel

RUN yum install -y zsh
RUN sh -c "$(curl -fsSL https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
