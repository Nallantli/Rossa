#include "Ruota.hpp"
#include "Lexer.hpp"
#include "Node.hpp"

Ruota::Ruota() {}

std::unique_ptr<DataManager> Ruota::manager = std::make_unique<DataManager>();

const std::map<std::string, signed int> Ruota::bOperators = {
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
	{"!", 999}, //undef
	{"&", 6},	//undef
	{"|", 4},	//undef
	{"&&", 3},
	{"||", 2},
	{"=", 1},
	{"*=", 1},
	{"/=", 1},
	{"%=", 1},
	{"+=", 1},
	{"-=", 1},
	{"until", 2}};

const std::map<std::string, signed int> Ruota::uOperators = {
	{"-", 5},
	{"+", 5},
	{"!", 5}};

Lexer Ruota::lexer = Lexer(bOperators, uOperators);

DatumID Ruota::parseCode(const std::string &code)
{
	auto tokens = lexer.lexString(code);
	NodeParser testnp(tokens, bOperators, uOperators);
	auto n = testnp.parse();
	if (n)
	{
		auto g = NodeParser::genParser(std::move(n));
		std::cout << g->toString() << "\n";
		auto res = g->evaluate(main);
		std::cout << manager->toString(res) << "\n";
		delete g;

		return res;
	}
	return DatumID();
}