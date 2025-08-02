#!/bin/bash

# Cross-compilation build script for Windows
# This script builds cpp_ripgrep for Windows from Linux

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Building cpp_ripgrep for Windows..."

# Check if we're on a supported platform
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "Error: This script is designed to run on Linux"
    exit 1
fi

# Check for required tools
check_tool() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed"
        echo "Please install it first:"
        case "$1" in
            "x86_64-w64-mingw32-gcc")
                echo "  sudo apt install gcc-mingw-w64-x86-64"
                ;;
            "i686-w64-mingw32-gcc")
                echo "  sudo apt install gcc-mingw-w64-i686"
                ;;
            *)
                echo "  sudo apt install $1"
                ;;
        esac
        exit 1
    fi
}

# Check for 64-bit toolchain
check_tool "x86_64-w64-mingw32-gcc"
check_tool "x86_64-w64-mingw32-g++"

# Check for 32-bit toolchain (optional)
if command -v "i686-w64-mingw32-gcc" &> /dev/null; then
    BUILD_32BIT=true
    echo "32-bit toolchain found, will build both 32-bit and 64-bit versions"
else
    BUILD_32BIT=false
    echo "32-bit toolchain not found, building only 64-bit version"
fi

# Create build directories
BUILD_DIR_64="$PROJECT_ROOT/build-windows64"
BUILD_DIR_32="$PROJECT_ROOT/build-windows32"

mkdir -p "$BUILD_DIR_64"
if [ "$BUILD_32BIT" = true ]; then
    mkdir -p "$BUILD_DIR_32"
fi

# Build 64-bit version
echo "Building 64-bit Windows version..."
cd "$BUILD_DIR_64"
cmake -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/windows-toolchain.cmake" "$PROJECT_ROOT"
make -j$(nproc)

# Build 32-bit version if toolchain is available
if [ "$BUILD_32BIT" = true ]; then
    echo "Building 32-bit Windows version..."
    cd "$BUILD_DIR_32"
    cmake -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/windows32-toolchain.cmake" "$PROJECT_ROOT"
    make -j$(nproc)
fi

# Create output directory
OUTPUT_DIR="$PROJECT_ROOT/dist"
mkdir -p "$OUTPUT_DIR"

# Copy executables
echo "Copying executables..."
cp "$BUILD_DIR_64/cpp_ripgrep.exe" "$OUTPUT_DIR/cpp_ripgrep-x64.exe"
if [ "$BUILD_32BIT" = true ]; then
    cp "$BUILD_DIR_32/cpp_ripgrep.exe" "$OUTPUT_DIR/cpp_ripgrep-x86.exe"
fi

# Create a simple batch file for Windows users
cat > "$OUTPUT_DIR/cpp_ripgrep.bat" << 'EOF'
@echo off
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    cpp_ripgrep-x64.exe %*
) else (
    cpp_ripgrep-x86.exe %*
)
EOF

echo "Build completed successfully!"
echo "Windows executables are available in: $OUTPUT_DIR"
echo ""
echo "Files created:"
ls -la "$OUTPUT_DIR"
echo ""
echo "To use on Windows:"
echo "1. Copy the .exe files to your Windows machine"
echo "2. Run: cpp_ripgrep-x64.exe --help"
echo "3. Or use the batch file: cpp_ripgrep.bat --help" 