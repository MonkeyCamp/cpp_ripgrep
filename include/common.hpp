#pragma once

#include <string>

namespace cpp_ripgrep {

struct Match {
    size_t start;
    size_t end;
    std::string text;
};

} // namespace cpp_ripgrep 