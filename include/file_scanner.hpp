#pragma once

#include <string>
#include <vector>
#include <functional>
#include "gitignore.hpp"
#include <memory>
#include <filesystem>
#include <vector>

// Forward declaration
namespace cpp_ripgrep {
    struct Options;
}

namespace cpp_ripgrep {

struct FileInfo {
    std::string path;
    std::string name;
    bool is_directory;
    size_t size;
    std::filesystem::file_type type;
};

struct LineInfo {
    size_t line_number;
    size_t start_pos;
    size_t end_pos;
    std::string content;
};

class FileScanner {
public:
    explicit FileScanner(const Options& options);
    
    // Scan files and directories
    void scan(const std::vector<std::string>& paths, 
              std::function<void(const FileInfo&)> file_callback);
    
    // Read file content with memory mapping
    std::string read_file(const std::string& path) const;
    
    // Get lines from file content
    std::vector<LineInfo> get_lines(const std::string& content) const;
    
    // Check if file should be included/excluded
    bool should_scan_file(const std::string& path) const;
    
    // Get file info
    static FileInfo get_file_info(const std::string& path);

private:
    const Options& options_;
    std::vector<Gitignore> gitignore_stack_;

    void scan_directory(const std::string& path, int depth,
                       std::function<void(const FileInfo&)> file_callback);
    
    bool is_ignored(const std::filesystem::path& path) const;

    bool matches_pattern(const std::string& path, 
                        const std::vector<std::string>& patterns) const;
    
    bool is_binary_file(const std::string& path) const;
    bool is_text_file(const std::string& path) const;
};

} // namespace cpp_ripgrep 