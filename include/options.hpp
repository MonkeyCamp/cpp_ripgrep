#pragma once

#include <string>
#include <vector>
#include <optional>

namespace cpp_ripgrep {

enum class SearchMode {
    LITERAL,
    REGEX,
    CASE_INSENSITIVE
};

enum class RegexEngine {
    PCRE2,
    RE2
};

struct Options {
    std::string pattern;
    std::vector<std::string> paths;
    SearchMode mode = SearchMode::LITERAL;
    RegexEngine regex_engine = RegexEngine::PCRE2;
    bool recursive = true;
    bool ignore_case = false;
    bool line_number = false;
    bool count_only = false;
    bool invert_match = false;
    bool word_match = false;
    bool line_match = false;
    int max_depth = -1;
    int threads = 0; // 0 means auto-detect
    std::vector<std::string> exclude_patterns;
    std::vector<std::string> include_patterns;
    bool quiet = false;
    bool show_filename = true;
    bool show_line_number = true;
    std::optional<std::string> color = std::nullopt;
};

class OptionsParser {
public:
    static Options parse(int argc, char* argv[]);
    static void print_usage(const char* program_name);
    static void print_version();

private:
    static void validate_options(const Options& options);
};

} // namespace cpp_ripgrep 