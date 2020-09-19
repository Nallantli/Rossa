#ifndef NODE_H
#define NODE_H

#include "Ruota.hpp"
#include "Lexer.hpp"
#include <vector>
#include <memory>
#include <iostream>

class NodeParser;

class Node;
class EntryNode;
class CallNode;
class IndexNode;
class UnOpNode;
class BinOpNode;
class InsNode;
class IfElseNode;
class WhileNode;
class ForNode;
class VectorNode;
class DefineNode;
class ReturnNode;
//class ExprNode;
//class EquNode;
class IDNode;
class NumNode;
class BoolNode;
class VarNode;
class StringNode;
class ExternCallNode;

class Node
{
public:
	virtual Instruction *genParser() const = 0;
};

class EntryNode : public Node
{
private:
	std::vector<std::unique_ptr<Node>> children;

public:
	EntryNode() {}
	void addNode(std::unique_ptr<Node> node) { children.push_back(std::move(node)); }
	Instruction *genParser() const override;
};

class VectorNode : public Node
{
private:
	std::vector<std::unique_ptr<Node>> args;

public:
	VectorNode(std::vector<std::unique_ptr<Node>> args) : args(std::move(args)) {}
	Instruction *genParser() const override;
	std::vector<std::unique_ptr<Node>> getChildren() { return std::move(args); }
};

class NumNode : public Node
{
private:
	double numberValue;

public:
	NumNode(double numberValue) : numberValue(numberValue) {}
	Instruction *genParser() const override;
};

class NilNode : public Node
{
public:
	NilNode() {}
	Instruction *genParser() const override;
};

class StringNode : public Node
{
private:
	std::string stringValue;

public:
	StringNode(const std::string &stringValue) : stringValue(stringValue) {}
	Instruction *genParser() const override;
};

class BoolNode : public Node
{
private:
	bool boolValue;

public:
	BoolNode(double boolValue) : boolValue(boolValue) {}
	Instruction *genParser() const override;
};

class IDNode : public Node
{
private:
	std::string key;

public:
	IDNode(const std::string &key) : key(key) {}
	const std::string getKey() const
	{
		return key;
	}
	Instruction *genParser() const override;
};

class DefineNode : public Node
{
private:
	std::string key;
	std::unique_ptr<Node> params;
	std::unique_ptr<Node> body;

public:
	DefineNode(const std::string &key, std::unique_ptr<Node> params, std::unique_ptr<Node> body) : key(key), params(std::move(params)), body(std::move(body)) {}
	Instruction *genParser() const override;
};

class VarNode : public Node
{
private:
	std::string key;

public:
	VarNode(const std::string &key) : key(key) {}
	Instruction *genParser() const override;
};

class CallNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::vector<std::unique_ptr<Node>> args;

public:
	CallNode(std::unique_ptr<Node> callee, std::vector<std::unique_ptr<Node>> args) : callee(std::move(callee)), args(std::move(args)) {}
	Instruction *genParser() const override;
};

class ExternCallNode : public Node
{
private:
	boost::function<SYM(std::vector<SYM>)> f;
	std::vector<std::unique_ptr<Node>> args;

public:
	ExternCallNode(boost::function<SYM(std::vector<SYM>)> f, std::vector<std::unique_ptr<Node>> args) : f(f), args(std::move(args)) {}
	Instruction *genParser() const override;
};

class ReturnNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReturnNode(std::unique_ptr<Node> a) : a(std::move(a)) {}
	Instruction *genParser() const override;
};

class IndexNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	IndexNode(std::unique_ptr<Node> callee, std::unique_ptr<Node> arg) : callee(std::move(callee)), arg(std::move(arg)) {}
	Instruction *genParser() const override;
};

class BinOpNode : public Node
{
private:
	std::string op;
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;

public:
	BinOpNode(const std::string &op, std::unique_ptr<Node> a, std::unique_ptr<Node> b) : op(op), a(std::move(a)), b(std::move(b)) {}
	Instruction *genParser() const override;
	const std::string getOp() { return op; }
	std::unique_ptr<Node> getA() { return std::move(a); };
	std::unique_ptr<Node> getB() { return std::move(b); };
};

class UnOpNode : public Node
{
private:
	std::string op;
	std::unique_ptr<Node> a;

public:
	UnOpNode(const std::string &op, std::unique_ptr<Node> a) : op(op), a(std::move(a)) {}
	Instruction *genParser() const override;
};

class InsNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	InsNode(std::unique_ptr<Node> callee, std::unique_ptr<Node> arg) : callee(std::move(callee)), arg(std::move(arg)) {}
	Instruction *genParser() const override;
	std::unique_ptr<Node> getCallee() { return std::move(callee); }
	std::unique_ptr<Node> getArg() { return std::move(arg); }
};

class IfElseNode : public Node
{
private:
	std::unique_ptr<Node> ifs;
	std::unique_ptr<Node> body;
	std::unique_ptr<Node> elses = nullptr;

public:
	IfElseNode(std::unique_ptr<Node> ifs, std::unique_ptr<Node> body) : ifs(std::move(ifs)), body(std::move(body)) {}
	void setElse(std::unique_ptr<Node> elses) { this->elses = std::move(elses); }
	Instruction *genParser() const override;
};

class WhileNode : public Node
{
private:
	std::unique_ptr<Node> whiles;
	std::unique_ptr<Node> body;

public:
	WhileNode(std::unique_ptr<Node> whiles, std::unique_ptr<Node> body) : whiles(std::move(whiles)), body(std::move(body)) {}
	Instruction *genParser() const override;
};

class ForNode : public Node
{
private:
	std::string id;
	std::unique_ptr<Node> fors;
	std::unique_ptr<Node> body;

public:
	ForNode(const std::string &id, std::unique_ptr<Node> fors, std::unique_ptr<Node> body) : id(id), fors(std::move(fors)), body(std::move(body)) {}
	Instruction *genParser() const override;
};

class NodeParser
{
private:
	std::map<std::string, signed int> bOperators;
	std::map<std::string, signed int> uOperators;
	unsigned int index = 0;
	Token currentToken;
	std::vector<Token> tokens;
	Token nextToken();
	std::unique_ptr<Node> parseNumNode();
	std::unique_ptr<Node> parseBoolNode();
	std::unique_ptr<Node> parseIDNode();
	std::unique_ptr<Node> parseEntryNode();
	std::unique_ptr<Node> parseExprNode();
	std::unique_ptr<Node> parseEquNode();
	std::unique_ptr<Node> parseVectorNode();
	std::unique_ptr<Node> parseUnitNode();
	std::unique_ptr<Node> parseBaseNode();
	std::unique_ptr<Node> parseUnOpNode();
	std::unique_ptr<Node> parseIfElseNode();
	std::unique_ptr<Node> parseWhileNode();
	std::unique_ptr<Node> parseForNode();
	std::unique_ptr<Node> parseSigNode();
	std::unique_ptr<Node> parseInsNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseDefineNode();
	std::unique_ptr<Node> parseLambdaNode();
	std::unique_ptr<Node> parseExternNode();
	std::unique_ptr<Node> parseExternCallNode();
	std::unique_ptr<Node> parseBinOpNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCallNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseIndexNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> logErrorN(const std::string &, Token);

public:
	NodeParser(std::vector<Token> tokens, std::map<std::string, signed int> bOperators, std::map<std::string, signed int> uOperators) : tokens(tokens), bOperators(bOperators), uOperators(uOperators) {}
	std::unique_ptr<Node> parse();
	static Instruction *genParser(std::unique_ptr<Node>);
};

#endif