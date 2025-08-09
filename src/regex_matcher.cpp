#include "regex_matcher.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace cpp_ripgrep {

RegexMatcher::RegexMatcher(const std::string& pattern, bool case_insensitive)
#ifdef HAVE_PCRE2
    : code_(nullptr), match_data_(nullptr) {
    
    int options = PCRE2_MULTILINE;
    if (case_insensitive) {
        options |= PCRE2_CASELESS;
    }
    
    int error_code;
    PCRE2_SIZE error_offset;
    
    code_ = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(pattern.c_str()),
        PCRE2_ZERO_TERMINATED,
        options,
        &error_code,
        &error_offset,
        nullptr
    );
    
    if (code_ == nullptr) {
        PCRE2_UCHAR error_buffer[256];
        pcre2_get_error_message(error_code, error_buffer, sizeof(error_buffer));
        error_ = std::string(reinterpret_cast<char*>(error_buffer));
        return;
    }
    
    match_data_ = pcre2_match_data_create_from_pattern(code_, nullptr);
    if (match_data_ == nullptr) {
        error_ = "Failed to create match data";
        cleanup();
    }
#else
    {
    error_ = "PCRE2 support not compiled in";
#endif
}

RegexMatcher::~RegexMatcher() {
    cleanup();
}

RegexMatcher::RegexMatcher(RegexMatcher&& other) noexcept
#ifdef HAVE_PCRE2
    : code_(nullptr), match_data_(nullptr), error_() {
#else
    : error_() {
#endif
    move_from(std::move(other));
}

RegexMatcher& RegexMatcher::operator=(RegexMatcher&& other) noexcept {
    if (this != &other) {
        cleanup();
        move_from(std::move(other));
    }
    return *this;
}

void RegexMatcher::cleanup() {
#ifdef HAVE_PCRE2
    if (match_data_) {
        pcre2_match_data_free(match_data_);
        match_data_ = nullptr;
    }
    if (code_) {
        pcre2_code_free(code_);
        code_ = nullptr;
    }
#endif
}

void RegexMatcher::move_from(RegexMatcher&& other) {
#ifdef HAVE_PCRE2
    code_ = other.code_;
    match_data_ = other.match_data_;
    other.code_ = nullptr;
    other.match_data_ = nullptr;
#endif
    error_ = std::move(other.error_);
}

std::vector<Match> RegexMatcher::find_all(const std::string& text) const {
    std::vector<Match> matches;
    
    #ifdef HAVE_PCRE2
    if (!is_valid()) {
        return matches;
    }
    PCRE2_SIZE start_offset = 0;
    const PCRE2_SPTR subject = reinterpret_cast<const PCRE2_SPTR>(text.c_str());
    const PCRE2_SIZE subject_length = text.length();
    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(code_, nullptr);
    if (!match_data) {
        return matches;
    }
    while (start_offset < subject_length) {
        int rc = pcre2_match(
            code_,
            subject,
            subject_length,
            start_offset,
            0,
            match_data,
            nullptr
        );
        if (rc < 0) {
            if (rc == PCRE2_ERROR_NOMATCH) {
                break;
            }
            // Handle other errors
            break;
        }
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
        Match match;
        match.start = ovector[0];
        match.end = ovector[1];
        if (match.start <= match.end && match.end <= text.size()) {
            match.text = text.substr(match.start, match.end - match.start);
            matches.push_back(match);
        }
        // Move to next position
        if (ovector[0] == ovector[1]) {
            start_offset = ovector[1] + 1;
        } else {
            start_offset = ovector[1];
        }
    }
    pcre2_match_data_free(match_data);
    #endif
    
    return matches;
}

bool RegexMatcher::matches(const std::string& text) const {
#ifdef HAVE_PCRE2
    if (!is_valid()) {
        return false;
    }
    
    int rc = pcre2_match(
        code_,
        reinterpret_cast<const PCRE2_SPTR>(text.c_str()),
        text.length(),
        0,
        0,
        match_data_,
        nullptr
    );
    
    return rc >= 0;
#else
    return false;
#endif
}

std::optional<Match> RegexMatcher::find_first(const std::string& text) const {
#ifdef HAVE_PCRE2
    if (!is_valid()) {
        return std::nullopt;
    }
    
    int rc = pcre2_match(
        code_,
        reinterpret_cast<const PCRE2_SPTR>(text.c_str()),
        text.length(),
        0,
        0,
        match_data_,
        nullptr
    );
    
    if (rc < 0) {
        return std::nullopt;
    }
    
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data_);
    
    Match match;
    match.start = ovector[0];
    match.end = ovector[1];
    match.text = text.substr(match.start, match.end - match.start);
    
    return match;
#else
    return std::nullopt;
#endif
}

bool RegexMatcher::literal_match(const std::string& text, const std::string& pattern, bool case_insensitive) {
    if (case_insensitive) {
        auto it = std::search(
            text.begin(), text.end(),
            pattern.begin(), pattern.end(),
            [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            }
        );
        return it != text.end();
    } else {
        return text.find(pattern) != std::string::npos;
    }
}

} // namespace cpp_ripgrep 