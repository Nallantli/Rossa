#ifndef NODE_H
#define NODE_H

#include "Ruota.h"
#include "Lexer.h"
#include <vector>
#include <memory>
#include <iostream>

enum NODE_TYPE
{
	ENTRY_NODE,
	CALL_NODE,
	CALL_BUILT_NODE,
	INDEX_NODE,
	UN_OP_NODE,
	BIN_OP_NODE,
	INS_NODE,
	IF_ELSE_NODE,
	WHILE_NODE,
	FOR_NODE,
	VECTOR_NODE,
	DEFINE_NODE,
	RETURN_NODE,
	ID_NODE,
	NUM_NODE,
	BOOL_NODE,
	VAR_NODE,
	STRING_NODE,
	CLASS_NODE,
	EXTERN_CALL_NODE,
	NEW_NODE,
	NIL_NODE,
	CAST_TO_NODE,
	UNTIL_NODE
};

class NodeParser;

class Node;
class EntryNode;
class CallNode;
class CallBuiltNode;
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
class ClassNode;
class ExternCallNode;
class NewNode;
class CastToNode;
class UntilNode;

class Node
{
protected:
	NODE_TYPE type;

public:
	Node(NODE_TYPE type) : type(type) {}
	virtual Instruction *genParser() const = 0;
	NODE_TYPE getType() const { return type; }
};

class EntryNode : public Node
{
private:
	std::vector<std::unique_ptr<Node>> children;

public:
	EntryNode() : Node(ENTRY_NODE) {}
	void addNode(std::unique_ptr<Node> node) { children.push_back(std::move(node)); }
	Instruction *genParser() const override;
};

class VectorNode : public Node
{
private:
	std::vector<std::unique_ptr<Node>> args;

public:
	VectorNode(std::vector<std::unique_ptr<Node>> args) : Node(VECTOR_NODE), args(std::move(args)) {}
	Instruction *genParser() const override;
	std::vector<std::unique_ptr<Node>> getChildren() { return std::move(args); }
};

class NumNode : public Node
{
private:
	long_double_t numberValue;

public:
	NumNode(long_double_t numberValue) : Node(NUM_NODE), numberValue(numberValue) {}
	Instruction *genParser() const override;
};

class NilNode : public Node
{
public:
	NilNode() : Node(NIL_NODE) {}
	Instruction *genParser() const override;
};

class StringNode : public Node
{
private:
	std::string stringValue;

public:
	StringNode(const std::string &stringValue) : Node(STRING_NODE), stringValue(stringValue) {}
	Instruction *genParser() const override;
};

class BoolNode : public Node
{
private:
	bool boolValue;

public:
	BoolNode(bool boolValue) : Node(BOOL_NODE), boolValue(boolValue) {}
	Instruction *genParser() const override;
};

class IDNode : public Node
{
private:
	std::string key;

public:
	IDNode(const std::string &key) : Node(ID_NODE), key(key) {}
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
	DefineNode(const std::string &key, std::unique_ptr<Node> params, std::unique_ptr<Node> body) : Node(DEFINE_NODE), key(key), params(std::move(params)), body(std::move(body)) {}
	Instruction *genParser() const override;
};

class NewNode : public Node
{
private:
	std::unique_ptr<Node> object;
	std::unique_ptr<Node> params;

public:
	NewNode(std::unique_ptr<Node> object, std::unique_ptr<Node> params) : Node(NEW_NODE), object(std::move(object)), params(std::move(params)) {}
	Instruction *genParser() const override;
};

class ClassNode : public Node
{
private:
	std::string key;
	int type;
	std::unique_ptr<Node> body;

public:
	ClassNode(const std::string &key, int type, std::unique_ptr<Node> body) : Node(CLASS_NODE), key(key), type(type), body(std::move(body)) {}
	Instruction *genParser() const override;
};

class VarNode : public Node
{
private:
	std::string key;

public:
	VarNode(const std::string &key) : Node(VAR_NODE), key(key) {}
	Instruction *genParser() const override;
};

class CallNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::vector<std::unique_ptr<Node>> args;

public:
	CallNode(std::unique_ptr<Node> callee, std::vector<std::unique_ptr<Node>> args) : Node(CALL_NODE), callee(std::move(callee)), args(std::move(args)) {}
	Instruction *genParser() const override;
	std::unique_ptr<Node> getCallee() { return std::move(callee); }
	std::vector<std::unique_ptr<Node>> getArgs() { return std::move(args); }
};

class ExternCallNode : public Node
{
private:
	boost::function<SYM(std::vector<SYM>)> f;
	std::vector<std::unique_ptr<Node>> args;

public:
	ExternCallNode(boost::function<SYM(std::vector<SYM>)> f, std::vector<std::unique_ptr<Node>> args) : Node(EXTERN_CALL_NODE), f(f), args(std::move(args)) {}
	Instruction *genParser() const override;
};

class CallBuiltNode : public Node
{
private:
	LEX_TOKEN_TYPE t;
	std::unique_ptr<Node> arg;

public:
	CallBuiltNode(LEX_TOKEN_TYPE t, std::unique_ptr<Node> arg) : Node(CALL_BUILT_NODE), t(t), arg(std::move(arg)) {}
	Instruction *genParser() const override;
};

class ReturnNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReturnNode(std::unique_ptr<Node> a) : Node(RETURN_NODE), a(std::move(a)) {}
	Instruction *genParser() const override;
};

class IndexNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	IndexNode(std::unique_ptr<Node> callee, std::unique_ptr<Node> arg) : Node(INDEX_NODE), callee(std::move(callee)), arg(std::move(arg)) {}
	Instruction *genParser() const override;
};

class BinOpNode : public Node
{
private:
	std::string op;
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;

public:
	BinOpNode(const std::string &op, std::unique_ptr<Node> a, std::unique_ptr<Node> b) : Node(BIN_OP_NODE), op(op), a(std::move(a)), b(std::move(b)) {}
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
	UnOpNode(const std::string &op, std::unique_ptr<Node> a) : Node(UN_OP_NODE), op(op), a(std::move(a)) {}
	Instruction *genParser() const override;
};

class CastToNode : public Node
{
private:
	D_TYPE convert;
	std::unique_ptr<Node> a;

public:
	CastToNode(D_TYPE convert, std::unique_ptr<Node> a) : Node(CAST_TO_NODE), convert(convert), a(std::move(a)) {}
	Instruction *genParser() const override;
};

class InsNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	InsNode(std::unique_ptr<Node> callee, std::unique_ptr<Node> arg) : Node(INS_NODE), callee(std::move(callee)), arg(std::move(arg)) {}
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
	IfElseNode(std::unique_ptr<Node> ifs, std::unique_ptr<Node> body) : Node(IF_ELSE_NODE), ifs(std::move(ifs)), body(std::move(body)) {}
	void setElse(std::unique_ptr<Node> elses) { this->elses = std::move(elses); }
	Instruction *genParser() const override;
};

class WhileNode : public Node
{
private:
	std::unique_ptr<Node> whiles;
	std::unique_ptr<Node> body;

public:
	WhileNode(std::unique_ptr<Node> whiles, std::unique_ptr<Node> body) : Node(WHILE_NODE), whiles(std::move(whiles)), body(std::move(body)) {}
	Instruction *genParser() const override;
};

class ForNode : public Node
{
private:
	std::string id;
	std::unique_ptr<Node> fors;
	std::unique_ptr<Node> body;

public:
	ForNode(const std::string &id, std::unique_ptr<Node> fors, std::unique_ptr<Node> body) : Node(FOR_NODE), id(id), fors(std::move(fors)), body(std::move(body)) {}
	Instruction *genParser() const override;
};

class UntilNode : public Node
{
private:
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;

public:
	UntilNode(std::unique_ptr<Node> a, std::unique_ptr<Node> b) : Node(UNTIL_NODE), a(std::move(a)), b(std::move(b)) {}
	Instruction *genParser() const override;
};

class NodeParser
{
private:
	boost::filesystem::path currentDir;
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
	std::unique_ptr<Node> parseCallBuiltNode();
	std::unique_ptr<Node> parseClassNode();
	std::unique_ptr<Node> parseNewNode();
	std::unique_ptr<Node> parseLoadNode();
	std::unique_ptr<Node> parseTrailingNode(std::unique_ptr<Node>, bool);
	std::unique_ptr<Node> parseUntilNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCastToNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseBinOpNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCallNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseIndexNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> logErrorN(const std::string &, Token);

public:
	NodeParser(std::vector<Token> tokens, std::map<std::string, signed int> bOperators, std::map<std::string, signed int> uOperators, boost::filesystem::path currentDir) : tokens(tokens), bOperators(bOperators), uOperators(uOperators), currentDir(currentDir) {}
	std::unique_ptr<Node> parse();
	static Instruction *genParser(std::unique_ptr<Node>);
};

#endif