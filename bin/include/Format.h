#pragma once

/*
Adapted from: https://stackoverflow.com/a/44608245
*/

#include <string>
#include <vector>
#include <regex>
#include <iostream>

/**
   Helper code to unpack variadic arguments
*/
namespace format
{
	/**
		Python-like string formatting
	 */
	inline const std::string format(const std::string &fmt, std::vector<std::string> args)
	{
		std::string in(fmt), out;    // unformatted and formatted strings
		std::regex re_arg("\\{\\d+\\}");  // search for {0}, {1}, ...
		std::regex re_idx("\\d+");        // search for 0, 1, ...
		std::smatch m_arg, m_idx;               // store matches
		size_t idx = 0;                         // index of argument inside {...}

		// Replace all {x} with vbuf[x]
		while (std::regex_search(in, m_arg, re_arg)) {
			out += m_arg.prefix();
			auto text = m_arg[0].str();
			if (std::regex_search(text, m_idx, re_idx)) {
				idx = std::stoi(m_idx[0].str()) - 1;
			}
			if (idx < args.size()) {
				out += std::regex_replace(text, re_arg, args[idx]);
			}
			in = m_arg.suffix();
		}
		out += in;
		return out;
	}
}