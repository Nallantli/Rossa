#include "parser.h"
#include <fstream>

#include "../symbol/symbol.h"
#include "../node/node.h"
#include "../function/function.h"
#include "../instruction/instruction.h"
#include "../tokenizer/tokenizer.h" 

#include "../util/util.h"

#ifdef DEBUG
long long parser_t::symbol_count = 0;
long long parser_t::object_count = 0;
#endif

Hash parser_t::MAIN_HASH = Hash();

const hash_ull parser_t::HASH_INIT = ROSSA_HASH(KEYWORD_INIT);
const hash_ull parser_t::HASH_BLANK = ROSSA_HASH("");
const hash_ull parser_t::HASH_THIS = ROSSA_HASH(KEYWORD_THIS);
const hash_ull parser_t::HASH_DELETER = ROSSA_HASH(KEYWORD_DELETE_FUNC);

const hash_ull parser_t::HASH_ADD = ROSSA_HASH("+");
const hash_ull parser_t::HASH_SUB = ROSSA_HASH("-");
const hash_ull parser_t::HASH_MUL = ROSSA_HASH("*");
const hash_ull parser_t::HASH_DIV = ROSSA_HASH("/");
const hash_ull parser_t::HASH_FDIV = ROSSA_HASH("//");
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
const hash_ull parser_t::HASH_RANGE_INC = ROSSA_HASH("<>");
const hash_ull parser_t::HASH_RANGE_EXC = ROSSA_HASH("..");
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
	{
		argv.push_back(symbol_t::String(s));
	}
	scopes.push_back({ROSSA_HASH("<*>")});
	consts.push_back({{ROSSA_HASH("<*>"), ROSSA_HASH("__args__")}, symbol_t::Array(argv)});
}

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
	if (tree)
	{
		entry->printTree("", true);
		for (auto &c : consts)
		{
			int i = 0;
			for (auto &p : c.first)
			{
				if (i++ > 0)
				{
					printc(".", CYAN_TEXT);
				}
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

	if (e.getToken().type != NULL_TOK)
	{
		std::string lineInfoRaw = "<" + e.getToken().filename.string() + ">:" + std::to_string(e.getToken().lineNumber + 1) + " | ";
		printc(lineInfoRaw, CYAN_TEXT);
		printc(e.getToken().line + "\n", MAGENTA_TEXT);

		std::string ret = "";
		for (size_t i = 0; i < e.getToken().distance - e.getToken().valueString.size() + lineInfoRaw.size(); i++)
		{
			ret += " ";
		}
		ret += "^";

		if (e.getToken().valueString.size() > 0)
		{
			for (size_t i = 0; i < e.getToken().valueString.size() - 1; i++)
			{
				ret += "~";
			}
		}
		printc(ret + "\n", RED_TEXT);
	}

	size_t j = 0;
	auto trace = e.getTrace();
	while (!trace.empty())
	{
		if (j++ > 10)
		{
			printc(util::format(_STACK_TRACE_MORE_, {std::to_string(trace.size())}), MAGENTA_TEXT);
			std::cout << "\n";
			trace.clear();
			break;
		}
		auto e = trace.back();
		printc(" ^ ", BLUE_TEXT);
		std::string ret = "";
		if (e.second.getParent().getKey() != "")
		{
			ret += e.second.getParent().getKey() + ".";
		}
		ret += ROSSA_DEHASH(e.second.key);
		printc(ret + "(", BRIGHT_BLACK_TEXT);
		size_t i = 0;
		for (auto &p : e.second.params)
		{
			if (i++ > 0)
			{
				printc(", ", RESET_TEXT);
			}
			if (p.first) {
				printc("ref ", BLUE_TEXT);
			}
			printc(ROSSA_DEHASH(p.second), RESET_TEXT);
		}
		printc(")\n", BRIGHT_BLACK_TEXT);
		printc("\t<" + e.first.filename.string() + ">:" + std::to_string(e.first.lineNumber) + " | ", CYAN_TEXT);
		printc(e.first.line + "\n", MAGENTA_TEXT);
		trace.pop_back();
	}
}

parser_t::~parser_t()
{
}