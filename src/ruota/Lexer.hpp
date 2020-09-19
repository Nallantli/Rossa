#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>

enum TokenType
{
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
	TOK_EXTERN_CALL = -25
};

class Token
{
private:
	std::string valueString;
	std::string line;
	double valueNumber;
	int type;
	unsigned long distance;

public:
	Token() : line(""), distance(0), valueString(""), valueNumber(0), type(0) {}
	Token(const std::string &line, unsigned long distance, const std::string &, double, int);
	const std::string getLine() const { return this->line; };
	const int getType() const { return this->type; }
	const unsigned long getDist() const { return this->distance; }
	const double getValueNumber() const { return this->valueNumber; }
	const std::string getValueString() const { return this->valueString; }
};

class Lexer
{
private:
	std::string ID_STRING;
	unsigned long LINE_INDEX;
	double NUM_VALUE;
	std::string INPUT;
	unsigned long INPUT_INDEX;
	unsigned long TOKEN_DIST;
	const int getToken();
	const char peekChar() const;
	const char nextChar();
	std::map<std::string, signed int> bOperators;
	std::map<std::string, signed int> uOperators;

public:
	Lexer(std::map<std::string, signed int> bOperators, std::map<std::string, signed int> uOperators) : bOperators(bOperators), uOperators(uOperators) {}
	std::vector<Token> lexString(const std::string &);
};

#endif