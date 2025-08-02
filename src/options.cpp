#include "options.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <thread>

namespace cpp_ripgrep {

Options OptionsParser::parse(int argc, char* argv[]) {
    Options options;
    
    if (argc < 2) {
        print_usage(argv[0]);
        std::exit(1);
    }
    
    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            std::exit(0);
        } else if (arg == "--version" || arg == "-V") {
            print_version();
            std::exit(0);
        } else if (arg == "--recursive" || arg == "-r") {
            options.recursive = true;
        } else if (arg == "--no-recursive") {
            options.recursive = false;
        } else if (arg == "--ignore-case" || arg == "-i") {
            options.ignore_case = true;
        } else if (arg == "--line-number" || arg == "-n") {
            options.line_number = true;
            options.show_line_number = true;
        } else if (arg == "--count" || arg == "-c") {
            options.count_only = true;
        } else if (arg == "--invert-match" || arg == "-v") {
            options.invert_match = true;
        } else if (arg == "--word-regexp" || arg == "-w") {
            options.word_match = true;
        } else if (arg == "--line-regexp" || arg == "-x") {
            options.line_match = true;
        } else if (arg == "--max-depth") {
            if (i + 1 < argc) {
                options.max_depth = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: --max-depth requires a value\n";
                std::exit(1);
            }
        } else if (arg == "--threads" || arg == "-j") {
            if (i + 1 < argc) {
                options.threads = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: --threads requires a value\n";
                std::exit(1);
            }
        } else if (arg == "--exclude") {
            if (i + 1 < argc) {
                options.exclude_patterns.push_back(argv[++i]);
            } else {
                std::cerr << "Error: --exclude requires a pattern\n";
                std::exit(1);
            }
        } else if (arg == "--include") {
            if (i + 1 < argc) {
                options.include_patterns.push_back(argv[++i]);
            } else {
                std::cerr << "Error: --include requires a pattern\n";
                std::exit(1);
            }
        } else if (arg == "--quiet" || arg == "-q") {
            options.quiet = true;
        } else if (arg == "--no-filename" || arg == "-h") {
            options.show_filename = false;
        } else if (arg == "--no-line-number") {
            options.show_line_number = false;
        } else if (arg == "--color") {
            if (i + 1 < argc) {
                options.color = argv[++i];
            } else {
                options.color = "auto";
            }
        } else if (arg == "--regex-engine") {
            if (i + 1 < argc) {
                std::string engine = argv[++i];
                if (engine == "pcre2") {
                    options.regex_engine = RegexEngine::PCRE2;
                } else if (engine == "re2") {
                    options.regex_engine = RegexEngine::RE2;
                } else {
                    std::cerr << "Error: Invalid regex engine. Use 'pcre2' or 're2'\n";
                    std::exit(1);
                }
            } else {
                std::cerr << "Error: --regex-engine requires a value\n";
                std::exit(1);
            }
        } else if (arg == "--no-color") {
            options.color = "never";
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            std::exit(1);
        } else {
            // This is either the pattern or a path
            if (options.pattern.empty()) {
                options.pattern = arg;
            } else {
                options.paths.push_back(arg);
            }
        }
    }
    
    // Set default paths if none provided
    if (options.paths.empty()) {
        options.paths.push_back(".");
    }
    
    // Auto-detect thread count
    if (options.threads == 0) {
        options.threads = std::thread::hardware_concurrency();
        if (options.threads == 0) options.threads = 4; // fallback
    }
    
    // Determine search mode
    if (options.ignore_case) {
        options.mode = SearchMode::CASE_INSENSITIVE;
    } else if (options.pattern.find_first_of(".*+?^$()[]{}|\\") != std::string::npos) {
        options.mode = SearchMode::REGEX;
    } else {
        options.mode = SearchMode::LITERAL;
    }
    
    validate_options(options);
    return options;
}

void OptionsParser::validate_options(const Options& options) {
    if (options.pattern.empty()) {
        std::cerr << "Error: No search pattern provided\n";
        std::exit(1);
    }
    
    if (options.threads < 1) {
        std::cerr << "Error: Thread count must be at least 1\n";
        std::exit(1);
    }
    
    if (options.max_depth < -1) {
        std::cerr << "Error: Max depth must be -1 or greater\n";
        std::exit(1);
    }
}

void OptionsParser::print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] PATTERN [PATH...]\n"
              << "\n"
              << "Search for PATTERN in files at PATH (default: current directory)\n"
              << "\n"
              << "Options:\n"
              << "  -i, --ignore-case       Case insensitive search\n"
              << "  -n, --line-number       Show line numbers\n"
              << "  -c, --count             Only show count of matches\n"
              << "  -v, --invert-match      Invert match\n"
              << "  -w, --word-regexp       Match whole words only\n"
              << "  -x, --line-regexp       Match whole lines only\n"
              << "  -r, --recursive         Search directories recursively (default)\n"
              << "  --no-recursive          Don't search directories recursively\n"
              << "  --max-depth DEPTH       Maximum directory depth\n"
              << "  -j, --threads NUM       Number of threads (default: auto)\n"
              << "  --exclude PATTERN       Exclude files matching pattern\n"
              << "  --include PATTERN       Only search files matching pattern\n"
              << "  -q, --quiet             Suppress normal output\n"
              << "  --color WHEN            When to use colors (never, auto, always)\n"
              << "  --no-color              Disable colors\n"
              << "  --regex-engine ENGINE   Use specific regex engine (pcre2, re2)\n"
              << "  -h, --help              Show this help message\n"
              << "  -V, --version           Show version information\n"
              << "\n"
              << "Examples:\n"
              << "  " << program_name << " hello                    # Search for 'hello' in current directory\n"
              << "  " << program_name << " -i hello src/            # Case insensitive search in src/\n"
              << "  " << program_name << " -r \"\\b\\w+\\b\" .         # Find all words using regex\n"
              << "  " << program_name << " -c error *.log           # Count error lines in log files\n";
}

void OptionsParser::print_version() {
    std::cout << "cpp_ripgrep version 1.0.0\n"
              << "A fast grep-like tool written in C++\n";
}

} // namespace cpp_ripgrep 