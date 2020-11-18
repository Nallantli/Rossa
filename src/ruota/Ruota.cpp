#include "Ruota.h"

using namespace ruota;

std::vector<boost::filesystem::path> dir::loaded = {};
std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)>> lib::loaded = {};

Hash Ruota::MAIN_HASH = Hash();

const hash_ull Ruota::HASH_INIT = RUOTA_HASH("init");
const hash_ull Ruota::HASH_THIS = RUOTA_HASH("this");
const hash_ull Ruota::HASH_DELETER = RUOTA_HASH("~");

const hash_ull Ruota::HASH_ADD = RUOTA_HASH("+");
const hash_ull Ruota::HASH_SUB = RUOTA_HASH("-");
const hash_ull Ruota::HASH_MUL = RUOTA_HASH("*");
const hash_ull Ruota::HASH_DIV = RUOTA_HASH("/");
const hash_ull Ruota::HASH_MOD = RUOTA_HASH("%");
const hash_ull Ruota::HASH_POW = RUOTA_HASH("**");
const hash_ull Ruota::HASH_B_AND = RUOTA_HASH("&");
const hash_ull Ruota::HASH_B_OR = RUOTA_HASH("|");
const hash_ull Ruota::HASH_B_XOR = RUOTA_HASH("^");
const hash_ull Ruota::HASH_B_SH_L = RUOTA_HASH("<<");
const hash_ull Ruota::HASH_B_SH_R = RUOTA_HASH(">>");
const hash_ull Ruota::HASH_LESS = RUOTA_HASH("<");
const hash_ull Ruota::HASH_MORE = RUOTA_HASH(">");
const hash_ull Ruota::HASH_ELESS = RUOTA_HASH("<=");
const hash_ull Ruota::HASH_EMORE = RUOTA_HASH(">=");
const hash_ull Ruota::HASH_INDEX = RUOTA_HASH("[]");
const hash_ull Ruota::HASH_EQUALS = RUOTA_HASH("==");
const hash_ull Ruota::HASH_NEQUALS = RUOTA_HASH("!=");
const hash_ull Ruota::HASH_SET = RUOTA_HASH("=");
const hash_ull Ruota::HASH_CALL = RUOTA_HASH("()");
const hash_ull Ruota::HASH_RANGE = RUOTA_HASH("..");

const hash_ull Ruota::HASH_TO_STRING = RUOTA_HASH("->String");
const hash_ull Ruota::HASH_TO_NUMBER = RUOTA_HASH("->Number");
const hash_ull Ruota::HASH_TO_BOOLEAN = RUOTA_HASH("->Boolean");
const hash_ull Ruota::HASH_TO_VECTOR = RUOTA_HASH("->Array");
const hash_ull Ruota::HASH_TO_DICTIONARY = RUOTA_HASH("->Dictionary");

Ruota::Ruota(std::vector<std::string> args)
{
	std::vector<Symbol> argv;
	for (auto &s : args)
		argv.push_back(Symbol(s));
	auto v = Symbol(argv);
	main.createVariable(RUOTA_HASH("_args"), v, NULL);
}

const std::map<std::string, signed int> Ruota::bOperators = {
	{"[]", 999},
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
	{"->", 0},
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

const std::map<std::string, signed int> Ruota::uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1} };

std::shared_ptr<Node> Ruota::compileCode(const std::string &code, boost::filesystem::path currentFile) const
{
	auto tokens = lexString(code, currentFile.filename().string());
	NodeParser testnp(tokens, currentFile);
	auto n = testnp.parse();
	return n->fold();
}

const Symbol Ruota::runCode(std::shared_ptr<Node> entry, bool tree)
{
	if (tree)
		std::cout << entry->printTree("", true).str();

	auto g = NodeParser::genParser(std::move(entry));

	std::vector<Function> stack_trace;
	return g->evaluate(&main, stack_trace);
}

void Ruota::printError(const RTError &e)
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
		if (RUOTA_DEHASH(f.getParent()->getHashedKey()) != "")
			std::cout << RUOTA_DEHASH(f.getParent()->getHashedKey()) << ".";
		std::cout << RUOTA_DEHASH(f.getKey()) << "(\033[0m";
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
			std::cout << RUOTA_DEHASH(p.second);
		}
		std::cout << "\033[" << BRIGHT_BLACK_TEXT << "m)\033[0m\n";
		trace.pop_back();
	}
}

const char Ruota::nextChar(
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

const char Ruota::peekChar(
	const size_t &i,
	const std::string &INPUT,
	const size_t &INPUT_INDEX)
{
	if (INPUT_INDEX + i < INPUT.size())
		return INPUT[INPUT_INDEX + i];
	return 0;
}

const int Ruota::getToken(
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

		if (ID_STRING == "then")
			return TOK_THEN;
		else if (ID_STRING == "else")
			return TOK_ELSE;
		else if (ID_STRING == "do")
			return TOK_DO;
		else if (ID_STRING == "in")
			return TOK_IN;
		else if (ID_STRING == "var")
			return TOK_VAR;
		else if (ID_STRING == "if")
			return TOK_IF;
		else if (ID_STRING == "while")
			return TOK_WHILE;
		else if (ID_STRING == "var")
			return TOK_VAR;
		else if (ID_STRING == "elif")
			return TOK_ELSEIF;
		else if (ID_STRING == "for")
			return TOK_FOR;
		else if (ID_STRING == "true")
			return TOK_TRUE;
		else if (ID_STRING == "false")
			return TOK_FALSE;
		else if (ID_STRING == "return")
			return TOK_RETURN;
		else if (ID_STRING == "nil")
			return TOK_NIL;
		else if (ID_STRING == "Number")
			return TOK_NUMBER;
		else if (ID_STRING == "String")
			return TOK_STRING;
		else if (ID_STRING == "Array")
			return TOK_ARRAY;
		else if (ID_STRING == "Boolean")
			return TOK_BOOLEAN;
		else if (ID_STRING == "Dictionary")
			return TOK_DICTIONARY;
		else if (ID_STRING == "Object")
			return TOK_OBJECT;
		else if (ID_STRING == "Function")
			return TOK_FUNCTION;
		else if (ID_STRING == "Type")
			return TOK_TYPE_NAME;
		else if (ID_STRING == "extern")
			return TOK_EXTERN;
		else if (ID_STRING == "extern_call")
			return TOK_EXTERN_CALL;
		else if (ID_STRING == "size")
			return TOK_SIZE;
		else if (ID_STRING == "length")
			return TOK_LENGTH;
		else if (ID_STRING == "struct")
			return TOK_STRUCT;
		else if (ID_STRING == "static")
			return TOK_STATIC;
		else if (ID_STRING == "class")
			return TOK_CLASS;
		else if (ID_STRING == "new")
			return TOK_NEW;
		else if (ID_STRING == "load")
			return TOK_LOAD;
		else if (ID_STRING == "alloc")
			return TOK_ALLOC;
		else if (ID_STRING == "ref")
			return TOK_REF;
		else if (ID_STRING == "break")
			return TOK_BREAK;
		else if (ID_STRING == "refer")
			return TOK_REFER;
		else if (ID_STRING == "Nil")
			return TOK_NIL_NAME;
		else if (ID_STRING == "Pointer")
			return TOK_POINTER;
		else if (ID_STRING == "virtual")
			return TOK_VIRTUAL;
		else if (ID_STRING == "switch")
			return TOK_SWITCH;
		else if (ID_STRING == "try")
			return TOK_TRY;
		else if (ID_STRING == "catch")
			return TOK_CATCH;
		else if (ID_STRING == "throw")
			return TOK_THROW;
		else if (ID_STRING == "charn")
			return TOK_CHARN;
		else if (ID_STRING == "chars")
			return TOK_CHARS;
		else if (ID_STRING == "case")
			return TOK_CASE;
		else if (ID_STRING == "parse")
			return TOK_PARSE;
		else if (ID_STRING == "continue")
			return TOK_CONTINUE;
		else if (ID_STRING == "inf") {
			NUM_VALUE = RNumber::Double(INFINITY);
			return TOK_NUM;
		} else if (ID_STRING == "nan") {
			NUM_VALUE = RNumber::Double(NAN);
			return TOK_NUM;
		} else if (Ruota::bOperators.find(ID_STRING) != Ruota::bOperators.end() || Ruota::uOperators.find(ID_STRING) != Ruota::uOperators.end())
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
	else if (Ruota::bOperators.find(std::string(1, last)) != Ruota::bOperators.end()) {
		std::string opStr = std::string(1, last);
		while (Ruota::bOperators.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != Ruota::bOperators.end()) {
			last = nextChar(INPUT, INPUT_INDEX, LINE_INDEX, TOKEN_DIST);
			opStr += last;
		}

		ID_STRING = opStr;
		if (ID_STRING == "->")
			return TOK_CAST;
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
	} else if (Ruota::uOperators.find(std::string(1, last)) != Ruota::uOperators.end()) {
		std::string opStr = std::string(1, last);
		while (Ruota::uOperators.find(opStr + peekChar(0, INPUT, INPUT_INDEX)) != Ruota::uOperators.end()) {
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

const std::vector<Token> Ruota::lexString(const std::string &INPUT, const boost::filesystem::path &filename)
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