FROM ubuntu:24.04

WORKDIR /app

ENV DEBIAN_FRONTEND=noninteractive

# 1. 必要なパッケージおよび vcpkg の依存ツールを一括インストール
RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    gnupg \
    build-essential \
    ninja-build \
    gdb \
    gpg \
    lldb \
    clang-format \
    git \
    curl \
    ca-certificates \
    openssh-client \
    ca-certificates \
    tar \
    unzip \
    zip \
    wget \
    pkg-config \
    && add-apt-repository ppa:ubuntu-toolchain-r/test -y \
    && apt-get update && apt-get install -y --no-install-recommends \
    gcc-16 \
    g++-16 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 2. デフォルトコンパイラを GCC 16 に設定
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100

# 3. Kitware の GPG キーとリポジトリを追加 (Ubuntu 24.04: noble)
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null

# 4. 最新の CMake をインストール
RUN apt-get update && apt-get install -y cmake

# 5. vcpkg のインストールとビルド
ENV VCPKG_ROOT=/usr/local/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

RUN if [ ! -d "$VCPKG_ROOT" ]; then \
        git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT; \
    fi && \
    $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

# 6. vcpkg でライブラリをインストール
RUN vcpkg install tbb entt