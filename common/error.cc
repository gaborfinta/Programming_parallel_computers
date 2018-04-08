#include "error.h"
#include <iostream>

[[noreturn]]
void error(const std::string& msg) {
    std::cerr << msg;
    if (!msg.empty() && msg.back() != '\n') {
        std::cerr << '\n';
    }
    std::cerr << std::flush;
    std::exit(EXIT_FAILURE);
}

[[noreturn]]
void error(const char* filename, const std::string& msg) {
    error(std::string(filename) + ": " + msg);
}
