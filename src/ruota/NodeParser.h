#ifndef NODEPARSER_H
#define NODEPARSER_H

#include "Ruota.h"
#include "Node.h"

class NodeParser
{
private:
	const std::vector<Token> tokens;
	const std::map<string, signed int> bOperators;
	const std::map<string, signed int> uOperators;
	const boost::filesystem::path currentFile;

	unsigned int index = 0;
	Token currentToken;
	void nextToken();
	std::unique_ptr<Node> parseNumNode();
	std::unique_ptr<Node> parseBoolNode();
	std::unique_ptr<Node> parseIDNode();
	std::unique_ptr<Node> parseBIDNode();
	std::unique_ptr<Node> parseEntryNode();
	std::unique_ptr<Node> parseExprNode();
	std::unique_ptr<Node> parseEquNode();
	std::unique_ptr<Node> parseVectorNode();
	std::unique_ptr<Node> parseUnitNode();
	std::unique_ptr<Node> parseBaseNode();
	std::unique_ptr<Node> parseUnOpNode();
	std::unique_ptr<Node> parseMapNode();
	std::unique_ptr<Node> parseIfElseNode();
	std::unique_ptr<Node> parseWhileNode();
	std::unique_ptr<Node> parseForNode();
	std::pair<Signature, std::vector<std::pair<LexerTokenType, hash_ull>>> parseSigNode(ValueType start);
	std::unique_ptr<Node> parseDefineNode();
	std::unique_ptr<Node> parseLambdaNode();
	std::unique_ptr<Node> parseExternNode();
	std::unique_ptr<Node> parseExternCallNode();
	std::unique_ptr<Node> parseCallBuiltNode();
	std::unique_ptr<Node> parseClassNode();
	std::unique_ptr<Node> parseNewNode();
	std::unique_ptr<Node> parseLoadNode();
	std::unique_ptr<Node> parseSwitchNode();
	std::unique_ptr<Node> parseTryCatchNode();
	std::unique_ptr<Node> parseTypeNode();
	std::unique_ptr<Node> parseTrailingNode(std::unique_ptr<Node>, bool);
	std::unique_ptr<Node> parseInsNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseUntilNode(std::unique_ptr<Node>, bool);
	std::unique_ptr<Node> parseCastToNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseBinOpNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCallNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseIndexNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseThenNode(std::unique_ptr<Node>);

	std::unique_ptr<Node> logErrorN(const string &, const Token);
	std::pair<Signature, std::vector<std::pair<LexerTokenType, hash_ull>>> logErrorSN(const string &, const Token);

public:
	NodeParser(const std::vector<Token> &, const std::map<string, signed int> &, const std::map<string, signed int> &, const boost::filesystem::path &);
	std::unique_ptr<Node> parse();
	static std::shared_ptr<Instruction> genParser(std::unique_ptr<Node>);
};

#endif