#include "Ruota.h"
#include "Lexer.h"
#include "Library.h"
#include "NodeParser.h"

std::vector<boost::filesystem::path> rdir::loaded = {};
std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, Token*)>> rlib::loaded = {};

std::vector<Function> Ruota::stack_trace = {};

Hash hash = Hash();

hashcode_t Ruota::HASH_INIT = hash.hashString("init");
hashcode_t Ruota::HASH_THIS = hash.hashString("this");
hashcode_t Ruota::HASH_KEY = hash.hashString("key");
hashcode_t Ruota::HASH_VALUE = hash.hashString("value");
hashcode_t Ruota::HASH_DELETER = hash.hashString("~");

hashcode_t Ruota::HASH_ADD = hash.hashString("+");
hashcode_t Ruota::HASH_SUB = hash.hashString("-");
hashcode_t Ruota::HASH_MUL = hash.hashString("*");
hashcode_t Ruota::HASH_DIV = hash.hashString("/");
hashcode_t Ruota::HASH_MOD = hash.hashString("%");
hashcode_t Ruota::HASH_POW = hash.hashString("**");
hashcode_t Ruota::HASH_B_AND = hash.hashString("&");
hashcode_t Ruota::HASH_B_OR = hash.hashString("|");
hashcode_t Ruota::HASH_B_XOR = hash.hashString("^");
hashcode_t Ruota::HASH_B_SH_L = hash.hashString("<<");
hashcode_t Ruota::HASH_B_SH_R = hash.hashString(">>");
hashcode_t Ruota::HASH_LESS = hash.hashString("<");
hashcode_t Ruota::HASH_MORE = hash.hashString(">");
hashcode_t Ruota::HASH_ELESS = hash.hashString("<=");
hashcode_t Ruota::HASH_EMORE = hash.hashString(">=");
hashcode_t Ruota::HASH_INDEX = hash.hashString("[]");
hashcode_t Ruota::HASH_EQUALS = hash.hashString("==");
hashcode_t Ruota::HASH_NEQUALS = hash.hashString("!=");
hashcode_t Ruota::HASH_SET = hash.hashString("=");
hashcode_t Ruota::HASH_CALL = hash.hashString("()");

hashcode_t Ruota::HASH_TO_STRING = hash.hashString("->String");
hashcode_t Ruota::HASH_TO_NUMBER = hash.hashString("->Number");
hashcode_t Ruota::HASH_TO_BOOLEAN = hash.hashString("->Boolean");
hashcode_t Ruota::HASH_TO_VECTOR = hash.hashString("->Vector");
hashcode_t Ruota::HASH_TO_DICTIONARY = hash.hashString("->Dictionary");

Ruota::Ruota() {}

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
	{">", 8},
	{"<", 8},
	{">=", 8},
	{"<=", 8},
	{"===", 7},
	{"==", 7},
	{"!==", 7},
	{"!=", 7},
	{"!", 0}, //undef
	{"&", 6},
	{"^", 5},
	{"|", 4},
	{"&&", 3},
	{"||", 2},
	{"=", -1},
	{"->", 0},
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

Symbol Ruota::parseCode(const std::string &code, boost::filesystem::path currentFile, bool tree)
{
	auto tokens = lexer.lexString(code, currentFile.filename().string());
	NodeParser testnp(tokens, bOperators, uOperators, currentFile);
	auto n = testnp.parse();

	auto folded = n->fold();
	if (tree)
		folded->printTree("", true);

	auto g = NodeParser::genParser(std::move(folded));
	auto res = g->evaluate(main);
	delete g;
	return res;
}