#ifndef LEXER_H
#define LEXER_H

#include "Ruota.h"

class Token
{
private:
	std::string filename;
	std::string valueString;
	std::string line;
	NUMBER_TYPE valueNumber;
	int type;
	size_t distance;
	size_t lineNumber;

public:
	Token();
	Token(const std::string &, const std::string &, size_t, size_t, const std::string &, NUMBER_TYPE, int);
	const std::string &getLine() const;
	int getType() const;
	size_t getDist() const;
	size_t getLineNumber() const;
	NUMBER_TYPE getValueNumber() const;
	const std::string &getValueString() const;
	const std::string &getFilename() const;
};

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