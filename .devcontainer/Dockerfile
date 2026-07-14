FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

# 1. GCC 14 と C++標準ライブラリをインストール
RUN echo "deb http://deb.debian.org/debian sid main" >> /etc/apt/sources.list && \
    apt-get update && apt-get install -y --no-install-recommends \
    gcc-14 \
    g++-14 \
    libstdc++-14-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 2. デフォルトコンパイラ(gcc/g++)を GCC 14 に切り替え
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-14

# 3. vcpkgが新しいGCC 14を使ってライブラリをビルド・認識するように、キャッシュを考慮してインストール
RUN rm -rf /usr/local/vpkg/installed && \
    /usr/local/vcpkg/vcpkg install tbb entt highfive

# 4. clang-formatを追加
RUN apt-get update && apt-get install -y \
    clang-format \
    && rm -rf /var/lib/apt/lists/*

#5. python3, pip
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

#6. pythonライブラリ群 (--break-system-packages を追加)
RUN pip3 install --no-cache-dir --break-system-packages \
    h5py \
    numpy \
    pandas \
    matplotlib \
    seaborn