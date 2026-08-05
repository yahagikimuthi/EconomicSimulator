cmake_minimum_required(VERSION 3.30)

project(EconomicSimulator CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CTest（自動テスト）の有効化
enable_testing()

find_package(HighFive REQUIRED)
find_package(TBB REQUIRED)
find_package(Git REQUIRED)

include(FetchContent)
FetchContent_Declare(
    pcg_cpp
    GIT_REPOSITORY https://github.com/imneme/pcg-cpp.git
    GIT_TAG        master # または特定のコミットハッシュやタグ
)
FetchContent_MakeAvailable(pcg_cpp)

# --- ソースファイルの取得 ---
# メイン実行ファイル用ソース
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "src/app_main.cpp")

# メインとテストで共有する実装ソース（main.cppを除外）
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES ${MAIN_SOURCES})

# テスト用ソース
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

# --- 共通コンパイル/リンク設定をモジュール化 ---
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -O0
    -g
    -Wall
    -Werror
    -Wextra
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wpedantic
    -Wunreachable-code
    -Wimplicit-fallthrough
    -D_GLIBCXX_DEBUG
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fno-omit-frame-pointer
    -fcontracts
)
target_link_options(project_warnings INTERFACE
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fcontracts
)

# --- 1. メイン実行ファイル ターゲット ---
add_executable(EconomicSimulator)
target_sources(EconomicSimulator PRIVATE ${MAIN_SOURCES} ${LIB_SOURCES})
target_link_libraries(EconomicSimulator PRIVATE
    TBB::tbb
    HighFive::HighFive
    project_warnings
)
target_include_directories(EconomicSimulator SYSTEM PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${pcg_cpp_SOURCE_DIR}/include
)
add_compile_definitions(HIGHFIVE_HAS_NO_EXCEPTIONS)

# --- 2. テスト用 ターゲット ---
add_executable(EconomicSimulator_test)
target_sources(EconomicSimulator_test PRIVATE ${LIB_SOURCES} ${TEST_SOURCES})
target_link_libraries(EconomicSimulator_test PRIVATE
    TBB::tbb
    HighFive::HighFive
    project_warnings
)
target_include_directories(EconomicSimulator_test SYSTEM PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${pcg_cpp_SOURCE_DIR}/include
)

# CTestにテストターゲットを登録
add_test(NAME UnitTests COMMAND EconomicSimulator_test)