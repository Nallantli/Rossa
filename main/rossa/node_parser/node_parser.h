#ifndef NODE_PARSER_H
#define NODE_PARSER_H

#include "../rossa.h"

class node_parser_t
{
private:
	hash_ull scope_i = 0;
	std::vector<std::pair<std::vector<hash_ull>, symbol_t>> *consts;

	const std::vector<token_t> tokens;
	const std::filesystem::path currentFile;

	unsigned int index = 0;
	token_t currentToken;
	void nextToken();
	ptr_node_t parseNumNode(std::vector<node_scope_t> *);
	ptr_node_t parseBoolNode(std::vector<node_scope_t> *);
	ptr_node_t parseIDNode(std::vector<node_scope_t> *);
	ptr_node_t parseBIDNode(std::vector<node_scope_t> *);
	ptr_node_t parseEntryNode(std::vector<node_scope_t> *);
	ptr_node_t parseExprNode(std::vector<node_scope_t> *);
	ptr_node_t parseEquNode(std::vector<node_scope_t> *);
	ptr_node_t parseVectorNode(std::vector<node_scope_t> *);
	ptr_node_t parseUnitNode(std::vector<node_scope_t> *);
	ptr_node_t parseBaseNode(std::vector<node_scope_t> *);
	ptr_node_t parseUnOpNode(std::vector<node_scope_t> *);
	ptr_node_t parseMapNode(std::vector<node_scope_t> *);
	ptr_node_t parseIfElseNode(std::vector<node_scope_t> *);
	ptr_node_t parseWhileNode(std::vector<node_scope_t> *);
	ptr_node_t parseForNode(std::vector<node_scope_t> *);
	ptr_node_t parseEachNode(std::vector<node_scope_t> *);
	std::pair<signature_t, std::vector<std::pair<token_type_enum, hash_ull>>> parseSigNode(std::vector<node_scope_t> *);
	ptr_node_t parseDefineNode(std::vector<node_scope_t> *);
	ptr_node_t parseLambdaNode(std::vector<node_scope_t> *);
	ptr_node_t parseNPLambdaNode(std::vector<node_scope_t> *);
	ptr_node_t parseExternNode(std::vector<node_scope_t> *);
	ptr_node_t parseExternCallNode(std::vector<node_scope_t> *);
	ptr_node_t parseCallOpNode(std::vector<node_scope_t> *);
	ptr_node_t parseCallBuiltNode(std::vector<node_scope_t> *);
	ptr_node_t parseClassNode(std::vector<node_scope_t> *);
	ptr_node_t parseNewNode(std::vector<node_scope_t> *);
	ptr_node_t parseLoadNode(std::vector<node_scope_t> *);
	ptr_node_t parseSwitchNode(std::vector<node_scope_t> *);
	ptr_node_t parseTryCatchNode(std::vector<node_scope_t> *);
	ptr_node_t parseTypeNode(std::vector<node_scope_t> *);
	parameter_t parseParamTypeNode(std::vector<node_scope_t> *, const aug_type_t &);
	ptr_node_t parseTrailingNode(std::vector<node_scope_t> *, const ptr_node_t &, const bool &);
	ptr_node_t parseInsNode(std::vector<node_scope_t> *, const ptr_node_t &);
	ptr_node_t parseUntilNode(std::vector<node_scope_t> *, const ptr_node_t &, const bool &);
	ptr_node_t parseBinOpNode(std::vector<node_scope_t> *, const ptr_node_t &);
	ptr_node_t parseCallNode(std::vector<node_scope_t> *, const ptr_node_t &);
	ptr_node_t parseIndexNode(std::vector<node_scope_t> *, const ptr_node_t &);
	ptr_node_t parseThenNode(std::vector<node_scope_t> *, const ptr_node_t &);

	ptr_node_t logErrorN(const std::string &, const token_t &);
	parameter_t logErrorPT(const std::string &, const token_t &);
	std::pair<signature_t, std::vector<std::pair<token_type_enum, hash_ull>>> logErrorSN(const std::string &, const token_t &);

public:
	node_parser_t(const std::vector<token_t> &, const std::filesystem::path &);
	ptr_node_t parse(std::vector<node_scope_t> *, std::vector<std::pair<std::vector<hash_ull>, symbol_t>> *);
	static ptr_instruction_t genParser(const ptr_node_t &);
};

#endif