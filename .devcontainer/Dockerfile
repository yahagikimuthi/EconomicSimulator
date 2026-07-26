FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# 1. 必要なパッケージおよび vcpkg の依存ツールを一括インストール
RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    gnupg \
    build-essential \
    ninja-build \
    python3 \
    python3-pip \
    clang-format \
    git \
    curl \
    tar \
    unzip \
    zip \
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

# 3. Pythonパッケージと CMake (3.30以上) のインストール
RUN pip install --no-cache-dir --break-system-packages \
    "cmake>=3.30.0" \
    h5py \
    numpy \
    pandas \
    matplotlib \
    seaborn \
    scipy \
    pyarrow

# 4. vcpkg のインストールとビルド
ENV VCPKG_ROOT=/usr/local/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

RUN if [ ! -d "$VCPKG_ROOT" ]; then \
        git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT; \
    fi && \
    $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

# 5. vcpkg でライブラリをインストール
RUN vcpkg install tbb entt highfive