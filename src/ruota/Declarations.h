#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#define _RUOTA_VERSION_ "v1.5.1-alpha"
#define RUOTA_EXT_SYM(name, args, token, hash) extern "C" BOOST_SYMBOL_EXPORT const Symbol name(std::vector<Symbol> args, const Token *token, Hash &hash)
#define RUOTA_LIB_HEADER Hash MAIN_HASH = Hash();

#include <vector>
#include <string>
#include <algorithm>
#include <boost/config.hpp>

typedef unsigned long long hashcode_t;
typedef signed long long object_type_t;

struct Hash
{
	std::vector<std::string> variable_hash;

	Hash()
	{
		variable_hash.push_back("<LAMBDA>");
	}

	inline hashcode_t hashString(const std::string &key)
	{
		if (std::find(variable_hash.begin(), variable_hash.end(), key) != variable_hash.end())
			return std::distance(variable_hash.begin(), std::find(variable_hash.begin(), variable_hash.end(), key));
		variable_hash.push_back(key);
		return variable_hash.size() - 1;
	}

	inline std::string deHash(hashcode_t code)
	{
		return variable_hash[code];
	}
};

extern Hash MAIN_HASH;

struct Signature;
class Token;
class Instruction;
class Function;
class Scope;
class Object;
class Node;
class Lexer;
class Ruota;
class Value;
class Symbol;

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

enum LexerTokenType
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
	TOK_VECTOR = -23,
	TOK_BOOLEAN = -24,
	TOK_EXTERN_CALL = -25,

	TOK_LENGTH = -26,
	TOK_SIZE = -27,
	TOK_CLASS = -28,
	TOK_STRUCT = -29,
	TOK_STATIC = -30,
	TOK_NEW = -31,
	TOK_TYPE_NAME = -32,
	TOK_DICTIONARY = -33,
	TOK_OBJECT = -34,
	TOK_FUNCTION = -35,
	TOK_CAST = -36,

	TOK_LOAD = -37,
	TOK_ALLOC = -38,
	TOK_INNER = -39,
	TOK_REF = -40,
	TOK_CASE = -41,
	TOK_DEF_TYPE = -42,
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
	TOK_UNTILF = -56
};

enum SymbolType
{
	ID_RETURN,
	ID_BREAK,
	ID_CASUAL,
	ID_REFER
};

enum ValueType
{
	NIL = -1,
	NUMBER = -2,
	BOOLEAN_D = -3,
	STRING = -4,
	VECTOR = -5,
	FUNCTION = -6,
	DICTIONARY = -7,
	OBJECT = -8,
	TYPE_NAME = -9,
	POINTER = -10
};

enum InstructionType
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
	RETURN,
	EXTERN,
	LENGTH,
	SIZE_I,
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
	B_SH_R
};

enum ObjectType
{
	STRUCT_O,
	STATIC_O,
	INSTANCE_O,
	VIRTUAL_O
};

enum NodeType
{
	ENTRY_NODE,
	CALL_NODE,
	CALL_BUILT_NODE,
	INDEX_NODE,
	UN_OP_NODE,
	BIN_OP_NODE,
	INS_NODE,
	IF_ELSE_NODE,
	WHILE_NODE,
	FOR_NODE,
	VECTOR_NODE,
	DEFINE_NODE,
	RETURN_NODE,
	ID_NODE,
	VAR_NODE,
	CLASS_NODE,
	EXTERN_CALL_NODE,
	NEW_NODE,
	CAST_TO_NODE,
	UNTIL_NODE,
	MAP_NODE,
	CONTAINER_NODE,
	BREAK_NODE,
	REFER_NODE,
	SWITCH_NODE,
	BID_NODE,
	TRY_CATCH_NODE,
	THROW_NODE
};

inline std::string getTypeString(const object_type_t &i)
{
	if (i >= 0)
		return MAIN_HASH.deHash(i);
	else
	{
		switch (i)
		{
		case NIL:
			return "Nil";
		case NUMBER:
			return "Number";
		case STRING:
			return "String";
		case BOOLEAN_D:
			return "Boolean";
		case VECTOR:
			return "Vector";
		case FUNCTION:
			return "Function";
		case DICTIONARY:
			return "Dictionary";
		case OBJECT:
			return "Object";
		case POINTER:
			return "Pointer";
		case TYPE_NAME:
			return "Type";
		default:
			return "<error-type>";
		}
	}
}

#endif