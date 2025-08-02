#pragma once

#include "common.hpp"
#include <vector>
#include <memory>
#include <optional>

#ifdef HAVE_PCRE2
// Define PCRE2 code unit width before including pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#endif

namespace cpp_ripgrep {

class RegexMatcher {
public:
    explicit RegexMatcher(const std::string& pattern, bool case_insensitive = false);
    ~RegexMatcher();

    // Disable copy
    RegexMatcher(const RegexMatcher&) = delete;
    RegexMatcher& operator=(const RegexMatcher&) = delete;

    // Allow move
    RegexMatcher(RegexMatcher&& other) noexcept;
    RegexMatcher& operator=(RegexMatcher&& other) noexcept;

    // Check if pattern is valid
    bool is_valid() const { 
#ifdef HAVE_PCRE2
        return code_ != nullptr; 
#else
        return false;
#endif
    }
    std::string get_error() const { return error_; }

    // Find all matches in a string
    std::vector<Match> find_all(const std::string& text) const;
    
    // Check if string matches pattern
    bool matches(const std::string& text) const;
    
    // Find first match
    std::optional<Match> find_first(const std::string& text) const;

    // Literal string matching (for performance when regex not needed)
    static bool literal_match(const std::string& text, const std::string& pattern, bool case_insensitive = false);

private:
#ifdef HAVE_PCRE2
    pcre2_code* code_;
    pcre2_match_data* match_data_;
#endif
    std::string error_;
    
    void cleanup();
    void move_from(RegexMatcher&& other);
};

} // namespace cpp_ripgrep 