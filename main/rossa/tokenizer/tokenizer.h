#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <filesystem>
#include <map>
#include <vector>
#include "../../number/number.h"

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
	TOK_EACH = -42,
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
	TOK_CONST = -62,
	TOK_WHERE = -63
};

const std::map<std::string, signed int> bOperators = {
	{"[]", 999},
	{"->", 14},
	{"**", -13},
	{"*", 12},
	{"/", 12},
	{"//", 12},
	{"%", 12},
	{"+", 11},
	{"++", 11},
	{"-", 11},
	{"<<", 10},
	{">>", 10},
	{"..", 0}, // undef
	{"<>", 0}, // undef
	{">", 8},
	{"<", 8},
	{">=", 8},
	{"<=", 8},
	{"===", 7},
	{"==", 7},
	{"!==", 7},
	{"!=", 7},
	{"!", 0}, // undef
	{".", 0}, // undef
	{"&", 6},
	{"^", 5},
	{"|", 4},
	{"&&", 3},
	{"||", 2},
	{"delete", 1},
	{"=", -1},
	{".=", -1},
	{":=", -1},
	{"**=", -1},
	{"*=", -1},
	{"/=", -1},
	{"//=", -1},
	{"%=", -1},
	{"+=", -1},
	{"++=", -1},
	{"-=", -1},
	{"&=", -1},
	{"|=", -1},
	{"^=", -1},
	{"||=", -1},
	{"&&=", -1},
	{">>=", -1},
	{"<<=", -1},
	{"...", 0},
	{":", 0},
	{"=>", 0},
	{"|>", 0}};

const std::map<std::string, signed int> uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1},
	{"~", -1},
	{"@", -1},
	{"$", -1}};

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

const char peekChar(
	const size_t &,
	const std::string &,
	const size_t &);

const char nextChar(
	const std::string &,
	size_t &,
	size_t &,
	size_t &);

const int getToken(
	const std::string &,
	size_t &,
	size_t &,
	size_t &,
	std::string &,
	number_t &);

const std::vector<token_t> lexString(
    const std::string &,
    const std::filesystem::path &);

#endif