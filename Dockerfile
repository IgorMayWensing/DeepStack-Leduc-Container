FROM ubuntu:18.04

# Update and install all dependencies in a single step
RUN apt-get update && apt-get install -y \
    git \
    curl \
    build-essential \
    cmake \
    python3 \
    sudo \
    nano \
    libjson-c-dev \
 && apt-get autoremove -y && apt-get clean && rm -rf /var/lib/apt/lists/*

# Clone, modify and install Torch
RUN git clone https://github.com/torch/distro.git /root/torch --recursive \
 && sed -i 's/python-software-properties/software-properties-common/g' /root/torch/install-deps \
 && cd /root/torch && bash install-deps && ./install.sh

# Set environment variables
ENV LUA_CPATH='/root/torch/install/lib/?.so;'$LUA_CPATH \
    PATH=$PATH:/root/torch/install/bin \
    LD_LIBRARY_PATH=/root/torch/install/lib:$LD_LIBRARY_PATH \
    DYLD_LIBRARY_PATH=/root/torch/install/lib:$DYLD_LIBRARY_PATH

# Install luasocket
RUN luarocks install luasocket

# Set the working directory
WORKDIR /app

# Copy the current directory contents into the container
# This is now the last step before the CMD, so Docker can cache all the previous layers.
COPY . /app

# Set default command
CMD [ "/bin/bash" ]

# Optionally, include metadata as described above
LABEL maintainer="igormaywensing@gmail.com" \
      version="1.0"
