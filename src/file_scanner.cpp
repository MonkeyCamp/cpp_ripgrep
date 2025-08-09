#include "file_scanner.hpp"
#include "options.hpp"
#include "gitignore.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace cpp_ripgrep {

FileScanner::FileScanner(const Options& options) : options_(options) {
    // Find git root and load top-level .gitignore
    std::filesystem::path current_path = std::filesystem::current_path();
    while (current_path.has_parent_path()) {
        if (std::filesystem::exists(current_path / ".git")) {
            gitignore_stack_.emplace_back(current_path);
            break;
        }
        current_path = current_path.parent_path();
    }
}

void FileScanner::scan(const std::vector<std::string>& paths, 
                      std::function<void(const FileInfo&)> file_callback) {
    for (const auto& path : paths) {
        try {
            std::filesystem::path fs_path(path);
            
            if (!std::filesystem::exists(fs_path)) {
                std::cerr << "Warning: Path does not exist: " << path << "\n";
                continue;
            }
            
            if (std::filesystem::is_directory(fs_path)) {
                if (options_.recursive) {
                    scan_directory(path, 0, file_callback);
                } else {
                    std::cerr << "Warning: Skipping directory (use -r for recursive): " << path << "\n";
                }
            } else if (std::filesystem::is_regular_file(fs_path)) {
                FileInfo info = get_file_info(path);
                if (should_scan_file(path)) {
                    file_callback(info);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error scanning path " << path << ": " << e.what() << "\n";
        }
    }
}

void FileScanner::scan_directory(const std::string& path, int depth,
                                std::function<void(const FileInfo&)> file_callback) {
    if (options_.max_depth >= 0 && depth > options_.max_depth) {
        return;
    }

    // Check for a .gitignore file in the current directory
    std::filesystem::path gitignore_path = std::filesystem::path(path) / ".gitignore";
    bool gitignore_pushed = false;
    if (std::filesystem::exists(gitignore_path)) {
        gitignore_stack_.emplace_back(path);
        gitignore_pushed = true;
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            const std::string entry_path = entry.path().string();
            
            if (is_ignored(entry.path())) {
                continue;
            }

            // Skip hidden files and directories, unless they are explicitly not ignored
            if (entry.path().filename().string()[0] == '.' && !is_ignored(entry.path())) {
                 if (entry.is_directory()) {
                    if (is_ignored(entry.path())) continue;
                 } else {
                    if (is_ignored(entry.path())) continue;
                 }
            }
            
            if (entry.is_directory()) {
                scan_directory(entry_path, depth + 1, file_callback);
            } else if (entry.is_regular_file()) {
                if (should_scan_file(entry_path)) {
                    FileInfo info = get_file_info(entry_path);
                    file_callback(info);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory " << path << ": " << e.what() << "\n";
    }

    if (gitignore_pushed) {
        gitignore_stack_.pop_back();
    }
}

std::string FileScanner::read_file(const std::string& path) const {
#ifdef _WIN32
    // Windows implementation using CreateFile and memory mapping
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                              nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        throw std::runtime_error("Cannot get file size: " + path);
    }
    
    if (fileSize.QuadPart == 0) {
        CloseHandle(hFile);
        return "";
    }
    
    // Check if file is too large for memory mapping
    if (fileSize.QuadPart > 100 * 1024 * 1024) { // 100MB limit
        CloseHandle(hFile);
        // Fall back to regular file reading
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + path);
        }
        return std::string(std::istreambuf_iterator<char>(file), 
                          std::istreambuf_iterator<char>());
    }
    
    HANDLE hMapping = CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (hMapping == nullptr) {
        CloseHandle(hFile);
        throw std::runtime_error("Cannot create file mapping: " + path);
    }
    
    void* mapped = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    
    if (mapped == nullptr) {
        throw std::runtime_error("Cannot map view of file: " + path);
    }
    
    std::string content(static_cast<const char*>(mapped), fileSize.QuadPart);
    UnmapViewOfFile(mapped);
    
    return content;
#else
    // Unix/Linux implementation using mmap
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    
    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        throw std::runtime_error("Cannot stat file: " + path);
    }
    
    if (st.st_size == 0) {
        close(fd);
        return "";
    }
    
    // Check if file is too large for memory mapping
    if (st.st_size > 100 * 1024 * 1024) { // 100MB limit
        close(fd);
        // Fall back to regular file reading
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + path);
        }
        return std::string(std::istreambuf_iterator<char>(file), 
                          std::istreambuf_iterator<char>());
    }
    
    void* mapped = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    
    if (mapped == MAP_FAILED) {
        throw std::runtime_error("Cannot memory map file: " + path);
    }
    
    std::string content(static_cast<const char*>(mapped), st.st_size);
    munmap(mapped, st.st_size);
    
    return content;
#endif
}

std::vector<LineInfo> FileScanner::get_lines(const std::string& content) const {
    std::vector<LineInfo> lines;
    size_t pos = 0;
    size_t line_number = 1;
    
    while (pos < content.length()) {
        size_t start_pos = pos;
        size_t end_pos = content.find('\n', pos);
        
        if (end_pos == std::string::npos) {
            end_pos = content.length();
        }
        
        LineInfo line;
        line.line_number = line_number;
        line.start_pos = start_pos;
        line.end_pos = end_pos;
        line.content = content.substr(start_pos, end_pos - start_pos);
        
        // Remove carriage return if present
        if (!line.content.empty() && line.content.back() == '\r') {
            line.content.pop_back();
        }
        
        lines.push_back(line);
        
        pos = end_pos + 1;
        line_number++;
    }
    
    return lines;
}

bool FileScanner::is_ignored(const std::filesystem::path& path) const {
    for (const auto& gitignore : gitignore_stack_) {
        if (gitignore.is_ignored(path)) {
            return true;
        }
    }
    return false;
}

bool FileScanner::should_scan_file(const std::string& path) const {
    if (is_ignored(path)) {
        return false;
    }

    // Check exclude patterns
    if (!options_.exclude_patterns.empty()) {
        for (const auto& pattern : options_.exclude_patterns) {
            if (matches_pattern(path, {pattern})) {
                return false;
            }
        }
    }
    
    // Check include patterns
    if (!options_.include_patterns.empty()) {
        bool included = false;
        for (const auto& pattern : options_.include_patterns) {
            if (matches_pattern(path, {pattern})) {
                included = true;
                break;
            }
        }
        if (!included) {
            return false;
        }
    }
    
    // Skip binary files
    if (is_binary_file(path)) {
        return false;
    }
    
    return true;
}

FileInfo FileScanner::get_file_info(const std::string& path) {
    FileInfo info;
    info.path = path;
    info.name = std::filesystem::path(path).filename().string();
    
    try {
        std::filesystem::path fs_path(path);
        info.is_directory = std::filesystem::is_directory(fs_path);
        info.type = std::filesystem::status(fs_path).type();
        
        if (std::filesystem::is_regular_file(fs_path)) {
            info.size = std::filesystem::file_size(fs_path);
        } else {
            info.size = 0;
        }
    } catch (const std::exception& e) {
        info.is_directory = false;
        info.size = 0;
        info.type = std::filesystem::file_type::unknown;
    }
    
    return info;
}

bool FileScanner::matches_pattern(const std::string& path, 
                                 const std::vector<std::string>& patterns) const {
    std::string filename = std::filesystem::path(path).filename().string();
    
    for (const auto& pattern : patterns) {
        // Simple glob-like pattern matching
        if (pattern.find('*') != std::string::npos || 
            pattern.find('?') != std::string::npos) {
            // TODO: Implement proper glob matching
            // For now, just check if pattern is a substring
            if (filename.find(pattern) != std::string::npos) {
                return true;
            }
        } else {
            // Exact match
            if (filename == pattern) {
                return true;
            }
        }
    }
    
    return false;
}

bool FileScanner::is_binary_file(const std::string& path) const {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return true; // Assume binary if can't open
        }
        
        // Read first 1024 bytes to check for null bytes
        char buffer[1024];
        file.read(buffer, sizeof(buffer));
        std::streamsize bytes_read = file.gcount();
        
        for (std::streamsize i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\0') {
                return true;
            }
        }
        
        return false;
    } catch (...) {
        return true; // Assume binary on error
    }
}

bool FileScanner::is_text_file(const std::string& path) const {
    return !is_binary_file(path);
}

} // namespace cpp_ripgrep 