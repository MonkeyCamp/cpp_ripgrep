cmake_minimum_required(VERSION 3.16)
project(cpp_ripgrep VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch PCRE2
FetchContent_Declare(
    pcre2
    GIT_REPOSITORY https://github.com/PhilipHazel/pcre2.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(pcre2)

# Fetch Abseil (required by RE2)
set(ABSL_ENABLE_INSTALL ON)
FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(absl)


# Fetch RE2 (disable install rules)
set(RE2_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(re2)

# Find required packages
find_package(Threads REQUIRED)
# find_package(absl REQUIRED)

include_directories(include)

# Compiler flags for optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")

# Source files
set(SOURCES
    src/main.cpp
    src/grep_engine.cpp
    src/file_scanner.cpp
    src/gitignore.cpp
    src/regex_matcher.cpp
    src/re2_matcher.cpp
    src/options.cpp
)

# Create executable
add_executable(cpp_ripgrep ${SOURCES})

# Test sources
set(TEST_SOURCES
    tests/test_main.cpp
    src/grep_engine.cpp
    src/file_scanner.cpp
    src/gitignore.cpp
    src/regex_matcher.cpp
    src/re2_matcher.cpp
    src/options.cpp
)

# Create test executable
add_executable(run_tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(cpp_ripgrep Threads::Threads)
target_link_libraries(run_tests Threads::Threads)

# Link PCRE2
target_link_libraries(cpp_ripgrep pcre2-8)
target_link_libraries(run_tests pcre2-8)
target_compile_definitions(cpp_ripgrep PRIVATE HAVE_PCRE2)
target_compile_definitions(run_tests PRIVATE HAVE_PCRE2)

# Link RE2
target_link_libraries(cpp_ripgrep re2)
target_link_libraries(run_tests re2)
target_compile_definitions(cpp_ripgrep PRIVATE HAVE_RE2)
target_compile_definitions(run_tests PRIVATE HAVE_RE2)

# Install target
install(TARGETS cpp_ripgrep DESTINATION bin)
install(TARGETS run_tests DESTINATION bin)

# Testing
enable_testing()
add_test(NAME gitignore_test COMMAND run_tests)