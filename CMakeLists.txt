cmake_minimum_required(VERSION 3.25)
project(EconomicSimulator LANGUAGES CXX)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# --- clangd / LSP 用の設定 ---
# これによりビルドディレクトリに compile_commands.json が出力
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- C++ 標準の設定 ---
set(CMAKE_CXX_STANDARD 23 CACHE STRING "Force C++23" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Force C++23 required" FORCE)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）

find_package(HighFive REQUIRED)
find_package(TBB REQUIRED)

# --- ソースファイルの収集 ---
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 実行ファイルの定義
add_executable(EconomicSimulator ${SOURCES})
target_link_libraries(EconomicSimulator PRIVATE TBB::tbb)
target_link_libraries(EconomicSimulator PRIVATE HighFive::HighFive)

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

# ターゲット（EconomicSimulator）に対して確実にサニタイザを紐付ける
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(EconomicSimulator PRIVATE 
        -fsanitize=address 
        -fsanitize=undefined
    )
    target_link_options(EconomicSimulator PRIVATE 
        -fsanitize=address 
        -fsanitize=undefined
    )
endif()