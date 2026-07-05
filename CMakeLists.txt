cmake_minimum_required(VERSION 3.14)
project(EconomicSimulator LANGUAGES CXX)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# --- clangd / LSP 用の設定 ---
# これによりビルドディレクトリに compile_commands.json が出力されます
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- C++ 標準の設定 ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）

# 1. WSLシステム内の HDF5 ライブラリを自動探索
find_package(HDF5 REQUIRED COMPONENTS CXX)
find_package(TBB REQUIRED)

# --- ✨ ここを追加 ✨ ---
# HighFiveにBoostを探しに行かせないように設定
set(HIGHFIVE_USE_BOOST OFF CACHE INTERNAL "Disable Boost for HighFive")
# ------------------------

# 2. HighFive（ヘッダーオンリー）をCMakeプロジェクトに自動ダウンロード・組み込み
include(FetchContent)
FetchContent_Declare(
    HighFive
    GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
    GIT_TAG        v2.10.0 # 安定版のタグを指定
)
FetchContent_MakeAvailable(HighFive)


# --- ソースファイルの収集 ---
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 実行ファイルの定義
add_executable(EconomicSimulator ${SOURCES})
target_link_libraries(EconomicSimulator PRIVATE HighFive HDF5::HDF5)
target_link_libraries(EconomicSimulator PRIVATE TBB::tbb)

# インクルードディレクトリの指定
target_include_directories(EconomicSimulator SYSTEM PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_compile_definitions(HIGHFIVE_HAS_NO_EXCEPTIONS)
# --- コンパイル・リンクオプション（警告） ---
target_compile_options(EconomicSimulator PRIVATE
    -O0
    -g
    -Wall
    -Werror
    -Wextra
    -Wshadow
    -Wconversion
    -Wsign-conversion # 符号あり/なしの変換も徹底的に弾く
    -Wpedantic        # ISO C++規格に違反したコードを禁止
    -Wunreachable-code# 実行されることのないデッドコードを禁止
    -Wimplicit-fallthrough # switch文のbreak忘れを禁止
)