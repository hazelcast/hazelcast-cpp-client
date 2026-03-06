# Use latest Ubuntu LTS
FROM ubuntu:24.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install all dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    gdb \
    cmake \
    build-essential \
    valgrind \
    libssl-dev \
    cmake \
    openjdk-17-jdk \
    rsync \
    passwd \
    openssh-server \
    ninja-build \
    maven \
    net-tools \
    gcovr \
    libboost-all-dev \
    libthrift-dev \
    libgtest-dev \
    libgmock-dev \
    && rm -rf /var/lib/apt/lists/*

# Set JAVA_HOME
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
ENV PATH="${JAVA_HOME}/bin:${PATH}"

# Optional: Configure SSH
RUN mkdir /var/run/sshd && \
    echo 'root:password' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

WORKDIR /workspace/hazelcast-cpp-client

CMD ["/bin/bash"]