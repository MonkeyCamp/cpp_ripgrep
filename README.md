# cpp_ripgrep

A high-performance grep-like tool written in C++ with similar performance characteristics to ripgrep.

## Features

- **Fast Pattern Matching**: Uses PCRE2 or RE2 for high-performance regex matching
- **Multiple Regex Engines**: Choose between PCRE2 (default) and Google RE2
- **Parallel Processing**: Multi-threaded file processing for optimal performance
- **Memory-Mapped I/O**: Efficient file reading using memory mapping
- **Unicode Support**: Full Unicode support through PCRE2 and RE2
- **Multiple Search Modes**: Literal, regex, and case-insensitive search
- **Recursive Directory Search**: Search through directories recursively
- **File Filtering**: Include/exclude patterns for file filtering
- **Color Output**: Colored output with configurable color settings
- **Binary File Detection**: Automatically skips binary files

## Performance Optimizations

- **Memory Mapping**: Uses `mmap()` for efficient file reading
- **Multi-threading**: Parallel file processing with configurable thread count
- **Optimized Regex Engine**: PCRE2 with JIT compilation support
- **Efficient String Matching**: Optimized literal string matching
- **Smart File Filtering**: Early filtering to avoid unnecessary processing

## Installation

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- PCRE2 development libraries (required)
- RE2 development libraries (optional, for RE2 support)

### Cross-Compilation for Windows

To build Windows executables from Linux:

```bash
# Install MinGW-w64 toolchain
sudo apt install gcc-mingw-w64-x86-64 gcc-mingw-w64-i686

# Build Windows executables
./scripts/build-windows.sh
```

This will create:
- `dist/cpp_ripgrep-x64.exe` - 64-bit Windows executable
- `dist/cpp_ripgrep-x86.exe` - 32-bit Windows executable (if toolchain available)
- `dist/cpp_ripgrep.bat` - Auto-detecting batch file

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake libpcre2-dev libre2-dev
```

### CentOS/RHEL/Fedora

```bash
sudo yum install -y gcc-c++ cmake pcre2-devel re2-devel
# or for Fedora:
sudo dnf install -y gcc-c++ cmake pcre2-devel re2-devel
```

### macOS

```bash
brew install cmake pcre2 re2
```

### Building from Source

#### Linux/macOS
```bash
git clone <repository-url>
cd cpp_ripgrep
mkdir build && cd build
cmake ..
make
sudo make install  # Optional: install system-wide
```

#### Windows
```bash
# Option 1: Cross-compile from Linux (recommended)
./scripts/build-windows.sh

# Option 2: Build natively on Windows
# Install Visual Studio or MinGW-w64, then:
mkdir build && cd build
cmake ..
make
```

## Usage

### Basic Usage

```bash
# Search for a pattern in current directory
./cpp_ripgrep "pattern"

# Search in specific files
./cpp_ripgrep "pattern" file1.txt file2.txt

# Search in directories
./cpp_ripgrep "pattern" src/ include/
```

### Command Line Options

```
Usage: cpp_ripgrep [OPTIONS] PATTERN [PATH...]

Search for PATTERN in files at PATH (default: current directory)

Options:
  -i, --ignore-case       Case insensitive search
  -n, --line-number       Show line numbers
  -c, --count             Only show count of matches
  -v, --invert-match      Invert match
  -w, --word-regexp       Match whole words only
  -x, --line-regexp       Match whole lines only
  -r, --recursive         Search directories recursively (default)
  --no-recursive          Don't search directories recursively
  --max-depth DEPTH       Maximum directory depth
  -j, --threads NUM       Number of threads (default: auto)
  --exclude PATTERN       Exclude files matching pattern
  --include PATTERN       Only search files matching pattern
  -q, --quiet             Suppress normal output
  --color WHEN            When to use colors (never, auto, always)
  --no-color              Disable colors
  --regex-engine ENGINE   Use specific regex engine (pcre2, re2)
  -h, --help              Show this help message
  -V, --version           Show version information
```

### Examples

```bash
# Case insensitive search
./cpp_ripgrep -i "hello" src/

# Search with line numbers
./cpp_ripgrep -n "error" *.log

# Count matches only
./cpp_ripgrep -c "TODO" src/

# Use regex to find all words
./cpp_ripgrep "\b\w+\b" document.txt

# Use specific regex engine
./cpp_ripgrep --regex-engine re2 "\\w+" document.txt

# Exclude certain file types
./cpp_ripgrep "pattern" --exclude "*.o" --exclude "*.a"

# Windows usage (if cross-compiled)
cpp_ripgrep.exe "pattern" file.txt
cpp_ripgrep.exe -i "hello" *.txt

# Use specific number of threads
./cpp_ripgrep -j 8 "pattern" large_directory/

# Invert match (find lines that don't match)
./cpp_ripgrep -v "debug" source.cpp
```

## Performance Comparison

This tool is designed to provide performance similar to ripgrep:

- **Memory-mapped I/O** for efficient file reading (Windows and Unix)
- **Multi-threaded processing** for parallel file handling
- **PCRE2 regex engine** with JIT compilation
- **RE2 regex engine** for guaranteed linear-time matching
- **Optimized string matching** for literal patterns
- **Smart file filtering** to avoid unnecessary processing
- **Cross-platform support** with native performance on Windows and Unix

## Architecture

### Core Components

1. **Options Parser**: Handles command-line argument parsing
2. **Regex Matcher**: PCRE2-based pattern matching with literal fallback
3. **RE2 Matcher**: RE2-based pattern matching for guaranteed linear-time performance
4. **File Scanner**: Efficient file I/O with memory mapping
5. **Grep Engine**: Orchestrates the search process with parallel processing

### Threading Model

- **Worker Threads**: Process files in parallel from a shared queue
- **Thread Pool**: Configurable number of worker threads
- **Synchronization**: Mutex-protected result collection

### File Processing

- **Memory Mapping**: Uses `mmap()` (Unix) or `CreateFileMapping` (Windows) for files under 100MB
- **Fallback I/O**: Standard file I/O for larger files
- **Binary Detection**: Skips files with null bytes
- **Line Parsing**: Efficient line-by-line processing
- **Cross-Platform**: Native file I/O for each platform

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by ripgrep (https://github.com/BurntSushi/ripgrep)
- Uses PCRE2 and RE2 for regex functionality
- Built with modern C++17 features 