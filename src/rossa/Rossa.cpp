#include "Rossa.h"

using namespace rossa;

std::vector<boost::filesystem::path> dir::loaded = {};
std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)>> lib::loaded = {};

Hash Rossa::MAIN_HASH = Hash();

const hash_ull Rossa::HASH_INIT = ROSSA_HASH(KEYWORD_INIT);
const hash_ull Rossa::HASH_BLANK = ROSSA_HASH("");
const hash_ull Rossa::HASH_THIS = ROSSA_HASH(KEYWORD_THIS);
const hash_ull Rossa::HASH_DELETER = ROSSA_HASH("~");

const hash_ull Rossa::HASH_ADD = ROSSA_HASH("+");
const hash_ull Rossa::HASH_SUB = ROSSA_HASH("-");
const hash_ull Rossa::HASH_MUL = ROSSA_HASH("*");
const hash_ull Rossa::HASH_DIV = ROSSA_HASH("/");
const hash_ull Rossa::HASH_MOD = ROSSA_HASH("%");
const hash_ull Rossa::HASH_POW = ROSSA_HASH("**");
const hash_ull Rossa::HASH_B_AND = ROSSA_HASH("&");
const hash_ull Rossa::HASH_B_OR = ROSSA_HASH("|");
const hash_ull Rossa::HASH_B_XOR = ROSSA_HASH("^");
const hash_ull Rossa::HASH_B_SH_L = ROSSA_HASH("<<");
const hash_ull Rossa::HASH_B_SH_R = ROSSA_HASH(">>");
const hash_ull Rossa::HASH_LESS = ROSSA_HASH("<");
const hash_ull Rossa::HASH_MORE = ROSSA_HASH(">");
const hash_ull Rossa::HASH_ELESS = ROSSA_HASH("<=");
const hash_ull Rossa::HASH_EMORE = ROSSA_HASH(">=");
const hash_ull Rossa::HASH_INDEX = ROSSA_HASH("[]");
const hash_ull Rossa::HASH_EQUALS = ROSSA_HASH("==");
const hash_ull Rossa::HASH_NEQUALS = ROSSA_HASH("!=");
const hash_ull Rossa::HASH_SET = ROSSA_HASH("=");
const hash_ull Rossa::HASH_CALL = ROSSA_HASH("()");
const hash_ull Rossa::HASH_RANGE = ROSSA_HASH("..");

Rossa::Rossa(std::vector<std::string> args)
{
	std::vector<Symbol> argv;
	for (auto &s : args)
		argv.push_back(Symbol(s));
	auto v = Symbol(argv);
	main.createVariable(ROSSA_HASH("_args"), v, NULL);
}

const std::map<std::string, signed int> Rossa::bOperators = {
	{"[]", 999},
	{"->", 14},
	{"**", -13},
	{"*", 12},
	{"/", 12},
	{"%", 12},
	{"+", 11},
	{"-", 11},
	{"<<", 10},
	{">>", 10},
	{"..", 0}, //undef
	{".+", 0}, //undef
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
	{"=", -1},
	{":=", -1},
	{"**=", -1},
	{"*=", -1},
	{"/=", -1},
	{"%=", -1},
	{"+=", -1},
	{"-=", -1},
	{"&=", -1},
	{"|=", -1},
	{"^=", -1},
	{":", 0},
	{"::", 0},
	{"=>", 0} };

const std::map<std::string, signed int> Rossa::uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1},
	{"$", -1} };

std::shared_ptr<Node> Rossa::compileCode(const std::string &code, boost::filesystem::path currentFile) const
{
	auto tokens = lexString(code, currentFile.filename().string());
	NodeParser testnp(tokens, currentFile);
	auto n = testnp.parse();
	return n->fold();
}

const Symbol Rossa::runCode(std::shared_ptr<Node> entry, bool tree)
{
	if (tree)
		std::cout << entry->printTree("", true).str();

	auto g = NodeParser::genParser(std::move(entry));

	std::vector<Function> stack_trace;
	return g->evaluate(&main, stack_trace);
}

void Rossa::printError(const RTError &e)
{
	std::string ret = "\033[" + std::to_string(RED_TEXT) + "m" + e.what() + "\n";

	if (e.getToken().type != NULL_TOK) {
		std::string lineInfoRaw = "<" + e.getToken().filename.string() + ">:" + std::to_string(e.getToken().lineNumber + 1) + " | ";
		ret += "\033[" + std::to_string(CYAN_TEXT) + "m<\033[4m" + e.getToken().filename.string() + "\033[0m\033[" + std::to_string(CYAN_TEXT) + "m>:" + std::to_string(e.getToken().lineNumber + 1) + " | ";
		ret += "\033[" + std::to_string(MAGENTA_TEXT) + "m" + e.getToken().line + "\n";

		ret += "\033[" + std::to_string(RED_TEXT) + "m";
		for (size_t i = 0; i < e.getToken().distance - e.getToken().valueString.size() + lineInfoRaw.size(); i++)
			ret += " ";
		ret += "^";

		if (e.getToken().valueString.size() > 0)
			for (size_t i = 0; i < e.getToken().valueString.size() - 1; i++)
				ret += "~";

		ret += "\033[0m";
	}

	std::cout << ret << "\n";
	size_t j = 0;
	auto trace = e.getTrace();
	while (!trace.empty()) {
		if (j++ > 10) {
			PRINTC((boost::format(_STACK_TRACE_MORE_) % trace.size()).str(), MAGENTA_TEXT);
			std::cout << "\n";
			trace.clear();
			break;
		}
		auto f = trace.back();
		PRINTC(" ^ ", MAGENTA_TEXT);
		std::cout << "\033[" << BRIGHT_BLACK_TEXT << "m";
		if (ROSSA_DEHASH(f.getParent()->getHashedKey()) != "")
			std::cout << ROSSA_DEHASH(f.getParent()->getHashedKey()) << ".";
		std::cout << ROSSA_DEHASH(f.getKey()) << "(\033[0m";
		size_t i = 0;
		for (auto &p : f.getParams()) {
			if (i++ > 0)
				std::cout << ", ";
			switch (p.first) {
				case TOK_REF:
					std::cout << "\033[" << MAGENTA_TEXT << "mref\033[0m ";
					break;
				default:
					break;
			}
			std::cout << ROSSA_DEHASH(p.second);
		}
		std::cout << "\033[" << BRIGHT_BLACK_TEXT << "m)\033[0m\n";
		trace.pop_back();
	}
}

const char Rossa::nextChar(
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

const char Rossa::peekChar(
	const size_t &i,
	const std::string &INPUT,
	const size_t &INPUT_INDEX)
{
	if (INPUT_INDEX + i < INPUT.size())
		return INPUT[INPUT_INDEX + i];
	return 0;
}

const int Rossa::getToken(
	const std::string &INPUT,
	size_t &INPUT_INDEX,
	size_t &LINE_INDEX,
	size_t &TOKEN_DIST,
	std::string &ID_STRING,
	RNumber &NUM_VALUE)
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
		else if (ID_STRING == KEYWORD_SIZE)
			return TOK_SIZE;
		else if (ID_STRING == KEYWORD_LENGTH)
			return TOK_LENGTH;
		else if (ID_STRING == KEYWORD_STRUCT)
			return TOK_STRUCT;
		else if (ID_STRING == KEYWORD_STATIC)
			return TOK_STATIC;
		else if (ID_STRING == KEYWORD_CLASS)
			return TOK_CLASS;
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
		else if (ID_STRING == "inf") {
			NUM_VALUE = RNumber::Double(INFINITY);
			return TOK_NUM;
		} else if (ID_STRING == "nan") {
			NUM_VALUE = RNumber::Double(NAN);
			return TOK_NUM;
		} else if (Rossa::bOperators.find(ID_STRING) != Rossa::bOperators.end() || Rossa::uOperators.find(ID_STRING) != Rossa::uOperators.end())
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
					NUM_VALUE = RNumber::Long(std::stoll(numStr, nullptr, 2));
					break;
				default:
					NUM_VALUE = RNumber::Long(std::stoll("0" + std::string(1, base) + numStr, nullptr, 0));
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
			NUM_VALUE = RNumber::Double(std::stold(numStr));
		else
			NUM_VALUE = RNumber::Long(std::stoll(numStr, nullptr, 10));
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
	else if (Rossa::bOperators.find(std::string(1, last)) != Rossa::bOperators.end()) {
		std::string opStr = std::string(1, last);
		while (Rossa::bOperators.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != Rossa::bOperators.end()) {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			opStr += last;
		}

		ID_STRING = opStr;
		if (ID_STRING == "=>")
			return TOK_DEF;
		if (ID_STRING == "::")
			return TOK_DEF_TYPE;
		if (ID_STRING == ".")
			return TOK_INNER;
		if (ID_STRING == "..")
			return TOK_UNTILF;
		if (ID_STRING == ".+")
			return TOK_UNTILT;
		if (ID_STRING == ":")
			return ':';
		return TOK_OPR;
	} else if (Rossa::uOperators.find(std::string(1, last)) != Rossa::uOperators.end()) {
		std::string opStr = std::string(1, last);
		while (Rossa::uOperators.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != Rossa::uOperators.end()) {
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
				NUM_VALUE = RNumber::Long(static_cast<unsigned char>(value[0]));
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

const std::vector<Token> Rossa::lexString(const std::string &INPUT, const boost::filesystem::path &filename)
{
	std::vector<std::string> LINES;
	std::stringstream ss(INPUT);
	std::string item;

	while (std::getline(ss, item, '\n')) {
		LINES.push_back(item);
	}

	std::vector<Token> tokens;
	size_t INPUT_INDEX = 0;
	size_t LINE_INDEX = 0;
	size_t TOKEN_DIST = 0;
	std::string ID_STRING;
	RNumber NUM_VALUE;

	while (true) {
		int token = getToken(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST, ID_STRING, NUM_VALUE);
		if (token == TOK_EOF)
			break;
		if (token == '#')
			continue;
		Token t = { filename, LINES[LINE_INDEX], LINE_INDEX, TOKEN_DIST, ID_STRING, NUM_VALUE, token };

		if (t.type == TOK_DEF) {
			std::vector<Token> temp;
			while (tokens.back().type != '(') {
				temp.push_back(tokens.back());
				tokens.pop_back();
			}
			temp.push_back(tokens.back());
			tokens.pop_back();
			if (!tokens.empty() && (tokens.back().type == TOK_IDF || tokens.back().type == '~' || tokens.back().type == TOK_SIZE || tokens.back().type == TOK_CHARN || tokens.back().type == TOK_CHARS || tokens.back().type == TOK_LENGTH || tokens.back().type == TOK_ALLOC || tokens.back().type == TOK_PARSE)) {
				temp.push_back(tokens.back());
				tokens.pop_back();
				if (tokens.back().type == TOK_DEF_TYPE) {
					temp.push_back(tokens.back());
					tokens.pop_back();
					temp.push_back(tokens.back());
					tokens.pop_back();
				}
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