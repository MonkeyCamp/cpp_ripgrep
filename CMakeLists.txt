cmake_minimum_required(VERSION 3.16)
project(cpp_ripgrep VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cross-compilation support
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_NAME}")
    set(IS_WINDOWS TRUE)
else()
    set(IS_WINDOWS FALSE)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Platform-specific package finding
if(IS_WINDOWS)
    # For Windows cross-compilation, we'll use bundled libraries or system paths
    set(PCRE2_INCLUDE_DIRS "")
    set(PCRE2_LIBRARIES "")
    set(PCRE2_CFLAGS_OTHER "")
    
    # Try to find PCRE2 in common Windows locations
    find_path(PCRE2_INCLUDE_DIRS NAMES pcre2.h
        PATHS
        /usr/x86_64-w64-mingw32/include
        /usr/i686-w64-mingw32/include
        /opt/mingw64/include
        /opt/mingw32/include
    )
    
    find_library(PCRE2_LIBRARIES NAMES pcre2-8
        PATHS
        /usr/x86_64-w64-mingw32/lib
        /usr/i686-w64-mingw32/lib
        /opt/mingw64/lib
        /opt/mingw32/lib
    )
    
    if(NOT PCRE2_LIBRARIES)
        message(WARNING "PCRE2 not found for Windows cross-compilation - building without PCRE2 support")
        set(PCRE2_LIBRARIES "")
        set(PCRE2_INCLUDE_DIRS "")
    endif()
else()
    find_package(PkgConfig REQUIRED)
    
    # Try to find PCRE2 for better regex performance
    pkg_check_modules(PCRE2 REQUIRED libpcre2-8)
    
    # Try to find RE2 as an alternative regex engine
    find_package(PkgConfig QUIET)
    pkg_check_modules(RE2 QUIET re2)
endif()

# Include directories
if(PCRE2_INCLUDE_DIRS)
    include_directories(${PCRE2_INCLUDE_DIRS})
endif()
if(RE2_FOUND)
    include_directories(${RE2_INCLUDE_DIRS})
endif()
include_directories(include)

# Compiler flags for optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")

# Source files
set(SOURCES
    src/main.cpp
    src/grep_engine.cpp
    src/file_scanner.cpp
    src/regex_matcher.cpp
    src/re2_matcher.cpp
    src/options.cpp
)

# Create executable
add_executable(cpp_ripgrep ${SOURCES})

# Link libraries
target_link_libraries(cpp_ripgrep Threads::Threads)

# Add PCRE2 if found
if(PCRE2_LIBRARIES)
    target_link_libraries(cpp_ripgrep ${PCRE2_LIBRARIES})
    target_compile_definitions(cpp_ripgrep PRIVATE HAVE_PCRE2)
endif()

# Add RE2 if found
if(RE2_FOUND)
    target_link_libraries(cpp_ripgrep ${RE2_LIBRARIES})
    target_compile_options(cpp_ripgrep PRIVATE ${RE2_CFLAGS_OTHER})
    target_compile_definitions(cpp_ripgrep PRIVATE HAVE_RE2)
endif()

# Set compiler flags
if(PCRE2_CFLAGS_OTHER)
    target_compile_options(cpp_ripgrep PRIVATE ${PCRE2_CFLAGS_OTHER})
endif()

# Install target
install(TARGETS cpp_ripgrep DESTINATION bin) 