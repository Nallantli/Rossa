#include "global.h"

#include "../rossa_error/rossa_error.h"
#include "../parser/parser.h"
#include "../node_parser/node_parser.h"
#include "../symbol/symbol.h"
#include "../function/function.h"

#include "../util/util.h"

std::vector<std::filesystem::path> dir::loaded = {};
std::map<std::string, std::map<std::string, extf_t>> global::loaded = {};

const std::filesystem::path dir::findFile(const std::filesystem::path &currentDir, const std::string &filename, const token_t *token)
{
	auto currentDirCheck = currentDir / filename;
	if (std::filesystem::exists(currentDirCheck))
		return currentDirCheck;
	auto libDirCheck = util::getRuntimePath().parent_path() / "lib" / filename;
	if (std::filesystem::exists(libDirCheck))
		return libDirCheck;
	trace_t stack_trace;
	throw rossa_error_t(util::format(_FILE_NOT_FOUND_, {filename}), *token, stack_trace);
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
			throw rossa_error_t(util::format(_EXTERNAL_LIBRARY_NOT_EXIST_, {libname}), *token, stack_trace);
		}
		auto f = GetProcAddress(library, (rawlibname + "_rossaExportFunctions").c_str());
#endif
		if (f == NULL)
		{
			trace_t stack_trace;
			throw rossa_error_t(util::format(_EXPORT_FUNCTION_NOT_FOUND_, {libname}), *token, stack_trace);
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
		throw rossa_error_t(util::format(_LIBRARY_NOT_IN_MEMORY_, {rawlibname}), *token, stack_trace);
	}
	if (loaded[rawlibname].find(fname) == loaded[rawlibname].end())
	{
		trace_t stack_trace;
		throw rossa_error_t(util::format(_LIBRARY_FUNCTION_NOT_EXIST_, {rawlibname, fname}), *token, stack_trace);
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

const mediator_t global::convertToMediator(const symbol_t &s, const token_t *token, trace_t &stack_trace)
{
	switch (s.getValueType())
	{
	case NIL:
		return mediator_t();
	case BOOLEAN_D:
		return mediator_t(
			MEDIATOR_BOOLEAN_D,
			std::make_shared<bool>(s.getBool(token, stack_trace)));
	case NUMBER:
		return mediator_t(
			MEDIATOR_NUMBER,
			std::make_shared<number_t>(s.getNumber(token, stack_trace)));
	case STRING:
		return mediator_t(
			MEDIATOR_STRING,
			std::make_shared<std::string>(s.getString(token, stack_trace)));
	case POINTER:
		return mediator_t(
			MEDIATOR_POINTER,
			s.getPointer(token, stack_trace));
	case FUNCTION:
		return MAKE_POINTER(s.getFunction({}, token, stack_trace));
	case ARRAY:
	{
		std::vector<mediator_t> mv;
		for (auto &e : s.getVector(token, stack_trace))
		{
			mv.push_back(convertToMediator(e, token, stack_trace));
		}
		return mediator_t(
			MEDIATOR_ARRAY,
			std::make_shared<std::vector<mediator_t>>(mv));
	}
	case DICTIONARY:
	{
		std::map<const std::string, const mediator_t> md;
		for (auto &e : s.getDictionary(token, stack_trace))
		{
			md.insert({e.first, convertToMediator(e.second, token, stack_trace)});
		}
		return MAKE_DICTIONARY(md);
	}
	default:
		throw rossa_error_t("[this shouldn't happen] Improper symbol_t to mediator_t conversion: " + s.toCodeString(), *token, stack_trace);
	}
}

const symbol_t global::convertToSymbol(const mediator_t &m)
{
	switch (m.getType())
	{
	case MEDIATOR_NIL:
		return symbol_t();
	case MEDIATOR_BOOLEAN_D:
		return symbol_t::Boolean(COERCE_BOOLEAN(m));
	case MEDIATOR_NUMBER:
		return symbol_t::Number(COERCE_NUMBER(m));
	case MEDIATOR_STRING:
		return symbol_t::String(COERCE_STRING(m));
	case MEDIATOR_POINTER:
		return symbol_t::Pointer(COERCE_POINTER(m, void));
	case MEDIATOR_ARRAY:
	{
		std::vector<symbol_t> sv;
		for (auto &e : COERCE_ARRAY(m))
		{
			sv.push_back(convertToSymbol(e));
		}
		return symbol_t::Array(sv);
	}
	case DICTIONARY:
	{
		std::map<const std::string, const symbol_t> sd;
		for (auto &e : COERCE_DICTIONARY(m))
		{
			sd.insert({e.first, convertToSymbol(e.second)});
		}
		return symbol_t::Dictionary(sd);
	}
	default:
		return symbol_t();
	}
}