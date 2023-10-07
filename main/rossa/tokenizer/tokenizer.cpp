#include "tokenizer.h"

#include "../Keywords.h"

const char peekChar(
	const size_t &i,
	const std::string &INPUT,
	const size_t &INPUT_INDEX)
{
	if (INPUT_INDEX + i < INPUT.size())
	{
		return INPUT[INPUT_INDEX + i];
	}
	return 0;
}

const char nextChar(
	const std::string &INPUT,
	size_t &INPUT_INDEX,
	size_t &LINE_INDEX,
	size_t &TOKEN_DIST)
{
	auto c = INPUT[INPUT_INDEX++];
	if (c == '\n')
	{
		LINE_INDEX++;
		TOKEN_DIST = 0;
	}
	else
	{
		TOKEN_DIST++;
	}
	return c;
}

const int getToken(
	const std::string &INPUT,
	size_t &INPUT_INDEX,
	size_t &LINE_INDEX,
	size_t &TOKEN_DIST,
	std::string &ID_STRING,
	number_t &NUM_VALUE)
{
	static int last;
	while (isspace(last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST)))
		;

	if (isalpha(last) || last == '_')
	{
		ID_STRING = last;
		while (isalnum(peekChar(0, INPUT, INPUT_INDEX)) || peekChar(0, INPUT, INPUT_INDEX) == '_')
		{
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			ID_STRING += last;
		}

		if (ID_STRING == KEYWORD_THEN)
			return TOK_THEN;
		else if (ID_STRING == KEYWORD_ELSE)
			return TOK_ELSE;
		else if (ID_STRING == KEYWORD_DO)
			return TOK_DO;
		else if (ID_STRING == KEYWORD_IN)
			return TOK_IN;
		else if (ID_STRING == KEYWORD_OF)
			return TOK_OF;
		else if (ID_STRING == KEYWORD_VAR)
			return TOK_VAR;
		else if (ID_STRING == KEYWORD_IF)
			return TOK_IF;
		else if (ID_STRING == KEYWORD_WHILE)
			return TOK_WHILE;
		else if (ID_STRING == KEYWORD_ELIF)
			return TOK_ELSEIF;
		else if (ID_STRING == KEYWORD_FOR)
			return TOK_FOR;
		else if (ID_STRING == KEYWORD_TRUE)
			return TOK_TRUE;
		else if (ID_STRING == KEYWORD_FALSE)
			return TOK_FALSE;
		else if (ID_STRING == KEYWORD_RETURN)
			return TOK_RETURN;
		else if (ID_STRING == KEYWORD_NIL)
			return TOK_NIL;
		else if (ID_STRING == KEYWORD_NUMBER)
			return TOK_NUMBER;
		else if (ID_STRING == KEYWORD_STRING)
			return TOK_STRING;
		else if (ID_STRING == KEYWORD_ARRAY)
			return TOK_ARRAY;
		else if (ID_STRING == KEYWORD_BOOLEAN)
			return TOK_BOOLEAN;
		else if (ID_STRING == KEYWORD_DICTIONARY)
			return TOK_DICTIONARY;
		else if (ID_STRING == KEYWORD_OBJECT)
			return TOK_OBJECT;
		else if (ID_STRING == KEYWORD_FUNCTION)
			return TOK_FUNCTION;
		else if (ID_STRING == KEYWORD_TYPE)
			return TOK_TYPE_NAME;
		else if (ID_STRING == KEYWORD_EXTERN)
			return TOK_EXTERN;
		else if (ID_STRING == KEYWORD_EXTERN_CALL)
			return TOK_EXTERN_CALL;
		else if (ID_STRING == KEYWORD_LENGTH)
			return TOK_LENGTH;
		else if (ID_STRING == KEYWORD_STRUCT)
			return TOK_STRUCT;
		else if (ID_STRING == KEYWORD_STATIC)
			return TOK_STATIC;
		else if (ID_STRING == KEYWORD_ENUM)
			return TOK_ENUM;
		else if (ID_STRING == KEYWORD_NEW)
			return TOK_NEW;
		else if (ID_STRING == KEYWORD_LOAD)
			return TOK_LOAD;
		else if (ID_STRING == KEYWORD_ALLOC)
			return TOK_ALLOC;
		else if (ID_STRING == KEYWORD_REF)
			return TOK_REF;
		else if (ID_STRING == KEYWORD_BREAK)
			return TOK_BREAK;
		else if (ID_STRING == KEYWORD_REFER)
			return TOK_REFER;
		else if (ID_STRING == KEYWORD_NIL_NAME)
			return TOK_NIL_NAME;
		else if (ID_STRING == KEYWORD_POINTER)
			return TOK_POINTER;
		else if (ID_STRING == KEYWORD_VIRTUAL)
			return TOK_VIRTUAL;
		else if (ID_STRING == KEYWORD_SWITCH)
			return TOK_SWITCH;
		else if (ID_STRING == KEYWORD_TRY)
			return TOK_TRY;
		else if (ID_STRING == KEYWORD_CATCH)
			return TOK_CATCH;
		else if (ID_STRING == KEYWORD_THROW)
			return TOK_THROW;
		else if (ID_STRING == KEYWORD_CHAR_N)
			return TOK_CHARN;
		else if (ID_STRING == KEYWORD_CHAR_S)
			return TOK_CHARS;
		else if (ID_STRING == KEYWORD_CASE)
			return TOK_CASE;
		else if (ID_STRING == KEYWORD_PARSE)
			return TOK_PARSE;
		else if (ID_STRING == KEYWORD_CONTINUE)
			return TOK_CONTINUE;
		else if (ID_STRING == KEYWORD_CALL_OP)
			return TOK_CALL_OP;
		else if (ID_STRING == KEYWORD_ANY)
			return TOK_ANY;
		else if (ID_STRING == KEYWORD_CONST)
			return TOK_CONST;
		else if (ID_STRING == KEYWORD_EACH)
			return TOK_EACH;
		else if (ID_STRING == KEYWORD_WHERE)
			return TOK_WHERE;
		else if (ID_STRING == KEYWORD_DEF)
			return TOK_DEF;
		else if (ID_STRING == "inf")
		{
			NUM_VALUE = number_t::Double(INFINITY);
			return TOK_NUM;
		}
		else if (ID_STRING == "nan")
		{
			NUM_VALUE = number_t::Double(NAN);
			return TOK_NUM;
		}
		else if (BINARY_OPERATORS.find(ID_STRING) != BINARY_OPERATORS.end() || UNARY_OPERATORS.find(ID_STRING) != UNARY_OPERATORS.end())
			return TOK_OPR;

		return TOK_IDF;
	}
	else if (isdigit(last) || (last == '.' && isdigit(peekChar(0, INPUT, INPUT_INDEX))))
	{
		if (last == '0' && isalpha(peekChar(0, INPUT, INPUT_INDEX)))
		{
			char base = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			std::string numStr = "";
			while (isalnum(peekChar(0, INPUT, INPUT_INDEX)))
			{
				last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
				numStr += last;
			}
			switch (base)
			{
			case 'b':
			case 'B':
				NUM_VALUE = number_t::Long(std::stoll(numStr, nullptr, 2));
				break;
			default:
				NUM_VALUE = number_t::Long(std::stoll("0" + std::string(1, base) + numStr, nullptr, 0));
				break;
			}
			ID_STRING = "0" + std::string(1, base) + numStr;
			return TOK_NUM;
		}

		std::string numStr = std::string(1, last);
		bool flag = false;
		while (isdigit(peekChar(0, INPUT, INPUT_INDEX)) || (peekChar(0, INPUT, INPUT_INDEX) == '.' && isdigit(peekChar(1, INPUT, INPUT_INDEX))))
		{
			if (flag && peekChar(0, INPUT, INPUT_INDEX) == '.')
				break;
			if (!flag)
				flag = last == '.';
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			numStr += last;
		}

		ID_STRING = numStr;
		if (flag)
			NUM_VALUE = number_t::Double(std::stold(numStr));
		else
			NUM_VALUE = number_t::Long(std::stoll(numStr, nullptr, 10));
		return TOK_NUM;
	}
	else if (last == '#')
	{
		std::string commentStr = "";
		do
		{
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			commentStr += last;
		} while (last != EOF && last != '\n' && last != '\r');

		ID_STRING = commentStr;
		return '#';
	}
	else if (last == EOF || last == 0)
		return TOK_EOF;
	else if (BINARY_OPERATORS.find(std::string(1, last)) != BINARY_OPERATORS.end())
	{
		std::string opStr = std::string(1, last);
		while (BINARY_OPERATORS.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != BINARY_OPERATORS.end())
		{
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			opStr += last;
		}

		ID_STRING = opStr;
		if (ID_STRING == ".")
			return TOK_INNER;
		if (ID_STRING == "..")
			return TOK_UNTILF;
		if (ID_STRING == "<>")
			return TOK_UNTILT;
		if (ID_STRING == "...")
			return TOK_VAR_ARGS;
		if (ID_STRING == "|>")
			return TOK_NO_PARAM_LAMBDA;
		if (ID_STRING == ":")
			return ':';
		return TOK_OPR;
	}
	else if (UNARY_OPERATORS.find(std::string(1, last)) != UNARY_OPERATORS.end())
	{
		std::string opStr = std::string(1, last);
		while (UNARY_OPERATORS.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != UNARY_OPERATORS.end())
		{
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			opStr += last;
		}

		ID_STRING = opStr;
		return TOK_OPR;
	}
	else if (last == '`')
	{
		std::string value = "";
		while (true)
		{
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			if (last == '`')
			{
				ID_STRING = value;
				return TOK_IDF;
			}
			value += last;
		}
	}
	else if (last == '"')
	{
		std::string value = "";
		while (true)
		{
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			if (last == '"')
			{
				ID_STRING = value;
				return TOK_STR_LIT;
			}
			else if (last == '\\')
			{
				switch (last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST))
				{
				case '"':
					last = '"';
					break;
				case '\'':
					last = '\'';
					break;
				case 'n':
					last = '\n';
					break;
				case '?':
					last = '\?';
					break;
				case 'a':
					last = '\a';
					break;
				case 'b':
					last = '\b';
					break;
				case 'f':
					last = '\f';
					break;
				case 'r':
					last = '\r';
					break;
				case 't':
					last = '\t';
					break;
				case 'v':
					last = '\v';
					break;
				case '0':
					last = '\0';
					break;
				case 'x':
				{
					std::string code = std::string({nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST)});
					char hex = std::stoul(code, nullptr, 16);
					last = hex;
					break;
				}
				case 'u':
				{
					std::string code = std::string({nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST)});
					char hex = std::stoul(code, nullptr, 16);
					last = hex;
					break;
				}
				}
			}
			value += last;
		}
	}
	else if (last == '\'')
	{
		std::string value = "";
		while (true)
		{
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			if (last == '\'')
			{
				if (value.size() < 1)
					value = std::string(1, 0);
				ID_STRING = value;
				NUM_VALUE = number_t::Long(static_cast<unsigned char>(value[0]));
				return TOK_NUM;
			}
			else if (last == '\\')
			{
				switch (last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST))
				{
				case '"':
					last = '"';
					break;
				case '\'':
					last = '\'';
					break;
				case 'n':
					last = '\n';
					break;
				case '?':
					last = '\?';
					break;
				case 'a':
					last = '\a';
					break;
				case 'b':
					last = '\b';
					break;
				case 'f':
					last = '\f';
					break;
				case 'r':
					last = '\r';
					break;
				case 't':
					last = '\t';
					break;
				case 'v':
					last = '\v';
					break;
				case '0':
					last = '\0';
					break;
				case 'x':
				{
					std::string code = std::string({nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST)});
					char hex = std::stoul(code, nullptr, 16);
					last = hex;
					break;
				}
				case 'u':
				{
					std::string code = std::string({nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST)});
					char hex = std::stoul(code, nullptr, 16);
					last = hex;
					break;
				}
				}
			}
			value += last;
		}
	}

	int ret = last;
	ID_STRING = last;
	return ret;
}

const std::vector<token_t> lexString(const std::string &INPUT, const std::filesystem::path &filename)
{
	std::vector<std::string> LINES;
	std::stringstream ss(INPUT);
	std::string item;

	while (std::getline(ss, item, '\n'))
	{
		LINES.push_back(item);
	}

	std::vector<token_t> tokens;
	size_t INPUT_INDEX = 0;
	size_t LINE_INDEX = 0;
	size_t TOKEN_DIST = 0;
	std::string ID_STRING;
	number_t NUM_VALUE;

	bool in_sig = false;

	while (true)
	{
		int token = getToken(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST, ID_STRING, NUM_VALUE);
		if (token == TOK_EOF)
			break;
		if (token == '#')
			continue;
		token_t t = {filename, LINES[LINE_INDEX], LINE_INDEX, TOKEN_DIST, ID_STRING, NUM_VALUE, token};
		if (t.type == TOK_DEF)
		{
			in_sig = true;
			tokens.push_back(t);
		}
		else
		{
			if (in_sig)
			{
				if (t.type == '(' && tokens.back().type == TOK_DEF)
				{
					tokens.back().type = TOK_LAMBDA;
				}
				if (t.type != TOK_IDF && t.valueString == ">>")
				{
					tokens.push_back({t.filename, t.line, t.lineNumber, t.distance, ">", t.valueNumber, '>'});
					tokens.push_back({t.filename, t.line, t.lineNumber, t.distance, ">", t.valueNumber, '>'});
				}
				else if (t.type != TOK_IDF && t.valueString == "<<")
				{
					tokens.push_back({t.filename, t.line, t.lineNumber, t.distance, "<", t.valueNumber, '<'});
					tokens.push_back({t.filename, t.line, t.lineNumber, t.distance, "<", t.valueNumber, '<'});
				}
				else if (t.type != TOK_IDF && t.valueString == "<>")
				{
					tokens.push_back({t.filename, t.line, t.lineNumber, t.distance, "<", t.valueNumber, '<'});
					tokens.push_back({t.filename, t.line, t.lineNumber, t.distance, ">", t.valueNumber, '>'});
				}
				else
				{
					tokens.push_back(t);
				}
				if (tokens.back().type == ')')
				{
					in_sig = false;
				}
			}
			else
			{
				tokens.push_back(t);
			}
		}
	}

	for (auto &t : tokens)
	{
		while (!t.line.empty() && isspace(t.line[0]))
		{
			t.line = t.line.substr(1);
			t.distance--;
		}
	}

	return tokens;
}
