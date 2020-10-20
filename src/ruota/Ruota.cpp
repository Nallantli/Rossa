#include "Ruota.h"
#include "Library.h"
#include "NodeParser.h"

#include <iostream>

using namespace ruota;

std::vector<boost::filesystem::path> dir::loaded = {};
std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)>> lib::loaded = {};

Hash _MAIN_HASH_ = Hash();

const hash_ull Ruota::HASH_INIT = RUOTA_HASH("init");
const hash_ull Ruota::HASH_IDEM = RUOTA_HASH("idem");
const hash_ull Ruota::HASH_THIS = RUOTA_HASH("this");
const hash_ull Ruota::HASH_KEY = RUOTA_HASH("key");
const hash_ull Ruota::HASH_VALUE = RUOTA_HASH("value");
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

Lexer Ruota::lexer = Lexer();

std::shared_ptr<Node> Ruota::compileCode(const std::string &code, boost::filesystem::path currentFile) const
{
	auto tokens = lexer.lexString(code, currentFile.filename().string());
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