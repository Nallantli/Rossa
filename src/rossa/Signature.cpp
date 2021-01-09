#include "../../bin/include/Rossa.h"

using namespace rossa;

const size_t sig::validity(const sig_t &values, const std::vector<Symbol> &check, std::vector<Function> &stack_trace)
{
	if (values.size() == 0)
		return 1;

	size_t v = 0;
	for (size_t i = 0; i < values.size(); i++) {
		type_sll vt = check[i].getAugValueType();
		if (values[i] == vt)
			v += 3;
		else if (values[i] == NIL)
			v += 1;
		else if (check[i].getValueType() == OBJECT) {
			if (values[i] == OBJECT || check[i].getObject(NULL, stack_trace)->extendsObject(values[i]))
				v += 2;
			else
				return 0;
		} else
			return 0;
	}
	return v;
}

const std::string sig::toCodeString(const sig_t &values)
{
	std::string s = "{";
	size_t i = 0;
	for (auto &v : values) {
		if (i++ > 0)
			s += ", ";
		s += "static_cast<type_sll>(" + std::to_string(v) + ")";
	}
	return s + "}";
}

const std::string sig::toString(const sig_t &values)
{
	std::string s = "(";
	size_t i = 0;
	for (auto &v : values) {
		if (i++ > 0)
			s += ", ";
		s += getTypeString(v);
	}
	return s + ")";
}


const std::string sig::getTypeString(const type_sll &i)
{
	if (i >= 0)
		return "@" + ROSSA_DEHASH(i);
	else {
		switch (i) {
			case NIL:
				return KEYWORD_NIL_NAME;
			case NUMBER:
				return KEYWORD_NUMBER;
			case STRING:
				return KEYWORD_STRING;
			case BOOLEAN_D:
				return KEYWORD_BOOLEAN;
			case ARRAY:
				return KEYWORD_ARRAY;
			case FUNCTION:
				return KEYWORD_FUNCTION;
			case DICTIONARY:
				return KEYWORD_DICTIONARY;
			case OBJECT:
				return KEYWORD_OBJECT;
			case POINTER:
				return KEYWORD_POINTER;
			case TYPE_NAME:
				return KEYWORD_TYPE;
			default:
				return "<error-type>";
		}
	}
}

std::vector<std::filesystem::path> dir::loaded = {};
std::map<std::string, std::map<std::string, extf_t>> lib::loaded = {};
std::vector<std::filesystem::path> lib::libPaths = {};

const std::filesystem::path dir::getRuntimePath()
{
#ifndef _WIN32
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
#else
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, path, MAX_PATH);
	return path;
#endif
}

const std::filesystem::path dir::findFile(const std::filesystem::path &currentDir, const std::string &filename, const Token *token)
{
	auto currentDirCheck = currentDir / filename;
	if (std::filesystem::exists(currentDirCheck))
		return currentDirCheck;
	auto libDirCheck = getRuntimePath().parent_path() / "lib" / filename;
	if (std::filesystem::exists(libDirCheck))
		return libDirCheck;
	std::vector<Function> stack_trace;
	throw RTError(format::format(_FILE_NOT_FOUND_, { filename }), *token, stack_trace);
}

const std::vector<std::string> dir::compiledOptions(int argc, char const *argv[])
{
	std::vector<std::string> passed;

	for (int i = 1; i < argc; i++) {
		passed.push_back(argv[i]);
	}

	return passed;
}

void lib::loadLibrary(const std::filesystem::path &currentDir, const std::string &rawlibname, const Token *token)
{
#ifndef _WIN32
	std::string libname = rawlibname + ".so";
	if (loaded.find(rawlibname) == loaded.end()) {
		auto library = dlopen(dir::findFile(currentDir, libname, token).string().c_str(), RTLD_LAZY);
		if (library == NULL) {
			std::vector<Function> stack_trace;
			throw RTError(format::format("External library does not exist: `{1}`", { libname }), *token, stack_trace);
		}
		auto f = dlsym(library, (rawlibname + "_rossaExportFunctions").c_str());
#else
	std::string libname = rawlibname + ".dll";
	if (loaded.find(rawlibname) == loaded.end()) {
		auto path = dir::findFile(currentDir, libname, token);
		libPaths.push_back(path);
		auto library = LoadLibraryA(path.string().c_str());
		if (library == NULL) {
			std::vector<Function> stack_trace;
			throw RTError(format::format("External library does not exist: {1}", { libname }), *token, stack_trace);
		}
		auto f = GetProcAddress(library, (rawlibname + "_rossaExportFunctions").c_str());
#endif
		if (f == NULL) {
			std::vector<Function> stack_trace;
			throw RTError(format::format("No export function found in library `{1}`", { libname }), *token, stack_trace);
		}
		std::map<std::string, extf_t> fns;
		auto ef = (export_fns_t)f;
		ef(fns);
		loaded[rawlibname] = fns;
	}
}

extf_t lib::loadFunction(const std::string & rawlibname, const std::string & fname, const Token * token)
{
	if (loaded.find(rawlibname) == loaded.end()) {
		std::vector<Function> stack_trace;
		throw RTError(format::format("Library has not yet been loaded into memory: {1}", { rawlibname }), *token, stack_trace);
	}
	if (loaded[rawlibname].find(fname) == loaded[rawlibname].end()) {
		std::vector<Function> stack_trace;
		throw RTError(format::format("Library `{1}` has not exported named function: {2}", { rawlibname, fname }), *token, stack_trace);
	}
	return loaded[rawlibname][fname];
}