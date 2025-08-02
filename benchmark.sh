#!/bin/bash

# Benchmark script for cpp_ripgrep
# Compares performance with grep and ripgrep (if available)

echo "cpp_ripgrep Performance Benchmark"
echo "================================="

# Create test data
echo "Creating test data..."
mkdir -p benchmark_data
for i in {1..100}; do
    echo "This is test file $i with some content to search through" > "benchmark_data/file$i.txt"
    echo "Another line with different content" >> "benchmark_data/file$i.txt"
    echo "And a third line for good measure" >> "benchmark_data/file$i.txt"
done

echo "Created 100 test files with 3 lines each"
echo ""

# Test cpp_ripgrep
echo "Testing cpp_ripgrep..."
time ./build/cpp_ripgrep "test" benchmark_data/ > /dev/null

# Test grep if available
if command -v grep &> /dev/null; then
    echo ""
    echo "Testing grep..."
    time grep -r "test" benchmark_data/ > /dev/null
fi

# Test ripgrep if available
if command -v rg &> /dev/null; then
    echo ""
    echo "Testing ripgrep..."
    time rg "test" benchmark_data/ > /dev/null
fi

# Test with regex
echo ""
echo "Testing regex performance..."
echo "cpp_ripgrep with regex:"
time ./build/cpp_ripgrep "\\b\\w+\\b" benchmark_data/ > /dev/null

if command -v grep &> /dev/null; then
    echo "grep with regex:"
    time grep -r -E "\\b\\w+\\b" benchmark_data/ > /dev/null
fi

if command -v rg &> /dev/null; then
    echo "ripgrep with regex:"
    time rg "\\b\\w+\\b" benchmark_data/ > /dev/null
fi

# Cleanup
echo ""
echo "Cleaning up test data..."
rm -rf benchmark_data

echo "Benchmark completed!" 