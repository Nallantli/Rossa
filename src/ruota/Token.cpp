#include "Ruota.h"

Token::Token() : filename("nil"),
				 line(""),
				 distance(0),
				 valueString(""),
				 valueNumber(CNumber()),
				 type(NULL_TOK),
				 lineNumber(0) {}

Token::Token(
	const std::string &filename,
	const std::string &line,
	size_t lineNumber,
	size_t distance,
	const std::string &valueString,
	CNumber valueNumber,
	int type) : filename(filename),
				line(line),
				distance(distance),
				valueString(valueString),
				lineNumber(lineNumber),
				valueNumber(valueNumber),
				type(type)
{
	while (!this->line.empty() && isspace(this->line[0]))
	{
		this->line = this->line.substr(1);
		this->distance--;
	}
}

const std::string &Token::getLine() const
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

const std::string &Token::getValueString() const
{
	return this->valueString;
}

const std::string &Token::getFilename() const
{
	return this->filename;
}