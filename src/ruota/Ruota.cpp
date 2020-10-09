#include "Ruota.h"
#include "Library.h"
#include "NodeParser.h"

std::vector<boost::filesystem::path> rdir::loaded = {};
std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)>> rlib::loaded = {};

std::vector<Function> Ruota::stack_trace = {};

Hash MAIN_HASH = Hash();

hashcode_t Ruota::HASH_INIT = MAIN_HASH.hashString("init");
hashcode_t Ruota::HASH_THIS = MAIN_HASH.hashString("this");
hashcode_t Ruota::HASH_KEY = MAIN_HASH.hashString("key");
hashcode_t Ruota::HASH_VALUE = MAIN_HASH.hashString("value");
hashcode_t Ruota::HASH_DELETER = MAIN_HASH.hashString("~");

hashcode_t Ruota::HASH_ADD = MAIN_HASH.hashString("+");
hashcode_t Ruota::HASH_SUB = MAIN_HASH.hashString("-");
hashcode_t Ruota::HASH_MUL = MAIN_HASH.hashString("*");
hashcode_t Ruota::HASH_DIV = MAIN_HASH.hashString("/");
hashcode_t Ruota::HASH_MOD = MAIN_HASH.hashString("%");
hashcode_t Ruota::HASH_POW = MAIN_HASH.hashString("**");
hashcode_t Ruota::HASH_B_AND = MAIN_HASH.hashString("&");
hashcode_t Ruota::HASH_B_OR = MAIN_HASH.hashString("|");
hashcode_t Ruota::HASH_B_XOR = MAIN_HASH.hashString("^");
hashcode_t Ruota::HASH_B_SH_L = MAIN_HASH.hashString("<<");
hashcode_t Ruota::HASH_B_SH_R = MAIN_HASH.hashString(">>");
hashcode_t Ruota::HASH_LESS = MAIN_HASH.hashString("<");
hashcode_t Ruota::HASH_MORE = MAIN_HASH.hashString(">");
hashcode_t Ruota::HASH_ELESS = MAIN_HASH.hashString("<=");
hashcode_t Ruota::HASH_EMORE = MAIN_HASH.hashString(">=");
hashcode_t Ruota::HASH_INDEX = MAIN_HASH.hashString("[]");
hashcode_t Ruota::HASH_EQUALS = MAIN_HASH.hashString("==");
hashcode_t Ruota::HASH_NEQUALS = MAIN_HASH.hashString("!=");
hashcode_t Ruota::HASH_SET = MAIN_HASH.hashString("=");
hashcode_t Ruota::HASH_CALL = MAIN_HASH.hashString("()");

hashcode_t Ruota::HASH_TO_STRING = MAIN_HASH.hashString("->String");
hashcode_t Ruota::HASH_TO_NUMBER = MAIN_HASH.hashString("->Number");
hashcode_t Ruota::HASH_TO_BOOLEAN = MAIN_HASH.hashString("->Boolean");
hashcode_t Ruota::HASH_TO_VECTOR = MAIN_HASH.hashString("->Vector");
hashcode_t Ruota::HASH_TO_DICTIONARY = MAIN_HASH.hashString("->Dictionary");

Ruota::Ruota(std::vector<std::string> args)
{
	std::vector<Symbol> argv;
	for (auto &s : args)
		argv.push_back(Symbol(s));
	auto v = Symbol(argv);
	main.createVariable(MAIN_HASH.hashString("_args"), v, NULL);
}

//std::unique_ptr<DataManager> Ruota::manager = std::make_unique<DataManager>();

const std::map<std::string, signed int> Ruota::bOperators = {
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
	{"=>", 0}};

const std::map<std::string, signed int> Ruota::uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1}};

Lexer Ruota::lexer = Lexer(bOperators, uOperators);

std::unique_ptr<Node> Ruota::compileCode(const std::string &code, boost::filesystem::path currentFile) const
{
	auto tokens = lexer.lexString(code, currentFile.filename().string());
	NodeParser testnp(tokens, bOperators, uOperators, currentFile);
	auto n = testnp.parse();
	return n->fold();
}

const Symbol Ruota::runCode(std::unique_ptr<Node> entry, bool tree)
{
	if (tree)
		entry->printTree("", true);

	auto g = NodeParser::genParser(std::move(entry));

	return g->evaluate(&main);
}