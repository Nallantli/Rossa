#include "parser.h"
#include <fstream>

#include "../symbol/symbol.h"
#include "../node/node.h"
#include "../function/function.h"
#include "../instruction/instruction.h"
#include "../node_parser/node_parser.h"
#include "../global/global.h"

Hash parser_t::MAIN_HASH = Hash();

const hash_ull parser_t::HASH_INIT = ROSSA_HASH(KEYWORD_INIT);
const hash_ull parser_t::HASH_BLANK = ROSSA_HASH("");
const hash_ull parser_t::HASH_THIS = ROSSA_HASH(KEYWORD_THIS);
const hash_ull parser_t::HASH_DELETER = ROSSA_HASH(KEYWORD_DELETE_FUNC);

const hash_ull parser_t::HASH_ADD = ROSSA_HASH("+");
const hash_ull parser_t::HASH_SUB = ROSSA_HASH("-");
const hash_ull parser_t::HASH_MUL = ROSSA_HASH("*");
const hash_ull parser_t::HASH_DIV = ROSSA_HASH("/");
const hash_ull parser_t::HASH_MOD = ROSSA_HASH("%");
const hash_ull parser_t::HASH_POW = ROSSA_HASH("**");
const hash_ull parser_t::HASH_B_AND = ROSSA_HASH("&");
const hash_ull parser_t::HASH_B_OR = ROSSA_HASH("|");
const hash_ull parser_t::HASH_B_XOR = ROSSA_HASH("^");
const hash_ull parser_t::HASH_B_SH_L = ROSSA_HASH("<<");
const hash_ull parser_t::HASH_B_SH_R = ROSSA_HASH(">>");
const hash_ull parser_t::HASH_B_NOT = ROSSA_HASH("~");
const hash_ull parser_t::HASH_LESS = ROSSA_HASH("<");
const hash_ull parser_t::HASH_MORE = ROSSA_HASH(">");
const hash_ull parser_t::HASH_ELESS = ROSSA_HASH("<=");
const hash_ull parser_t::HASH_EMORE = ROSSA_HASH(">=");
const hash_ull parser_t::HASH_INDEX = ROSSA_HASH("[]");
const hash_ull parser_t::HASH_EQUALS = ROSSA_HASH("==");
const hash_ull parser_t::HASH_NEQUALS = ROSSA_HASH("!=");
const hash_ull parser_t::HASH_SET = ROSSA_HASH("=");
const hash_ull parser_t::HASH_CALL = ROSSA_HASH("()");
const hash_ull parser_t::HASH_RANGE = ROSSA_HASH("..");
const hash_ull parser_t::HASH_NOT = ROSSA_HASH("!");
const hash_ull parser_t::HASH_VAR_ARGS = ROSSA_HASH("_");
const hash_ull parser_t::HASH_LENGTH = ROSSA_HASH("len");
const hash_ull parser_t::HASH_CCT = ROSSA_HASH("++");
const hash_ull parser_t::HASH_DEL = ROSSA_HASH("delete");
const hash_ull parser_t::HASH_HASH = ROSSA_HASH("@");

parser_t::parser_t(const std::vector<std::string> &args)
{
	main = object_t(static_cast<hash_ull>(0));
#ifdef _WIN32
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
#endif
	std::vector<symbol_t> argv;
	for (auto &s : args)
		argv.push_back(symbol_t::String(s));
	scopes.push_back({ ROSSA_HASH("<*>") });
	consts.push_back({ {ROSSA_HASH("<*>"), ROSSA_HASH("_args")}, symbol_t::Array(argv) });
}

const std::map<std::string, signed int> parser_t::bOperators = {
	{"[]", 999},
	{"->", 14},
	{"**", -13},
	{"*", 12},
	{"/", 12},
	{"%", 12},
	{"+", 11},
	{"++", 11},
	{"-", 11},
	{"<<", 10},
	{">>", 10},
	{"..", 0}, //undef
	{"<>", 0}, //undef
	{">", 8},
	{"<", 8},
	{">=", 8},
	{"<=", 8},
	{"===", 7},
	{"==", 7},
	{"!==", 7},
	{"!=", 7},
	{"!", 0}, //undef
	{".", 0}, //undef
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
	{"|>", 0} };

const std::map<std::string, signed int> parser_t::uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1},
	{"~", -1},
	{"@", -1},
	{"$", -1} };

const ptr_node_t parser_t::compileCode(const std::string &code, const std::filesystem::path &currentFile)
{
	auto tokens = lexString(code, currentFile);
	node_parser_t testnp(tokens, currentFile);
	auto n = testnp.parse(&this->scopes, &this->consts);
	// fold twice (temporary) to refold constants
	return n->fold(this->consts)->fold(this->consts);
}

const symbol_t parser_t::runCode(const ptr_node_t &entry, const bool &tree)
{
	if (tree) {
		entry->printTree("", true);
		for (auto &c : consts) {
			int i = 0;
			for (auto &p : c.first) {
				if (i++ > 0)
					printc(".", CYAN_TEXT);
				printc(ROSSA_DEHASH(p), CYAN_TEXT);
			}
			std::cout << " = ";
			printc(c.second.toCodeString(), CYAN_TEXT);
			std::cout << "\n";
		}
	}

	auto g = node_parser_t::genParser(entry);

	trace_t stack_trace;
	return g->evaluate(&main, stack_trace);
}

void parser_t::printError(const rossa_error_t &e)
{
	printc(e.what(), RED_TEXT);
	std::cout << "\n";

	if (e.getToken().type != NULL_TOK) {
		std::string lineInfoRaw = "<" + e.getToken().filename.string() + ">:" + std::to_string(e.getToken().lineNumber + 1) + " | ";
		printc(lineInfoRaw, CYAN_TEXT);
		printc(e.getToken().line + "\n", MAGENTA_TEXT);

		std::string ret = "";
		for (size_t i = 0; i < e.getToken().distance - e.getToken().valueString.size() + lineInfoRaw.size(); i++)
			ret += " ";
		ret += "^";

		if (e.getToken().valueString.size() > 0)
			for (size_t i = 0; i < e.getToken().valueString.size() - 1; i++)
				ret += "~";
		printc(ret + "\n", RED_TEXT);
	}

	size_t j = 0;
	auto trace = e.getTrace();
	while (!trace.empty()) {
		if (j++ > 10) {
			printc(global::format(_STACK_TRACE_MORE_, { std::to_string(trace.size()) }), MAGENTA_TEXT);
			std::cout << "\n";
			trace.clear();
			break;
		}
		auto e = trace.back();
		printc(" ^ ", BLUE_TEXT);
		std::string ret = "";
		if (e.second.getParent().getKey() != "")
			ret += e.second.getParent().getKey() + ".";
		ret += ROSSA_DEHASH(e.second.key);
		printc(ret + "(", BRIGHT_BLACK_TEXT);
		size_t i = 0;
		for (auto &p : e.second.params) {
			if (i++ > 0)
				printc(", ", RESET_TEXT);
			switch (p.first) {
				case TOK_REF:
					printc("ref ", BLUE_TEXT);
					break;
				default:
					break;
			}
			printc(ROSSA_DEHASH(p.second), RESET_TEXT);
		}
		printc(")\n", BRIGHT_BLACK_TEXT);
		printc("\t<" + e.first.filename.string() + ">:" + std::to_string(e.first.lineNumber) + " | ", CYAN_TEXT);
		printc(e.first.line + "\n", MAGENTA_TEXT);
		trace.pop_back();
	}
}

const char parser_t::nextChar(
	const std::string &INPUT,
	size_t &INPUT_INDEX,
	size_t &LINE_INDEX,
	size_t &TOKEN_DIST)
{
	auto c = INPUT[INPUT_INDEX++];
	if (c == '\n') {
		LINE_INDEX++;
		TOKEN_DIST = 0;
	} else {
		TOKEN_DIST++;
	}
	return c;
}

const char parser_t::peekChar(
	const size_t &i,
	const std::string &INPUT,
	const size_t &INPUT_INDEX)
{
	if (INPUT_INDEX + i < INPUT.size())
		return INPUT[INPUT_INDEX + i];
	return 0;
}

const int parser_t::getToken(
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

	if (isalpha(last) || last == '_') {
		ID_STRING = last;
		while (isalnum(peekChar(0, INPUT, INPUT_INDEX)) || peekChar(0, INPUT, INPUT_INDEX) == '_') {
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
		else if (ID_STRING == "inf") {
			NUM_VALUE = number_t::Double(INFINITY);
			return TOK_NUM;
		} else if (ID_STRING == "nan") {
			NUM_VALUE = number_t::Double(NAN);
			return TOK_NUM;
		} else if (parser_t::bOperators.find(ID_STRING) != parser_t::bOperators.end() || parser_t::uOperators.find(ID_STRING) != parser_t::uOperators.end())
			return TOK_OPR;

		return TOK_IDF;
	} else if (isdigit(last) || (last == '.' && isdigit(peekChar(0, INPUT, INPUT_INDEX)))) {
		if (last == '0' && isalpha(peekChar(0, INPUT, INPUT_INDEX))) {
			char base = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			std::string numStr = "";
			while (isalnum(peekChar(0, INPUT, INPUT_INDEX))) {
				last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
				numStr += last;
			}
			switch (base) {
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
		while (isdigit(peekChar(0, INPUT, INPUT_INDEX)) || (peekChar(0, INPUT, INPUT_INDEX) == '.' && isdigit(peekChar(1, INPUT, INPUT_INDEX)))) {
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
	} else if (last == '#') {
		std::string commentStr = "";
		do {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			commentStr += last;
		} while (last != EOF && last != '\n' && last != '\r');

		ID_STRING = commentStr;
		return '#';
	} else if (last == EOF || last == 0)
		return TOK_EOF;
	else if (parser_t::bOperators.find(std::string(1, last)) != parser_t::bOperators.end()) {
		std::string opStr = std::string(1, last);
		while (parser_t::bOperators.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != parser_t::bOperators.end()) {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			opStr += last;
		}

		ID_STRING = opStr;
		if (ID_STRING == "=>")
			return TOK_DEF;
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
	} else if (parser_t::uOperators.find(std::string(1, last)) != parser_t::uOperators.end()) {
		std::string opStr = std::string(1, last);
		while (parser_t::uOperators.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != parser_t::uOperators.end()) {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			opStr += last;
		}

		ID_STRING = opStr;
		return TOK_OPR;
	} else if (last == '`') {
		std::string value = "";
		while (true) {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			if (last == '`') {
				ID_STRING = value;
				return TOK_IDF;
			}
			value += last;
		}
	} else if (last == '"') {
		std::string value = "";
		while (true) {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			if (last == '"') {
				ID_STRING = value;
				return TOK_STR_LIT;
			} else if (last == '\\') {
				switch (last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST)) {
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
						std::string code = std::string({ nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST) });
						char hex = std::stoul(code, nullptr, 16);
						last = hex;
						break;
					}
					case 'u':
					{
						std::string code = std::string({ nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST) });
						char hex = std::stoul(code, nullptr, 16);
						last = hex;
						break;
					}
				}
			}
			value += last;
		}
	} else if (last == '\'') {
		std::string value = "";
		while (true) {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			if (last == '\'') {
				if (value.size() < 1)
					value = std::string(1, 0);
				ID_STRING = value;
				NUM_VALUE = number_t::Long(static_cast<unsigned char>(value[0]));
				return TOK_NUM;
			} else if (last == '\\') {
				switch (last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST)) {
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
						std::string code = std::string({ nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST) });
						char hex = std::stoul(code, nullptr, 16);
						last = hex;
						break;
					}
					case 'u':
					{
						std::string code = std::string({ nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST), nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST) });
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

const std::vector<token_t> parser_t::lexString(const std::string &INPUT, const std::filesystem::path &filename)
{
	std::vector<std::string> LINES;
	std::stringstream ss(INPUT);
	std::string item;

	while (std::getline(ss, item, '\n')) {
		LINES.push_back(item);
	}

	std::vector<token_t> tokens;
	size_t INPUT_INDEX = 0;
	size_t LINE_INDEX = 0;
	size_t TOKEN_DIST = 0;
	std::string ID_STRING;
	number_t NUM_VALUE;

	while (true) {
		int token = getToken(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST, ID_STRING, NUM_VALUE);
		if (token == TOK_EOF)
			break;
		if (token == '#')
			continue;
		token_t t = { filename, LINES[LINE_INDEX], LINE_INDEX, TOKEN_DIST, ID_STRING, NUM_VALUE, token };

		if (t.type == TOK_DEF) {
			std::vector<token_t> temp;
			while (tokens.back().type != '(') {
				if (tokens.back().valueString == ">>") {
					temp.push_back({ tokens.back().filename, tokens.back().line, tokens.back().lineNumber, tokens.back().distance, ">", tokens.back().valueNumber, '>' });
					temp.push_back({ tokens.back().filename, tokens.back().line, tokens.back().lineNumber, tokens.back().distance, ">", tokens.back().valueNumber, '>' });
				} else if (tokens.back().valueString == "<<") {
					temp.push_back({ tokens.back().filename, tokens.back().line, tokens.back().lineNumber, tokens.back().distance, "<", tokens.back().valueNumber, '<' });
					temp.push_back({ tokens.back().filename, tokens.back().line, tokens.back().lineNumber, tokens.back().distance, "<", tokens.back().valueNumber, '<' });
				} else if (tokens.back().valueString == "<>") {
					temp.push_back({ tokens.back().filename, tokens.back().line, tokens.back().lineNumber, tokens.back().distance, "<", tokens.back().valueNumber, '<' });
					temp.push_back({ tokens.back().filename, tokens.back().line, tokens.back().lineNumber, tokens.back().distance, ">", tokens.back().valueNumber, '>' });
				} else {
					temp.push_back(tokens.back());
				}
				tokens.pop_back();
			}
			temp.push_back(tokens.back());
			tokens.pop_back();
			if (!tokens.empty() && (tokens.back().type == TOK_IDF || tokens.back().type == TOK_CHARN || tokens.back().type == TOK_CHARS || tokens.back().type == TOK_LENGTH || tokens.back().type == TOK_ALLOC || tokens.back().type == TOK_PARSE)) {
				temp.push_back(tokens.back());
				tokens.pop_back();
				tokens.push_back(t);
			} else {
				tokens.push_back({ filename, LINES[LINE_INDEX], LINE_INDEX, TOKEN_DIST, ID_STRING, NUM_VALUE, TOK_LAMBDA });
			}
			while (!temp.empty()) {
				tokens.push_back(temp.back());
				temp.pop_back();
			}
		} else {
			tokens.push_back(t);
		}
	}

	for (auto &t : tokens) {
		while (!t.line.empty() && isspace(t.line[0])) {
			t.line = t.line.substr(1);
			t.distance--;
		}
	}

	return tokens;
}

parser_t::~parser_t()
{}