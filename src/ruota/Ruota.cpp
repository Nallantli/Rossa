#include "Ruota.h"
#include "Lexer.h"
#include "Node.h"

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
	{"::", -1}};

const std::map<std::string, signed int> Ruota::uOperators = {
	{"-", -1},
	{"+", -1},
	{"!", -1}};

Lexer Ruota::lexer = Lexer(bOperators, uOperators);

Symbol Ruota::parseCode(const std::string &code)
{
	auto tokens = lexer.lexString(code);
	NodeParser testnp(tokens, bOperators, uOperators, boost::filesystem::current_path());
	auto n = testnp.parse();
	if (n)
	{
		auto g = NodeParser::genParser(std::move(n));
		auto res = g->evaluate(main);
		delete g;
		return res;
	}
	return Symbol();
}