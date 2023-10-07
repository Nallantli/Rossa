#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <regex>
#include <filesystem>

namespace util
{
    const std::string format(const std::string &, const std::vector<std::string> &);
    const std::filesystem::path getRuntimePath();
}

#endif