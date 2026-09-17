FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# 1. 基本ツールと PPA から GCC 16 を導入
RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    gnupg \
    build-essential \
    ninja-build \
    gdb \
    lldb \
    clang-format \
    git \
    curl \
    ca-certificates \
    tar \
    unzip \
    zip \
    wget \
    pkg-config \
    libtbb-dev \
    && add-apt-repository ppa:ubuntu-toolchain-r/test -y \
    && apt-get update && apt-get install -y --no-install-recommends \
    gcc-16 \
    g++-16 \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 2. デフォルトコンパイラを GCC 16 に設定
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100

# 3. Kitware から最新の CMake をインストール
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update && apt-get install -y --no-install-recommends cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 4. EnTT (Header-only) の導入
RUN git clone https://github.com/skypjack/entt.git /tmp/entt \
    && cmake -S /tmp/entt -B /tmp/entt/build \
    && cmake --install /tmp/entt/build \
    && rm -rf /tmp/entt

WORKDIR /opt
RUN git clone https://github.com/emscripten-core/emsdk.git
WORKDIR /opt/emsdk
RUN ./emsdk install latest && ./emsdk activate latest

WORKDIR /app

CMD ["tail", "-f", "/dev/null"]