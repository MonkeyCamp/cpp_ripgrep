#include "grep_engine.hpp"
#include "options.hpp"
#include "regex_matcher.hpp"
#include "re2_matcher.hpp"
#include "file_scanner.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace cpp_ripgrep {

GrepEngine::GrepEngine(const Options& options) 
    : options_(options), scanner_(options) {
    
    // Create appropriate matcher based on search mode and regex engine
    switch (options.mode) {
        case SearchMode::LITERAL:
            // For literal search, we'll use the static method
            break;
        case SearchMode::REGEX:
        case SearchMode::CASE_INSENSITIVE:
            if (options.regex_engine == RegexEngine::RE2) {
                re2_matcher_ = std::make_unique<RE2Matcher>(options.pattern, options.ignore_case);
                if (!re2_matcher_->is_valid()) {
                    std::cerr << "Error: Invalid RE2 regex pattern: " << re2_matcher_->get_error() << "\n";
                    std::exit(1);
                }
            } else {
                pcre2_matcher_ = std::make_unique<RegexMatcher>(options.pattern, options.ignore_case);
                if (!pcre2_matcher_->is_valid()) {
                    std::cerr << "Error: Invalid PCRE2 regex pattern: " << pcre2_matcher_->get_error() << "\n";
                    std::exit(1);
                }
            }
            break;
    }
}

void GrepEngine::start_search() {
    // Initialize worker threads
    workers_.reserve(options_.threads);
    for (int i = 0; i < options_.threads; ++i) {
        workers_.emplace_back(&GrepEngine::worker_thread, this);
    }

    // Scan files and push to the queue
    scanner_.scan(options_.paths, [this](const FileInfo& file_info) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            file_queue_.push(file_info);
        }
        queue_cv_.notify_one();
    });

    // Signal that scanning is done
    done_.store(true);
    queue_cv_.notify_all();
}

void GrepEngine::stop_search() {
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

int GrepEngine::search() {
    // Start the search
    start_search();

    // Wait for all workers to finish
    stop_search();

    // Print results
    if (!options_.quiet) {
        if (options_.count_only) {
            std::cout << match_count_.load() << "\n";
        } else {
            // Sort results for consistent output
            std::sort(results_.begin(), results_.end(),
                      [](const SearchResult& a, const SearchResult& b) {
                          if (a.file_path != b.file_path) {
                              return a.file_path < b.file_path;
                          }
                          return a.line_number < b.line_number;
                      });

            for (const auto& result : results_) {
                print_result(result);
            }
        }
    }

    return match_count_.load() > 0 ? 0 : 1;
}

void GrepEngine::worker_thread() {
    while (true) {
        FileInfo file_info;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { 
                return !file_queue_.empty() || done_.load(); 
            });
            
            if (file_queue_.empty() && done_.load()) {
                break;
            }
            
            if (!file_queue_.empty()) {
                file_info = file_queue_.front();
                file_queue_.pop();
            }
        }
        
        if (file_info.path.empty()) {
            continue;
        }
        
        process_file(file_info);
    }
}

void GrepEngine::process_file(const FileInfo& file_info) {
    try {
        std::string content = scanner_.read_file(file_info.path);
        auto file_results = search_in_content(file_info.path, content);

        for (const auto& result : file_results) {
            add_result(result);
        }
    } catch (const std::exception& e) {
        if (!options_.quiet) {
            std::cerr << "Error reading file " << file_info.path << ": " << e.what() << "\n";
        }
    }
}

std::vector<SearchResult> GrepEngine::search_in_content(const std::string& file_path, 
                                                       const std::string& content) {
    std::vector<SearchResult> results;
    auto lines = scanner_.get_lines(content);
    
    for (const auto& line : lines) {
        bool matched = false;
        std::vector<Match> matches;
        
        // Determine if line matches based on search mode
        switch (options_.mode) {
            case SearchMode::LITERAL:
                matched = RegexMatcher::literal_match(line.content, options_.pattern, options_.ignore_case);
                if (matched) {
                    // Create a simple match for literal search
                    size_t pos = options_.ignore_case ? 
                        line.content.find(options_.pattern) :
                        line.content.find(options_.pattern);
                    if (pos != std::string::npos) {
                        Match match;
                        match.start = pos;
                        match.end = pos + options_.pattern.length();
                        match.text = options_.pattern;
                        matches.push_back(match);
                    }
                }
                break;
                
            case SearchMode::REGEX:
            case SearchMode::CASE_INSENSITIVE:
                if (options_.regex_engine == RegexEngine::RE2 && re2_matcher_) {
                    matches = re2_matcher_->find_all(line.content);
                    matched = !matches.empty();
                } else if (pcre2_matcher_) {
                    matches = pcre2_matcher_->find_all(line.content);
                    matched = !matches.empty();
                }
                break;
        }
        
        // Apply word/line match constraints
        if (matched) {
            if (options_.word_match) {
                // TODO: Implement word boundary checking
                // For now, just check if it's surrounded by word boundaries
            }
            
            if (options_.line_match) {
                // Check if the entire line matches
                if (options_.regex_engine == RegexEngine::RE2 && re2_matcher_) {
                    matched = re2_matcher_->matches(line.content);
                } else if (pcre2_matcher_) {
                    matched = pcre2_matcher_->matches(line.content);
                } else {
                    matched = (line.content == options_.pattern);
                }
            }
        }
        
        // Apply invert match
        if (options_.invert_match) {
            matched = !matched;
        }
        
        if (matched) {
            SearchResult result;
            result.file_path = file_path;
            result.line_number = line.line_number;
            result.line_content = line.content;
            result.matches = matches;
            result.matched = true;
            
            results.push_back(result);
            match_count_.fetch_add(1);
        }
    }
    
    return results;
}

void GrepEngine::add_result(const SearchResult& result) {
    std::lock_guard<std::mutex> lock(results_mutex_);
    results_.push_back(result);
}

void GrepEngine::print_result(const SearchResult& result) const {
    std::cout << format_output(result) << "\n";
}

std::string GrepEngine::format_output(const SearchResult& result) const {
    std::ostringstream oss;
    
    // Add filename if requested and multiple files
    if (options_.show_filename) {
        oss << colorize(result.file_path, "blue") << ":";
    }
    
    // Add line number if requested
    if (options_.show_line_number) {
        oss << colorize(std::to_string(result.line_number), "green") << ":";
    }
    
    // Add line content
    std::string line_content = result.line_content;
    
    // Highlight matches if color is enabled
    if (options_.color && (*options_.color == "always" || 
        (*options_.color == "auto" && 
#ifdef _WIN32
        _isatty(_fileno(stdout))
#else
        isatty(STDOUT_FILENO)
#endif
        ))) {
        
        // Sort matches by start position in reverse order to avoid offset issues
        std::vector<Match> sorted_matches = result.matches;
        std::sort(sorted_matches.begin(), sorted_matches.end(),
                  [](const Match& a, const Match& b) { return a.start > b.start; });
        
        for (const auto& match : sorted_matches) {
            std::string highlighted = colorize(match.text, "red");
            line_content.replace(match.start, match.end - match.start, highlighted);
        }
    }
    
    oss << line_content;
    
    return oss.str();
}

std::string GrepEngine::colorize(const std::string& text, const std::string& color) const {
    if (!options_.color || *options_.color == "never") {
        return text;
    }
    
    if (*options_.color == "auto" && 
#ifdef _WIN32
        !_isatty(_fileno(stdout))
#else
        !isatty(STDOUT_FILENO)
#endif
        ) {
        return text;
    }
    
    std::string color_code;
    if (color == "red") color_code = "\033[31m";
    else if (color == "green") color_code = "\033[32m";
    else if (color == "blue") color_code = "\033[34m";
    else if (color == "yellow") color_code = "\033[33m";
    else if (color == "magenta") color_code = "\033[35m";
    else if (color == "cyan") color_code = "\033[36m";
    else return text;
    
    return color_code + text + "\033[0m";
}

} // namespace cpp_ripgrep