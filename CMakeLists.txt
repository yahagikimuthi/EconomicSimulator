cmake_minimum_required(VERSION 3.25)
project(EconomicSimulator LANGUAGES CXX)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# --- clangd / LSP 用の設定 ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- C++ 標準の設定 ---
set(CMAKE_CXX_STANDARD 23 CACHE STRING "Force C++23" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Force C++23 required" FORCE)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）

# --- 依存パッケージの検索 ---
find_package(HighFive REQUIRED)
find_package(TBB REQUIRED)

# --- FetchContent による外部ライブラリの取得 ---
include(FetchContent)
FetchContent_Declare(
    pcg_cpp
    GIT_REPOSITORY https://github.com/imneme/pcg-cpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(pcg_cpp)

# --- ソースファイルの収集 ---
file(GLOB_RECURSE ALL_SRC_SOURCES "src/*.cpp")

# テスト実行ファイルで main() が重複しないよう、src/main.cpp を特定して分離
set(MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
set(CORE_SOURCES ${ALL_SRC_SOURCES})
list(REMOVE_ITEM CORE_SOURCES ${MAIN_SOURCE})

# --- 共通設定を適用するヘルパー関数 ---
function(apply_common_settings TARGET_NAME)
    # インクルードディレクトリの指定 (include/ に doctest.h があるため自動的に参照されます)
    target_include_directories(${TARGET_NAME} SYSTEM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/doctest
        ${pcg_cpp_SOURCE_DIR}/include
    )

    # 依存ライブラリのリンク
    target_link_libraries(${TARGET_NAME} PRIVATE
        TBB::tbb
        HighFive::HighFive
    )

    # コンパイル定義
    add_compile_definitions(HIGHFIVE_HAS_NO_EXCEPTIONS)

    # 厳格なコンパイル・警告オプション
    target_compile_options(${TARGET_NAME} PRIVATE
        -O0
        -g
        -Wall
        -Werror
        -Wextra
        -Wshadow
        -Wconversion
        -Wsign-conversion     # 符号あり/なしの変換も徹底的に弾く
        -Wpedantic            # ISO C++規格に違反したコードを禁止
        -Wunreachable-code    # 実行されることのないデッドコードを禁止
        -Wimplicit-fallthrough# switch文のbreak忘れを禁止
        -D_GLIBCXX_DEBUG      # operator[]の境界チェック
    )

    # サニタイザ（必要に応じてコメント解除）
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET_NAME} PRIVATE 
            #-fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
            #-fno-omit-frame-pointer
        )
        target_link_options(${TARGET_NAME} PRIVATE 
            #-fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
        )
    endif()
endfunction()

# ==============================================================================
# 1. 本体シミュレーション実行ファイル
# ==============================================================================
add_executable(EconomicSimulator ${ALL_SRC_SOURCES})
apply_common_settings(EconomicSimulator)

# ==============================================================================
# 2. テスト環境 (doctest)
# ==============================================================================
enable_testing()

# tests/ 配下の全テストソースコードを取得
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

# テスト実行ファイルの定義 (src/main.cpp 以外のコアソース + tests/*.cpp)
add_executable(run_tests ${CORE_SOURCES} ${TEST_SOURCES})

# 共通設定の適用
apply_common_settings(run_tests)

# ctest 連携
add_test(NAME AllTests COMMAND run_tests)