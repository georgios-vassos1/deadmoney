cmake_minimum_required(VERSION 3.14)
project(holdem VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
enable_testing()

# Sources accumulate as we implement components via TDD.
add_library(holdem_core STATIC src/placeholder.cpp)
target_include_directories(holdem_core PUBLIC include)
target_compile_options(holdem_core PRIVATE -Wall -Wextra -O2)

function(add_holdem_test name)
  add_executable(${name} tests/${name}.cpp)
  target_link_libraries(${name} PRIVATE holdem_core GTest::gtest_main)
  target_compile_options(${name} PRIVATE -Wall -Wextra -O2)
  gtest_discover_tests(${name})
endfunction()

add_holdem_test(card_test)
