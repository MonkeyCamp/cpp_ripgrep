#pragma once

#include "options.hpp"
#include "regex_matcher.hpp"
#include "re2_matcher.hpp"
#include "file_scanner.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace cpp_ripgrep {

struct SearchResult {
    std::string file_path;
    size_t line_number;
    std::string line_content;
    std::vector<Match> matches;
    bool matched;
};

class GrepEngine {
public:
    explicit GrepEngine(const Options& options);
    
    // Main search function
    int search();
    
    // Get results (for testing or programmatic use)
    const std::vector<SearchResult>& get_results() const { return results_; }
    
    // Get match count
    size_t get_match_count() const { return match_count_; }

private:
    Options options_;
    std::unique_ptr<RegexMatcher> pcre2_matcher_;
    std::unique_ptr<RE2Matcher> re2_matcher_;
    FileScanner scanner_;
    
    std::vector<SearchResult> results_;
    std::atomic<size_t> match_count_{0};
    
    // Threading support
    std::vector<std::thread> workers_;
    std::queue<FileInfo> file_queue_;
    std::mutex queue_mutex_;
    std::mutex results_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> done_{false};
    
    // Worker thread function
    void worker_thread();
    
    // Process a single file
    void process_file(const FileInfo& file_info);
    
    // Search in file content
    std::vector<SearchResult> search_in_content(const std::string& file_path, 
                                               const std::string& content);
    
    // Add result thread-safely
    void add_result(const SearchResult& result);
    
    // Print result
    void print_result(const SearchResult& result) const;
    
    // Format output
    std::string format_output(const SearchResult& result) const;
    
    // Color support
    std::string colorize(const std::string& text, const std::string& color) const;
};

} // namespace cpp_ripgrep 