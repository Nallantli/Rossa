#ifndef NODEPARSER_H
#define NODEPARSER_H

#include "Ruota.h"
#include "Node.h"

class NodeParser
{
private:
	boost::filesystem::path currentFile;
	std::map<std::string, signed int> bOperators;
	std::map<std::string, signed int> uOperators;
	unsigned int index = 0;
	Token *currentToken;
	std::vector<Token> tokens;
	Token *nextToken();
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
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> parseSigNode();
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
	std::unique_ptr<Node> parseUntilNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCastToNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseBinOpNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCallNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseIndexNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseThenNode(std::unique_ptr<Node>);

	std::unique_ptr<Node> logErrorN(const std::string &, Token *);
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> logErrorSN(const std::string &, Token *);

public:
	NodeParser(std::vector<Token>, std::map<std::string, signed int>, std::map<std::string, signed int>, boost::filesystem::path);
	std::unique_ptr<Node> parse();
	static Instruction *genParser(std::unique_ptr<Node>);
};

#endif