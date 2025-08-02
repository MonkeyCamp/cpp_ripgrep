#include "options.hpp"
#include "grep_engine.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // Parse command line options
        auto options = cpp_ripgrep::OptionsParser::parse(argc, argv);
        
        // Create and run grep engine
        cpp_ripgrep::GrepEngine engine(options);
        return engine.search();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
} 