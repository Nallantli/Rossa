#include "util.h"

#ifndef _WIN32
#include <limits.h>
#include <unistd.h>
#else
#include <libloaderapi.h>
#endif

const std::string util::format(const std::string &fmt, const std::vector<std::string> &args)
{
    std::string in(fmt), out;        // unformatted and formatted strings
    std::regex re_arg("\\{\\d+\\}"); // search for {0}, {1}, ...
    std::regex re_idx("\\d+");       // search for 0, 1, ...
    std::smatch m_arg, m_idx;        // store matches
    size_t idx = 0;                  // index of argument inside {...}

    // Replace all {x} with vbuf[x]
    while (std::regex_search(in, m_arg, re_arg))
    {
        out += m_arg.prefix();
        auto text = m_arg[0].str();
        if (std::regex_search(text, m_idx, re_idx))
        {
            idx = std::stoi(m_idx[0].str());
        }
        if (idx < args.size())
        {
            out += std::regex_replace(text, re_arg, args[idx]);
        }
        in = m_arg.suffix();
    }
    out += in;
    return out;
}

const std::filesystem::path util::getRuntimePath()
{
#ifndef _WIN32
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#else
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
#endif
}