cmake_minimum_required(VERSION 3.30)
project(PROBANTE VERSION 0.1.0 LANGUAGES CXX)
cmake_policy(SET CMP0169 OLD)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)
find_package(Threads REQUIRED)
find_package(Boost 1.90 QUIET COMPONENTS json)
if(NOT Boost_FOUND)
  FetchContent_Declare(boost_json
    URL https://archives.boost.io/release/1.90.0/source/boost_1_90_0.tar.bz2
    URL_HASH SHA256=49551aff3b22cbc5c5a9ed3dbc92f0e23ea50a0f7325b0d198b705e8ee3fc305)
  FetchContent_GetProperties(boost_json)
  if(NOT boost_json_POPULATED)
    FetchContent_Populate(boost_json)
  endif()
  add_library(boost_json_local STATIC src/boost_json.cpp)
  target_include_directories(boost_json_local SYSTEM PUBLIC ${boost_json_SOURCE_DIR})
  add_library(Boost::json ALIAS boost_json_local)
endif()

add_executable(probante
  src/main.cpp src/contract.cpp src/executor.cpp src/oracle.cpp src/evidence.cpp)
target_include_directories(probante PRIVATE src)
target_link_libraries(probante PRIVATE Boost::json OpenSSL::Crypto Threads::Threads)
target_compile_options(probante PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)

enable_testing()
add_subdirectory(tests/fixtures)
add_subdirectory(tests/qualification)
