#include "global.h"

#include "../rossa_error/rossa_error.h"
#include "../parser/parser.h"
#include "../node_parser/node_parser.h"
#include "../symbol/symbol.h"
#include "../function/function.h"

#include <regex>

std::vector<std::filesystem::path> dir::loaded = {};
std::map<std::string, std::map<std::string, extf_t>> global::loaded = {};

const std::filesystem::path dir::getRuntimePath()
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

const std::filesystem::path dir::findFile(const std::filesystem::path &currentDir, const std::string &filename, const token_t *token)
{
	auto currentDirCheck = currentDir / filename;
	if (std::filesystem::exists(currentDirCheck))
		return currentDirCheck;
	auto libDirCheck = getRuntimePath().parent_path() / "lib" / filename;
	if (std::filesystem::exists(libDirCheck))
		return libDirCheck;
	trace_t stack_trace;
	throw rossa_error_t(global::format(_FILE_NOT_FOUND_, {filename}), *token, stack_trace);
}

void global::loadLibrary(const std::filesystem::path &currentDir, const std::string &rawlibname, const token_t *token)
{
	if (loaded.find(rawlibname) == loaded.end())
	{
#ifndef _WIN32
		std::string libname = rawlibname + ".so";
		auto library = dlopen(dir::findFile(currentDir, libname, token).string().c_str(), RTLD_LAZY);
		if (library == NULL)
		{
			trace_t stack_trace;
			throw rossa_error_t(format(_EXTERNAL_LIBRARY_NOT_EXIST_, {libname}), *token, stack_trace);
		}
		auto f = dlsym(library, (rawlibname + "_rossaExportFunctions").c_str());
#else
		std::string libname = rawlibname + ".dll";
		auto path = dir::findFile(currentDir, libname, token);
		auto library = LoadLibraryA(path.string().c_str());
		if (library == NULL)
		{
			trace_t stack_trace;
			throw rossa_error_t(format(_EXTERNAL_LIBRARY_NOT_EXIST_, {libname}), *token, stack_trace);
		}
		auto f = GetProcAddress(library, (rawlibname + "_rossaExportFunctions").c_str());
#endif
		if (f == NULL)
		{
			trace_t stack_trace;
			throw rossa_error_t(format(_EXPORT_FUNCTION_NOT_FOUND_, {libname}), *token, stack_trace);
		}
		std::map<std::string, extf_t> fns;
		auto ef = (export_fns_t)f;
		ef(fns);
		loaded[rawlibname] = fns;
	}
}

extf_t global::loadFunction(const std::string &rawlibname, const std::string &fname, const token_t *token)
{
	if (loaded.find(rawlibname) == loaded.end())
	{
		trace_t stack_trace;
		throw rossa_error_t(format(_LIBRARY_NOT_IN_MEMORY_, {rawlibname}), *token, stack_trace);
	}
	if (loaded[rawlibname].find(fname) == loaded[rawlibname].end())
	{
		trace_t stack_trace;
		throw rossa_error_t(format(_LIBRARY_FUNCTION_NOT_EXIST_, {rawlibname, fname}), *token, stack_trace);
	}
	return loaded[rawlibname][fname];
}

const std::string global::getTypeString(const aug_type_t &t)
{
	std::string ret = "";
	int j = 0;
	for (auto &i : t)
	{
		if (j++ > 0)
			ret += ".";
		if (i >= 0)
			ret += ROSSA_DEHASH(i);
		else
		{
			switch (i)
			{
			case value_type_enum::NIL:
				ret += KEYWORD_NIL_NAME;
				break;
			case value_type_enum::ANY:
				ret += KEYWORD_ANY;
				break;
			case value_type_enum::NUMBER:
				ret += KEYWORD_NUMBER;
				break;
			case value_type_enum::STRING:
				ret += KEYWORD_STRING;
				break;
			case value_type_enum::BOOLEAN_D:
				ret += KEYWORD_BOOLEAN;
				break;
			case value_type_enum::ARRAY:
				ret += KEYWORD_ARRAY;
				break;
			case value_type_enum::FUNCTION:
				ret += KEYWORD_FUNCTION;
				break;
			case value_type_enum::DICTIONARY:
				ret += KEYWORD_DICTIONARY;
				break;
			case value_type_enum::OBJECT:
				ret += KEYWORD_OBJECT;
				break;
			case value_type_enum::POINTER:
				ret += KEYWORD_POINTER;
				break;
			case value_type_enum::TYPE_NAME:
				ret += KEYWORD_TYPE;
				break;
			default:
				return "<error-type>";
			}
		}
	}
	return ret;
}

const std::string global::deHashVec(const std::vector<node_scope_t> &t)
{
	std::string ret = "";
	int j = 0;
	for (auto &i : t)
	{
		if (j++ > 0)
			ret += ".";
		ret += ROSSA_DEHASH(i.id) + "[";
		int k = 0;
		for (auto &v : i.var_ids)
		{
			if (k++ > 0)
				ret += ", ";
			ret += ROSSA_DEHASH(v);
		}
		ret += "]";
	}
	return ret;
}

const std::string global::format(const std::string &fmt, std::vector<std::string> args)
{
	std::string in(fmt), out;		 // unformatted and formatted strings
	std::regex re_arg("\\{\\d+\\}"); // search for {0}, {1}, ...
	std::regex re_idx("\\d+");		 // search for 0, 1, ...
	std::smatch m_arg, m_idx;		 // store matches
	size_t idx = 0;					 // index of argument inside {...}

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