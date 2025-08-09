#include "re2_matcher.hpp"
#include <algorithm>
#include <iostream>

namespace cpp_ripgrep {

RE2Matcher::RE2Matcher(const std::string& pattern, bool case_insensitive)
    : pattern_(pattern), case_insensitive_(case_insensitive) {
    
#ifdef HAVE_RE2
    re2::RE2::Options options;
    options.set_case_sensitive(!case_insensitive);
    
    regex_ = std::make_unique<re2::RE2>(pattern, options);
    
    if (!regex_->ok()) {
        error_ = regex_->error();
    }
#else
    error_ = "RE2 support not compiled in";
#endif
}

RE2Matcher::~RE2Matcher() {
    cleanup();
}

RE2Matcher::RE2Matcher(RE2Matcher&& other) noexcept
    : error_(), pattern_(), case_insensitive_(false) {
    move_from(std::move(other));
}

RE2Matcher& RE2Matcher::operator=(RE2Matcher&& other) noexcept {
    if (this != &other) {
        cleanup();
        move_from(std::move(other));
    }
    return *this;
}

void RE2Matcher::cleanup() {
#ifdef HAVE_RE2
    regex_.reset();
#endif
}

void RE2Matcher::move_from(RE2Matcher&& other) {
#ifdef HAVE_RE2
    regex_ = std::move(other.regex_);
#endif
    error_ = std::move(other.error_);
    pattern_ = std::move(other.pattern_);
    case_insensitive_ = other.case_insensitive_;
}

bool RE2Matcher::is_valid() const {
#ifdef HAVE_RE2
    return regex_ && regex_->ok();
#else
    return false;
#endif
}

std::vector<Match> RE2Matcher::find_all(const std::string& text) const {
    std::vector<Match> matches;
    
#ifdef HAVE_RE2
    if (!is_valid()) {
        return matches;
    }
    
    re2::StringPiece input(text);
    re2::StringPiece match_text;
    size_t start_pos = 0;
    
    while (re2::RE2::FindAndConsume(&input, *regex_, &match_text)) {
        Match match;
        match.start = start_pos + (text.data() + start_pos - input.data());
        match.end = match.start + match_text.size();
        match.text = std::string(match_text);
        matches.push_back(match);
        
        start_pos = match.end;
        if (start_pos >= text.length()) {
            break;
        }
    }
#endif
    
    return matches;
}

bool RE2Matcher::matches(const std::string& text) const {
#ifdef HAVE_RE2
    if (!is_valid()) {
        return false;
    }
    
    return re2::RE2::FullMatch(text, *regex_);
#else
    return false;
#endif
}

std::optional<Match> RE2Matcher::find_first(const std::string& text) const {
#ifdef HAVE_RE2
    if (!is_valid()) {
        return std::nullopt;
    }
    
    re2::StringPiece input(text);
    re2::StringPiece match_text;
    if (re2::RE2::FindAndConsume(&input, *regex_, &match_text)) {
        Match match;
        match.start = match_text.data() - text.data();
        match.end = match.start + match_text.size();
        match.text = std::string(match_text);
        return match;
    }
#endif
    
    return std::nullopt;
}

} // namespace cpp_ripgrep 