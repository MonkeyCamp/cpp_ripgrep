#include "options.hpp"
#include "grep_engine.hpp"
#include <iostream>
#include <chrono> // Add this for timing

int main(int argc, char* argv[]) {
    try {
        // Parse command line options
        auto options = cpp_ripgrep::OptionsParser::parse(argc, argv);

        // Start performance timer
        auto start = std::chrono::high_resolution_clock::now();

        // Create and run grep engine
        cpp_ripgrep::GrepEngine engine(options);
        int result = engine.search();

        // End performance timer
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Search completed in " << elapsed.count() << " seconds and ";
        std::cout << "searched in " << engine.get_files_searched() << ".\n";

        return result;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
}