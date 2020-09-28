#ifndef LEXER_H
#define LEXER_H

#include "Ruota.h"
class Lexer
{
private:
	std::string ID_STRING;
	size_t LINE_INDEX;
	NUMBER_TYPE NUM_VALUE;
	std::string INPUT;
	size_t INPUT_INDEX;
	size_t TOKEN_DIST;

	int getToken();
	char peekChar() const;
	char nextChar();
	std::map<std::string, signed int> bOperators;
	std::map<std::string, signed int> uOperators;

public:
	Lexer(std::map<std::string, signed int>, std::map<std::string, signed int>);
	std::vector<Token> lexString(const std::string &, const std::string &);
};

#endif