FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

# 1. Debian sid (実験的リポジトリ) から GCC 14 と C++用標準ライブラリ一式をインストール
RUN echo "deb http://deb.debian.org/debian sid main" >> /etc/apt/sources.list && \
    apt-get update && apt-get install -y --no-install-recommends \
    gcc-14 \
    g++-14 \
    libstdc++-14-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 2. システム全体のデフォルトコンパイラ(gcc/g++)を GCC 14 に強制切り替え
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-14

# 3. vcpkgが新しいGCC 14を使ってライブラリをビルド・認識するように、キャッシュを考慮してインストール
# (一回インストール済みのバイナリを新しいコンパイラ用にクリアするため、あえて既存のインストール先を一度消しています)
RUN rm -rf /usr/local/vpkg/installed && \
    /usr/local/vcpkg/vcpkg install tbb entt highfive