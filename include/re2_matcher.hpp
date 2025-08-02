#pragma once

#include "common.hpp"
#include <vector>
#include <memory>
#include <optional>

#ifdef HAVE_RE2
#include <re2/re2.h>
#endif

namespace cpp_ripgrep {

class RE2Matcher {
public:
    explicit RE2Matcher(const std::string& pattern, bool case_insensitive = false);
    ~RE2Matcher();

    // Disable copy
    RE2Matcher(const RE2Matcher&) = delete;
    RE2Matcher& operator=(const RE2Matcher&) = delete;

    // Allow move
    RE2Matcher(RE2Matcher&& other) noexcept;
    RE2Matcher& operator=(RE2Matcher&& other) noexcept;

    // Check if pattern is valid
    bool is_valid() const;
    std::string get_error() const { return error_; }

    // Find all matches in a string
    std::vector<Match> find_all(const std::string& text) const;
    
    // Check if string matches pattern
    bool matches(const std::string& text) const;
    
    // Find first match
    std::optional<Match> find_first(const std::string& text) const;

private:
#ifdef HAVE_RE2
    std::unique_ptr<re2::RE2> regex_;
#endif
    std::string error_;
    std::string pattern_;
    bool case_insensitive_;
    
    void cleanup();
    void move_from(RE2Matcher&& other);
};

} // namespace cpp_ripgrep 