#ifndef ROSSA_H
#define ROSSA_H

#include "number/number.h"
#include "Locale.h"

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>

#define _ROSSA_VERSION_ "v1.15.0-alpha"
#define COERCE_PTR(v, t) reinterpret_cast<t *>(v)

#define ROSSA_DEHASH(x) parser_t::MAIN_HASH.deHash(x)
#define ROSSA_HASH(x) parser_t::MAIN_HASH.hashValue(x)

#define ROSSA_EXT_SIG(name, args, token, hash, stack_trace) inline const symbol_t name(const std::vector<symbol_t> &args, const token_t *token, Hash &hash, trace_t &stack_trace)
#define ADD_EXT(name) fmap[#name] = name

#ifndef _WIN32
#include <limits.h>
#include <unistd.h>
#include <dlfcn.h>
#define colorASCII(c) "\033[" + std::to_string(c) + "m"
#define EXPORT_FUNCTIONS(name) extern "C" void name##_rossaExportFunctions(std::map<std::string, extf_t> &fmap)
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define EXPORT_FUNCTIONS(name) extern "C" __declspec(dllexport) void name##_rossaExportFunctions(std::map<std::string, extf_t> &fmap)
#endif

struct token_t;
struct symbol_t;
struct param_t;
struct signature_t;
struct object_t;
struct wrapper_t;
struct node_scope_t;

class Hash;
class Instruction;
class function_t;
class scope_t;
class Node;
class node_parser_t;
class parser_t;
class value_t;

typedef unsigned long long hash_ull;
typedef unsigned long long refc_ull;
typedef signed long long type_sll;

typedef std::shared_ptr<Node> ptr_node_t;
typedef std::shared_ptr<const Instruction> ptr_instruction_t;

typedef std::shared_ptr<function_t> ptr_function_t;

typedef std::vector<std::pair<token_t, function_t>> trace_t;
typedef std::vector<type_sll> aug_type_t;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

typedef const symbol_t(*extf_t)(const std::vector<symbol_t> &, const token_t *, Hash &, trace_t &);
typedef void (*export_fns_t)(std::map<std::string, extf_t> &);
typedef std::string(*cm_fns_t)();

struct node_scope_t
{
	hash_ull id;
	std::vector<hash_ull> var_ids;
};

enum value_type_enum
{
	NIL = -1,
	NUMBER = -2,
	BOOLEAN_D = -3,
	STRING = -4,
	ARRAY = -5,
	FUNCTION = -6,
	DICTIONARY = -7,
	OBJECT = -8,
	TYPE_NAME = -9,
	POINTER = -10,
	ANY = -11
};

enum token_type_enum
{
	NULL_TOK = 0,

	TOK_EOF = -1,

	TOK_IDF = -2,
	TOK_NUM = -3,

	TOK_OPR = -4,
	TOK_IF = -5,
	TOK_ELSE = -6,
	TOK_ELSEIF = -7,
	TOK_IN = -8,
	TOK_VAR = -9,
	TOK_DO = -10,
	TOK_WHILE = -11,
	TOK_THEN = -12,
	TOK_FOR = -13,
	TOK_DEF = -14,
	TOK_FALSE = -15,
	TOK_TRUE = -16,
	TOK_RETURN = -17,
	TOK_STR_LIT = -18,
	TOK_NIL = -19,
	TOK_EXTERN = -20,

	TOK_NUMBER = -21,
	TOK_STRING = -22,
	TOK_ARRAY = -23,
	TOK_BOOLEAN = -24,
	TOK_EXTERN_CALL = -25,

	TOK_LENGTH = -26,
	TOK_ANY = -27,
	TOK_ENUM = -28,
	TOK_STRUCT = -29,
	TOK_STATIC = -30,
	TOK_NEW = -31,
	TOK_TYPE_NAME = -32,
	TOK_DICTIONARY = -33,
	TOK_OBJECT = -34,
	TOK_FUNCTION = -35,
	TOK_OF = -36,

	TOK_LOAD = -37,
	TOK_ALLOC = -38,
	TOK_INNER = -39,
	TOK_REF = -40,
	TOK_CASE = -41,
	//TOK_DELETE = -42,
	TOK_BREAK = -43,
	TOK_REFER = -44,
	TOK_NIL_NAME = -45,
	TOK_POINTER = -46,
	TOK_VIRTUAL = -47,
	TOK_SWITCH = -48,
	TOK_TRY = -49,
	TOK_CATCH = -50,
	TOK_THROW = -51,
	TOK_CHARN = -52,
	TOK_CHARS = -53,
	TOK_LAMBDA = -54,
	TOK_UNTILT = -55,
	TOK_UNTILF = -56,
	TOK_PARSE = -57,
	TOK_CONTINUE = -58,
	TOK_CALL_OP = -59,
	TOK_NO_PARAM_LAMBDA = -60,
	TOK_VAR_ARGS = -61,
	TOK_CONST = -62
};

enum object_type_enum
{
	OBJECT_WEAK,
	OBJECT_STRONG
};

enum scope_type_enum
{
	SCOPE_BOUNDED,
	SCOPE_STRUCT,
	SCOPE_STATIC,
	SCOPE_INSTANCE,
	SCOPE_VIRTUAL
};

enum instruction_type_enum
{
	CONTAINER,
	VARIABLE,
	SEQUENCE,
	DECLARE,
	INDEX,
	INNER,
	IFELSE,
	WHILE,
	DEFINE,
	VARG_DEFINE,
	RETURN,
	EXTERN,
	LENGTH,
	CLASS_I,
	NEW_I,
	CAST_TO_I,
	POW_I,
	ALLOC_I,
	UNTIL_I,
	SCOPE_I,
	REFER_I,
	MAP_I,
	SWITCH_I,
	TRY_CATCH_I,
	THROW_I,
	CHARS_I,
	CHARN_I,
	FOR,
	SET,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	LESS,
	MORE,
	ELESS,
	EMORE,
	EQUALS,
	NEQUALS,
	PURE_EQUALS,
	PURE_NEQUALS,
	AND,
	OR,
	B_AND,
	B_OR,
	B_XOR,
	B_SH_L,
	B_SH_R,
	DECLARE_VARS_I,
B_NOT_I,
TYPE_I,
CALL_OP_I,
GET_THIS_I,
DELETE_I,
UN_ADD_I,
NEG_I,
NOT_I,
CONCAT_I,
SET_INDEX_I,
HASH_I
};

#ifndef _WIN32
enum TextColor
{
	BLACK_TEXT = 30,
	RED_TEXT = 31,
	GREEN_TEXT = 32,
	YELLOW_TEXT = 33,
	BLUE_TEXT = 34,
	MAGENTA_TEXT = 35,
	CYAN_TEXT = 36,
	WHITE_TEXT = 37,
	BRIGHT_BLACK_TEXT = 90,
	BRIGHT_RED_TEXT = 91,
	BRIGHT_GREEN_TEXT = 92,
	BRIGHT_YELLOW_TEXT = 93,
	BRIGHT_BLUE_TEXT = 94,
	BRIGHT_MAGENTA_TEXT = 95,
	BRIGHT_CYAN_TEXT = 96,
	BRIGHT_WHITE_TEXT = 97,

	RESET_TEXT = 0
};
#else
enum TextColor
{
	BLACK_TEXT = 0,
	RED_TEXT = FOREGROUND_RED,
	GREEN_TEXT = FOREGROUND_GREEN,
	YELLOW_TEXT = FOREGROUND_RED | FOREGROUND_GREEN,
	BLUE_TEXT = FOREGROUND_BLUE,
	MAGENTA_TEXT = FOREGROUND_RED | FOREGROUND_BLUE,
	CYAN_TEXT = FOREGROUND_GREEN | FOREGROUND_BLUE,
	WHITE_TEXT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	BRIGHT_BLACK_TEXT = FOREGROUND_INTENSITY,
	BRIGHT_RED_TEXT = FOREGROUND_RED | FOREGROUND_INTENSITY,
	BRIGHT_GREEN_TEXT = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	BRIGHT_YELLOW_TEXT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	BRIGHT_BLUE_TEXT = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	BRIGHT_MAGENTA_TEXT = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	BRIGHT_CYAN_TEXT = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	BRIGHT_WHITE_TEXT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,

	RESET_TEXT = WHITE_TEXT
};
#endif

inline void printc(const std::string &s, const TextColor &color)
{
#ifndef _WIN32
	std::cout << colorASCII(color) << s << colorASCII(0);
#else
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
	std::cout << s;
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

struct token_t
{
	std::filesystem::path filename;
	std::string line;
	size_t lineNumber;
	size_t distance;
	std::string valueString;
	number_t valueNumber;
	int type = NULL_TOK;
};

class Hash
{
private:
	std::vector<std::string> variable_hash;
public:
	Hash()
	{
		variable_hash.push_back("<LAMBDA>");
	}

	inline const hash_ull hashValue(const std::string &key)
	{
		if (std::find(variable_hash.begin(), variable_hash.end(), key) != variable_hash.end())
			return std::distance(variable_hash.begin(), std::find(variable_hash.begin(), variable_hash.end(), key));
		variable_hash.push_back(key);
		return variable_hash.size() - 1;
	}

	inline const std::string deHash(const hash_ull &code) const
	{
		return variable_hash[code];
	}

	inline const std::vector<std::string> getHashTable() const
	{
		return variable_hash;
	}
};

#endif