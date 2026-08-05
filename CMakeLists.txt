cmake_minimum_required(VERSION 3.30)

project(EconomicSimulator CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(EconomicSimulator)

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

# ソースファイルを取得
file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "src/*.cpp"
)

# ターゲットへの登録
target_sources(EconomicSimulator PRIVATE
    ${SOURCES}
)

# ライブラリのリンク
target_link_libraries(EconomicSimulator PRIVATE
    TBB::tbb
    HighFive::HighFive
)

# インクルードディレクトリの指定
target_include_directories(EconomicSimulator SYSTEM PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${pcg_cpp_SOURCE_DIR}/include
)
add_compile_definitions(HIGHFIVE_HAS_NO_EXCEPTIONS)

# --- コンパイル・リンクオプション ---
target_compile_options(EconomicSimulator PRIVATE
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

target_link_options(EconomicSimulator PRIVATE 
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fcontracts
)