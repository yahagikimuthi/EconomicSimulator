list(APPEND CMAKE_PREFIX_PATH "/usr/local/vcpkg/installed/x64-linux")

cmake_minimum_required(VERSION 3.30)

project(EconomicSimulator CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CTest（自動テスト）の有効化
enable_testing()

find_package(TBB REQUIRED)
find_package(Git REQUIRED)

include(FetchContent)
FetchContent_Declare(
    pcg_cpp
    GIT_REPOSITORY https://github.com/imneme/pcg-cpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(pcg_cpp)

include(FetchContent)
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG        v1.2.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(Crow)

# --- ソースファイルの取得 ---
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

# 共通コンパイル/リンク設定
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -O0
    -Wall
    -Werror
    -Wextra
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wpedantic
    -Wunreachable-code
    -Wimplicit-fallthrough
    -Wnoexcept
    -D_GLIBCXX_DEBUG
    -D_GLIBCXX_ASSERTIONS
    -D_FORTIFY_SOURCE=2
    -fstack-protector-strong
    -fstack-clash-protection
    -fcf-protection
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fno-omit-frame-pointer
    -fcontracts
    -mtune=native
    -march=native
    -flto
)
target_link_options(project_warnings INTERFACE
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fcontracts
)

# --- ヘッダーオンリーライブラリの定義 ---
add_library(EconomicSimulator_lib INTERFACE)

# ヘッダーディレクトリおよび依存関係の追加（INTERFACE のため INTERFACE キーワードを使用）
target_include_directories(EconomicSimulator_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${pcg_cpp_SOURCE_DIR}/include
)

target_link_libraries(EconomicSimulator_lib INTERFACE
    TBB::tbb
)

# メイン実行ファイル ターゲット
add_executable(EconomicSimulator)
target_sources(EconomicSimulator PRIVATE ${MAIN_SOURCES})
target_link_libraries(EconomicSimulator PRIVATE
    EconomicSimulator_lib
    project_warnings
    Crow::Crow
)

target_include_directories(EconomicSimulator SYSTEM PRIVATE ${crow_SOURCE_DIR}/include)

# テスト用 ターゲット
add_executable(EconomicSimulator_test)
target_sources(EconomicSimulator_test PRIVATE ${TEST_SOURCES})
target_link_libraries(EconomicSimulator_test PRIVATE
    EconomicSimulator_lib
    project_warnings
    Crow::Crow
)
target_include_directories(EconomicSimulator_test SYSTEM PRIVATE ${crow_SOURCE_DIR}/include)

add_test(NAME UnitTests COMMAND EconomicSimulator_test)