#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace cpp_ripgrep {

class Gitignore {
public:
    explicit Gitignore(const std::filesystem::path& base_path);

    bool is_ignored(const std::filesystem::path& path) const;

private:
    void load_gitignore_file(const std::filesystem::path& gitignore_path);
    bool matches(const std::filesystem::path& path, const std::string& pattern) const;

    struct GitignorePattern {
        std::string pattern;
        bool negated;
    };

    std::filesystem::path base_path_;
    std::vector<GitignorePattern> patterns_;
};

} // namespace cpp_ripgrep
