#pragma once

#include "Ruota.h"
#include "Node.h"

namespace ruota
{
	class NodeParser
	{
	private:
		const std::vector<Token> tokens;
		const std::map<std::string, signed int> bOperators;
		const std::map<std::string, signed int> uOperators;
		const boost::filesystem::path currentFile;

		unsigned int index = 0;
		Token currentToken;
		void nextToken();
		std::shared_ptr<Node> parseNumNode();
		std::shared_ptr<Node> parseBoolNode();
		std::shared_ptr<Node> parseIDNode();
		std::shared_ptr<Node> parseBIDNode();
		std::shared_ptr<Node> parseEntryNode();
		std::shared_ptr<Node> parseExprNode();
		std::shared_ptr<Node> parseEquNode();
		std::shared_ptr<Node> parseVectorNode();
		std::shared_ptr<Node> parseUnitNode();
		std::shared_ptr<Node> parseBaseNode();
		std::shared_ptr<Node> parseUnOpNode();
		std::shared_ptr<Node> parseMapNode();
		std::shared_ptr<Node> parseIfElseNode();
		std::shared_ptr<Node> parseWhileNode();
		std::shared_ptr<Node> parseForNode();
		std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> parseSigNode(ValueType start);
		std::shared_ptr<Node> parseDefineNode();
		std::shared_ptr<Node> parseLambdaNode();
		std::shared_ptr<Node> parseExternNode();
		std::shared_ptr<Node> parseExternCallNode();
		std::shared_ptr<Node> parseCallBuiltNode();
		std::shared_ptr<Node> parseClassNode();
		std::shared_ptr<Node> parseNewNode();
		std::shared_ptr<Node> parseLoadNode();
		std::shared_ptr<Node> parseSwitchNode();
		std::shared_ptr<Node> parseTryCatchNode();
		std::shared_ptr<Node> parseTypeNode();
		std::shared_ptr<Node> parseTrailingNode(std::shared_ptr<Node>, bool);
		std::shared_ptr<Node> parseInsNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseUntilNode(std::shared_ptr<Node>, bool);
		std::shared_ptr<Node> parseCastToNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseBinOpNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseCallNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseIndexNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseThenNode(std::shared_ptr<Node>);

		std::shared_ptr<Node> logErrorN(const std::string &, const Token);
		std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> logErrorSN(const std::string &, const Token);

	public:
		NodeParser(const std::vector<Token> &, const std::map<std::string, signed int> &, const std::map<std::string, signed int> &, const boost::filesystem::path &);
		std::shared_ptr<Node> parse();
		static std::shared_ptr<Instruction> genParser(std::shared_ptr<Node>);
	};
}