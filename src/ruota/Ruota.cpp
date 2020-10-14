#include "Ruota.h"
#include "Library.h"
#include "NodeParser.h"

std::vector<boost::filesystem::path> rdir::loaded = {};
std::map<string, boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)>> rlib::loaded = {};

std::vector<Function> Ruota::stack_trace = {};

Hash MAIN_HASH = Hash();

hash_ull Ruota::HASH_INIT = MAIN_HASH.hashString("init");
hash_ull Ruota::HASH_THIS = MAIN_HASH.hashString("this");
hash_ull Ruota::HASH_KEY = MAIN_HASH.hashString("key");
hash_ull Ruota::HASH_VALUE = MAIN_HASH.hashString("value");
hash_ull Ruota::HASH_DELETER = MAIN_HASH.hashString("~");

hash_ull Ruota::HASH_ADD = MAIN_HASH.hashString("+");
hash_ull Ruota::HASH_SUB = MAIN_HASH.hashString("-");
hash_ull Ruota::HASH_MUL = MAIN_HASH.hashString("*");
hash_ull Ruota::HASH_DIV = MAIN_HASH.hashString("/");
hash_ull Ruota::HASH_MOD = MAIN_HASH.hashString("%");
hash_ull Ruota::HASH_POW = MAIN_HASH.hashString("**");
hash_ull Ruota::HASH_B_AND = MAIN_HASH.hashString("&");
hash_ull Ruota::HASH_B_OR = MAIN_HASH.hashString("|");
hash_ull Ruota::HASH_B_XOR = MAIN_HASH.hashString("^");
hash_ull Ruota::HASH_B_SH_L = MAIN_HASH.hashString("<<");
hash_ull Ruota::HASH_B_SH_R = MAIN_HASH.hashString(">>");
hash_ull Ruota::HASH_LESS = MAIN_HASH.hashString("<");
hash_ull Ruota::HASH_MORE = MAIN_HASH.hashString(">");
hash_ull Ruota::HASH_ELESS = MAIN_HASH.hashString("<=");
hash_ull Ruota::HASH_EMORE = MAIN_HASH.hashString(">=");
hash_ull Ruota::HASH_INDEX = MAIN_HASH.hashString("[]");
hash_ull Ruota::HASH_EQUALS = MAIN_HASH.hashString("==");
hash_ull Ruota::HASH_NEQUALS = MAIN_HASH.hashString("!=");
hash_ull Ruota::HASH_SET = MAIN_HASH.hashString("=");
hash_ull Ruota::HASH_CALL = MAIN_HASH.hashString("()");
hash_ull Ruota::HASH_RANGE = MAIN_HASH.hashString("..");

hash_ull Ruota::HASH_TO_STRING = MAIN_HASH.hashString("->String");
hash_ull Ruota::HASH_TO_NUMBER = MAIN_HASH.hashString("->Number");
hash_ull Ruota::HASH_TO_BOOLEAN = MAIN_HASH.hashString("->Boolean");
hash_ull Ruota::HASH_TO_VECTOR = MAIN_HASH.hashString("->Array");
hash_ull Ruota::HASH_TO_DICTIONARY = MAIN_HASH.hashString("->Dictionary");

Ruota::Ruota(std::vector<string> args)
{
	std::vector<Symbol> argv;
	for (auto &s : args)
		argv.push_back(Symbol(s));
	auto v = Symbol(argv);
	main.createVariable(MAIN_HASH.hashString("_args"), v, NULL);
}

const std::map<string, signed int> Ruota::bOperators = {
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

const std::map<string, signed int> Ruota::uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1} };

Lexer Ruota::lexer = Lexer(bOperators, uOperators);

std::shared_ptr<Node> Ruota::compileCode(const string &code, boost::filesystem::path currentFile) const
{
	auto tokens = lexer.lexString(code, currentFile.filename().string());
	NodeParser testnp(tokens, bOperators, uOperators, currentFile);
	auto n = testnp.parse();
	return n->fold();
}

const Symbol Ruota::runCode(std::shared_ptr<Node> entry, bool tree)
{
	if (tree)
		entry->printTree("", true);

	auto g = NodeParser::genParser(std::move(entry));

	return g->evaluate(&main);
}