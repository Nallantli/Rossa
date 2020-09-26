#include "Ruota.h"
#include "Node.h"

std::vector<boost::filesystem::path> rdir::loaded = {};
std::map<std::string, boost::function<Symbol(std::vector<Symbol>)>> rlib::loaded = {};

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
hashcode_t Ruota::HASH_LESS = hash.hashString("<");
hashcode_t Ruota::HASH_MORE = hash.hashString(">");
hashcode_t Ruota::HASH_ELESS = hash.hashString("<=");
hashcode_t Ruota::HASH_EMORE = hash.hashString(">=");
hashcode_t Ruota::HASH_INDEX = hash.hashString("[]");

Ruota::Ruota() {}

//std::unique_ptr<DataManager> Ruota::manager = std::make_unique<DataManager>();

const std::map<std::string, signed int> Ruota::bOperators = {
	{"**", 13},
	{"*", 12},
	{"/", 12},
	{"%", 12},
	{"+", 11},
	{"-", 11},
	{">", 8},
	{"<", 8},
	{">=", 8},
	{"<=", 8},
	{"==", 7},
	{"!=", 7},
	{"!", -1}, //undef
	{"&", 6},  //undef
	{"|", 4},  //undef
	{"&&", 3},
	{"||", 2},
	{"=", 1},
	{"->", 1},
	{"**=", 1},
	{"*=", 1},
	{"/=", 1},
	{"%=", 1},
	{"+=", 1},
	{"-=", 1},
	{":", -1},
	{"::", -1},
	{"=>", -1}};

const std::map<std::string, signed int> Ruota::uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1}};

Lexer Ruota::lexer = Lexer(bOperators, uOperators);

Symbol Ruota::parseCode(const std::string &code, boost::filesystem::path currentFile)
{
	auto tokens = lexer.lexString(code, currentFile.filename().string());
	NodeParser testnp(tokens, bOperators, uOperators, currentFile);
	auto n = testnp.parse();
	//n->printTree("", true);
	auto folded = n->fold();
	//folded->printTree("", true);

	auto g = NodeParser::genParser(std::move(folded));
	auto res = g->evaluate(main);
	delete g;
	return res;
}