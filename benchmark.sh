#!/bin/bash

# Benchmark script for cpp_ripgrep
# Compares performance with grep and ripgrep (if available)

echo "cpp_ripgrep Performance Benchmark"
echo "================================="

# Create test data
echo "Creating test data..."
mkdir -p /app/benchmark_data
for i in {1..100}; do
    echo "This is test file $i with some content to search through" > "/app/benchmark_data/file$i.txt"
    echo "Another line with different content" >> "/app/benchmark_data/file$i.txt"
    echo "And a third line for good measure" >> "/app/benchmark_data/file$i.txt"
done

echo "Created 100 test files with 3 lines each"
echo ""

# Test cpp_ripgrep
echo "Testing cpp_ripgrep..."
time /app/build/cpp_ripgrep "test" /app/benchmark_data/ > /dev/null

# Test grep if available
if command -v grep &> /dev/null; then
    echo ""
    echo "Testing grep..."
    time grep -r "test" /app/benchmark_data/ > /dev/null
fi

# Test ripgrep if available
if command -v rg &> /dev/null; then
    echo ""
    echo "Testing ripgrep..."
    time rg "test" /app/benchmark_data/ > /dev/null
fi

# Test with regex
echo ""
echo "Testing regex performance..."
echo "cpp_ripgrep with regex:"
time /app/build/cpp_ripgrep "\\b\\w+\\b" /app/benchmark_data/ > /dev/null

if command -v grep &> /dev/null; then
    echo "grep with regex:"
    time grep -r -E "\\b\\w+\\b" /app/benchmark_data/ > /dev/null
fi

if command -v rg &> /dev/null; then
    echo "ripgrep with regex:"
    time rg "\\b\\w+\\b" /app/benchmark_data/ > /dev/null
fi

# Cleanup
echo ""
echo "Cleaning up test data..."
rm -rf /app/benchmark_data

echo "Benchmark completed!" 