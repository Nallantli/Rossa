#include "../../bin/include/Rossa.h"

using namespace rossa;

const size_t sig::validity(const sig_t &values, const sym_vec_t &check, trace_t &stack_trace)
{
	if (values.size() == 0)
		return 1;

	size_t v = 0;
	for (size_t i = 0; i < values.size(); i++) {
		type_sll vt = check[i].getAugValueType();
		if (values[i].getQualifiers().empty()) {
			auto base = values[i].getBase();
			if (base == vt)
				v += 3;
			else if (base > 0 && check[i].getValueType() == NIL)
				v += 2;
			else if (base == ANY)
				v += 1;
			else if (check[i].getValueType() == OBJECT) {
				if (base == OBJECT || check[i].getObject(NULL, stack_trace)->extendsObject(base))
					v += 2;
				else
					return 0;
			} else
				return 0;
		} else {
			auto ql = values[i].getQualifiers();
			auto fo = check[i].getFunctionOverloads(NULL, stack_trace);
			if (fo.find(ql.size()) == fo.end()) {
				if (check[i].hasVarg(NULL, stack_trace))
					v += 1;
				else
					return 0;
			} else {
				size_t flag = 0;
				for (auto f : fo[ql.size()]) {
					for (size_t i = 0; i < ql.size(); i++) {
						auto val = ql[i] & f.first[i];
						if (val > flag) {
							flag = val;
							if (val == 3)
								break;
						}
					}
					if (flag == 3)
						break;
				}
				if (flag > 0)
					v += flag;
				else {
					if (check[i].hasVarg(NULL, stack_trace))
						v += 1;
					else
						return 0;
				}
			}
		}
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
		s += v.toCodeString();
	}
	return s + "}";
}

const std::string sig::toString(const sig_t &values)
{
	std::string s = "";
	size_t i = 0;
	for (auto &v : values) {
		if (i++ > 0)
			s += ", ";
		s += v.toString();
	}
	return s;
}


const std::string sig::getTypeString(const type_sll &i)
{
	if (i >= 0)
		return "@" + ROSSA_DEHASH(i);
	else {
		switch (i) {
			case NIL:
				return KEYWORD_NIL_NAME;
			case ANY:
				return KEYWORD_ANY;
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
std::map<std::string, std::string> lib::compilerCommands = {};

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
	trace_t stack_trace;
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
	if (loaded.find(rawlibname) == loaded.end()) {
#ifndef _WIN32
		std::string libname = rawlibname + ".so";
		auto library = dlopen(dir::findFile(currentDir, libname, token).string().c_str(), RTLD_LAZY);
		if (library == NULL) {
			trace_t stack_trace;
			throw RTError(format::format(_EXTERNAL_LIBRARY_NOT_EXIST_, { libname }), *token, stack_trace);
		}
		auto f = dlsym(library, (rawlibname + "_rossaExportFunctions").c_str());
		auto cm = dlsym(library, (rawlibname + "_rossaCompilerCommands").c_str());
#else
		std::string libname = rawlibname + ".dll";
		auto path = dir::findFile(currentDir, libname, token);
		auto library = LoadLibraryA(path.string().c_str());
		if (library == NULL) {
			trace_t stack_trace;
			throw RTError(format::format(_EXTERNAL_LIBRARY_NOT_EXIST_, { libname }), *token, stack_trace);
		}
		auto f = GetProcAddress(library, (rawlibname + "_rossaExportFunctions").c_str());
		auto cm = GetProcAddress(library, (rawlibname + "_rossaCompilerCommands").c_str());
#endif
		if (f == NULL) {
			trace_t stack_trace;
			throw RTError(format::format(_EXPORT_FUNCTION_NOT_FOUND_, { libname }), *token, stack_trace);
		}
		std::map<std::string, extf_t> fns;
		auto ef = (export_fns_t)f;
		ef(fns);
		loaded[rawlibname] = fns;

		auto ecm = (cm_fns_t)cm;
		compilerCommands[rawlibname] = ecm();
	}
}

extf_t lib::loadFunction(const std::string &rawlibname, const std::string &fname, const Token *token)
{
	if (loaded.find(rawlibname) == loaded.end()) {
		trace_t stack_trace;
		throw RTError(format::format(_LIBRARY_NOT_IN_MEMORY_, { rawlibname }), *token, stack_trace);
	}
	if (loaded[rawlibname].find(fname) == loaded[rawlibname].end()) {
		trace_t stack_trace;
		throw RTError(format::format(_LIBRARY_FUNCTION_NOT_EXIST_, { rawlibname, fname }), *token, stack_trace);
	}
	return loaded[rawlibname][fname];
}

ParamType::ParamType(const type_sll &base)
	: base{ base }
{}

ParamType::ParamType(const type_sll &base, const std::vector<ParamType> &qualifiers)
	: base{ base }
	, qualifiers{ qualifiers }
{}

void ParamType::addQualifier(const ParamType &param)
{
	this->qualifiers.push_back(param);
}

const std::string ParamType::toString() const
{
	std::string s = sig::getTypeString(base);
	if (!qualifiers.empty()) {
		s += "<";
		size_t i = 0;
		for (auto &v : qualifiers) {
			if (i++ > 0)
				s += ", ";
			s += v.toString();
		}
		s += ">";
	}
	return s;
}

const std::string ParamType::toCodeString() const
{
	std::string s = "ParamType(static_cast<type_sll>(" + std::to_string(base) + "), ";
	s += "{";
	size_t i = 0;
	for (auto &v : qualifiers) {
		if (i++ > 0)
			s += ", ";
		s += v.toCodeString();
	}
	s += "}";
	return s + ")";
}

const std::vector<ParamType> ParamType::getQualifiers() const
{
	return this->qualifiers;
}

const type_sll ParamType::getBase() const
{
	return this->base;
}

const bool ParamType::operator<(const ParamType &pt) const
{
	if (base != pt.base)
		return base < pt.base;
	if (qualifiers.size() != pt.qualifiers.size())
		return qualifiers.size() < pt.qualifiers.size();
	for (size_t i = 0; i < qualifiers.size(); i++) {
		if (qualifiers[i] < pt.qualifiers[i])
			return true;
	}
	return false;
}

// Function<Number, Function<Function, Nil>>
// Function<Nil, Function<Function, Number>>
// Function<Nil, Function<@extendsF<Number>, Function<String>>>
const size_t ParamType::operator&(const ParamType &pt) const
{
	if (base == ANY)
		return 2;
	if (base != pt.base && pt.base < 0)
		return 0;
	if (qualifiers.empty() && pt.qualifiers.empty())
		return 3;
	if (qualifiers.empty() && !pt.qualifiers.empty())
		return 2;
	if (!qualifiers.empty() && qualifiers.size() != pt.qualifiers.size())
		return 0;
	size_t v = 3;
	for (size_t i = 0; i < qualifiers.size(); i++) {
		auto val = qualifiers[i] & pt.qualifiers[i];
		if (val == 0)
			return 0;
		if (val < v)
			v = val;
	}
	return v;
}