#include "gitignore.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <regex>

namespace cpp_ripgrep {

Gitignore::Gitignore(const std::filesystem::path& base_path) : base_path_(base_path) {
    std::filesystem::path gitignore_path = base_path / ".gitignore";
    if (std::filesystem::exists(gitignore_path)) {
        load_gitignore_file(gitignore_path);
    }
}

void Gitignore::load_gitignore_file(const std::filesystem::path& gitignore_path) {
    std::ifstream file(gitignore_path);
    if (!file) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        bool negated = false;
        if (line[0] == '!') {
            negated = true;
            line.erase(0, 1);
        }

        patterns_.push_back({line, negated});
    }
}

bool Gitignore::is_ignored(const std::filesystem::path& path) const {
    std::filesystem::path relative_path;
    try {
        relative_path = std::filesystem::relative(path, base_path_);
    } catch (const std::filesystem::filesystem_error& e) {
        return false;
    }

    bool ignored = false;
    for (const auto& p : patterns_) {
        if (matches(relative_path, p.pattern)) {
            ignored = !p.negated;
        }
    }

    return ignored;
}

// Convert gitignore pattern to regex
// This is a simplified conversion and does not handle all cases.
std::string pattern_to_regex(const std::string& pattern) {
    std::string regex_str;
    regex_str.reserve(pattern.length() * 2);

    for (size_t i = 0; i < pattern.length(); ++i) {
        char c = pattern[i];
        switch (c) {
            case '*':
                if (i + 1 < pattern.length() && pattern[i+1] == '*') {
                    regex_str += ".*";
                    i++;
                } else {
                    regex_str += "[^/]*";
                }
                break;
            case '?':
                regex_str += "[^/]";
                break;
            case '.':
                regex_str += "\\.";
                break;
            default:
                regex_str += c;
                break;
        }
    }
    return regex_str;
}

bool Gitignore::matches(const std::filesystem::path& path, const std::string& pattern) const {
    std::string path_str = path.string();
    std::replace(path_str.begin(), path_str.end(), '\\', '/');

    std::string regex_pattern = pattern_to_regex(pattern);

    // If pattern ends with '/', it should match a directory
    if (pattern.back() == '/') {
        if (!std::filesystem::is_directory(base_path_ / path)) {
            return false;
        }
        regex_pattern.pop_back(); // remove trailing slash for regex
    }

    // If pattern contains '/', it is matched against the full path
    if (pattern.find('/') != std::string::npos) {
        try {
            std::regex re(regex_pattern);
            return std::regex_search(path_str, re);
        } catch (const std::regex_error& e) {
            return false;
        }
    }

    // Otherwise, it is matched against the filename only
    try {
        std::regex re(regex_pattern);
        return std::regex_search(path.filename().string(), re);
    } catch (const std::regex_error& e) {
        return false;
    }
}

} // namespace cpp_ripgrep
