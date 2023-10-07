#ifndef ROSSA_H
#define ROSSA_H

#include "../number/number.h"
#include "Locale.h"

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>

#define _ROSSA_VERSION_ "v1.18.2-alpha"


#define ROSSA_DEHASH(x) parser_t::MAIN_HASH.deHash(x)
#define ROSSA_HASH(x) parser_t::MAIN_HASH.hashValue(x)

#ifndef _WIN32
#include <limits.h>
#include <unistd.h>
#include <dlfcn.h>
#define colorASCII(c) "\033[" + std::to_string(c) + "m"
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

struct symbol_t;
struct parameter_t;
struct signature_t;
struct object_t;
struct wrapper_t;

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
typedef std::vector<type_sll> aug_type_t;

typedef std::string (*cm_fns_t)();

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