#ifndef ERROR_H
#define ERROR_H

#include <string>

[[noreturn]] void error(const std::string& msg);
[[noreturn]] void error(const char* filename, const std::string& msg);

#endif
