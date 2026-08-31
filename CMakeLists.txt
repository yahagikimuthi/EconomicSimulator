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
    GIT_TAG        master
)
FetchContent_MakeAvailable(pcg_cpp)

# --- ソースファイルの取得 ---
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "src/main.cpp")

# モジュールインターフェースファイル (.cppm) を include/ から再帰取得
file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS "include/*.cppm")

# 実装ソース (.cpp)
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES ${MAIN_SOURCES})

# テスト用ソース
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

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
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fno-omit-frame-pointer
    -fcontracts
    -D_GLIBCXX_ASSERTIONS
    -march=native
    -flto
)
target_link_options(project_warnings INTERFACE
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fcontracts
)

# モジュール・共有ロジック用ライブラリ
add_library(EconomicSimulator_lib STATIC)

# C++ モジュールインターフェースファイルの登録
target_sources(EconomicSimulator_lib
    PRIVATE
        ${LIB_SOURCES}
    PUBLIC
        FILE_SET CXX_MODULES TYPE CXX_MODULES FILES ${MODULE_SOURCES}
)

target_link_libraries(EconomicSimulator_lib PUBLIC
    TBB::tbb
    HighFive::HighFive
    project_warnings
)

target_include_directories(EconomicSimulator_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${pcg_cpp_SOURCE_DIR}/include
)

target_compile_definitions(EconomicSimulator_lib PUBLIC HIGHFIVE_HAS_NO_EXCEPTIONS)

# メイン実行ファイル ターゲット
add_executable(EconomicSimulator)
target_sources(EconomicSimulator PRIVATE ${MAIN_SOURCES})
target_link_libraries(EconomicSimulator PRIVATE EconomicSimulator_lib)

# テスト用 ターゲット
add_executable(EconomicSimulator_test)
target_sources(EconomicSimulator_test PRIVATE ${TEST_SOURCES})
target_link_libraries(EconomicSimulator_test PRIVATE EconomicSimulator_lib)
target_include_directories(EconomicSimulator_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# CTestにテストターゲット
add_test(NAME UnitTests COMMAND EconomicSimulator_test)