#ifndef PARSER_H
#define PARSER_H

#include <filesystem>

#include "../rossa.h"

#include "../object/object.h"
#include "../rossa_error/rossa_error.h"

class parser_t
{
private:
	std::vector<std::pair<std::vector<hash_ull>, symbol_t>> consts;
	std::vector<node_scope_t> scopes;
	static const int getToken(const std::string &, size_t &, size_t &, size_t &, std::string &, number_t &);
	static const char peekChar(const size_t &, const std::string &, const size_t &);
	static const char nextChar(const std::string &, size_t &, size_t &, size_t &);

public:
	object_t main;

	static const std::map<std::string, signed int> bOperators;
	static const std::map<std::string, signed int> uOperators;

	static Hash MAIN_HASH;

	static const hash_ull HASH_THIS;
	static const hash_ull HASH_BLANK;
	static const hash_ull HASH_INIT;
	static const hash_ull HASH_DELETER;

	static const hash_ull HASH_ADD;
	static const hash_ull HASH_SUB;
	static const hash_ull HASH_MUL;
	static const hash_ull HASH_DIV;
	static const hash_ull HASH_MOD;
	static const hash_ull HASH_POW;
	static const hash_ull HASH_B_AND;
	static const hash_ull HASH_B_OR;
	static const hash_ull HASH_B_XOR;
	static const hash_ull HASH_B_SH_L;
	static const hash_ull HASH_B_SH_R;
	static const hash_ull HASH_B_NOT;
	static const hash_ull HASH_LESS;
	static const hash_ull HASH_MORE;
	static const hash_ull HASH_ELESS;
	static const hash_ull HASH_EMORE;
	static const hash_ull HASH_INDEX;
	static const hash_ull HASH_EQUALS;
	static const hash_ull HASH_NEQUALS;
	static const hash_ull HASH_SET;
	static const hash_ull HASH_CALL;
	static const hash_ull HASH_RANGE;
	static const hash_ull HASH_VAR_ARGS;
	static const hash_ull HASH_LENGTH;
	static const hash_ull HASH_NOT;
	static const hash_ull HASH_CCT;
	static const hash_ull HASH_DEL;
	static const hash_ull HASH_HASH;

	parser_t(const std::vector<std::string> &);
	static void loadStandardFunctions(std::map<std::string, extf_t> &fmap);
	const ptr_node_t compileCode(const std::string &, const std::filesystem::path &);
	const symbol_t runCode(const ptr_node_t &, const bool &);
	static void printError(const rossa_error_t &);
	static const std::vector<token_t> lexString(const std::string &, const std::filesystem::path &);

	~parser_t();
};

#endif