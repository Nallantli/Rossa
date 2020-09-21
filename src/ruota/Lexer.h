#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>

#include "RuotaTypes.h"

class Token
{
private:
	std::string valueString;
	std::string line;
	Number valueNumber;
	int type;
	unsigned long distance;

public:
	Token() : line(""), distance(0), valueString(""), valueNumber((long_int_t)0), type(0) {}
	Token(const std::string &line, unsigned long distance, const std::string &, Number, int);
	const std::string getLine() const { return this->line; };
	const int getType() const { return this->type; }
	const unsigned long getDist() const { return this->distance; }
	const Number getValueNumber() const { return this->valueNumber; }
	const std::string getValueString() const { return this->valueString; }
};

class Lexer
{
private:
	std::string ID_STRING;
	unsigned long LINE_INDEX;
	Number NUM_VALUE;
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