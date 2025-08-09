#include "file_scanner.hpp"
#include "options.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <vector>
#include <string>
#include <set>

void create_test_directory() {
    std::filesystem::create_directory("test_dir");
    std::filesystem::create_directory("test_dir/subdir");

    std::ofstream gitignore("test_dir/.gitignore");
    gitignore << "*.log\n";
    gitignore << "build/\n";
    gitignore << "!important.log\n";
    gitignore.close();

    std::ofstream("test_dir/file1.txt").close();
    std::ofstream("test_dir/file2.log").close();
    std::ofstream("test_dir/important.log").close();
    std::filesystem::create_directory("test_dir/build");
    std::ofstream("test_dir/build/some_file.txt").close();
    std::ofstream("test_dir/subdir/file3.txt").close();
}

void cleanup_test_directory() {
    std::filesystem::remove_all("test_dir");
}

int main() {
    create_test_directory();

    cpp_ripgrep::Options options;
    options.recursive = true;

    cpp_ripgrep::FileScanner scanner(options);

    std::set<std::string> scanned_files;
    scanner.scan({"test_dir"}, [&](const cpp_ripgrep::FileInfo& file_info) {
        scanned_files.insert(file_info.path);
    });

    std::set<std::string> expected_files;
    expected_files.insert("test_dir/file1.txt");
    expected_files.insert("test_dir/important.log");
    expected_files.insert("test_dir/subdir/file3.txt");

    std::cout << "Scanned files:" << std::endl;
    for (const auto& file : scanned_files) {
        std::cout << file << std::endl;
    }

    assert(scanned_files.size() == 3);
    assert(scanned_files == expected_files);

    std::cout << "All tests passed!" << std::endl;

    cleanup_test_directory();

    return 0;
}
