#include "Ruota.h"

Token::Token() : filename("nil"),
line(""),
lineNumber(0),
distance(0),
valueString(""),
valueNumber(CNumber()),
type(NULL_TOK)
{}

Token::Token(
	const string &filename,
	const string &line,
	size_t lineNumber,
	size_t distance,
	const string &valueString,
	CNumber valueNumber,
	int type) : filename(filename),
	line(line),
	lineNumber(lineNumber),
	distance(distance),
	valueString(valueString),
	valueNumber(valueNumber),
	type(type)
{
	while (!this->line.empty() && isspace(this->line[0])) {
		this->line = this->line.substr(1);
		this->distance--;
	}
}

const string &Token::getLine() const
{
	return this->line;
}

int Token::getType() const
{
	return this->type;
}

size_t Token::getDist() const
{
	return this->distance;
}

size_t Token::getLineNumber() const
{
	return this->lineNumber;
}

const CNumber Token::getValueNumber() const
{
	return this->valueNumber;
}

const string &Token::getValueString() const
{
	return this->valueString;
}

const string &Token::getFilename() const
{
	return this->filename;
}