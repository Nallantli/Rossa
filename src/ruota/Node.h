#ifndef NODE_H
#define NODE_H

#include "Ruota.h"

inline std::string colorASCII(TextColor color)
{
	return "\033[" + std::to_string(color) + "m";
}

class ContainerNode : public Node
{
private:
	Symbol s;

public:
	ContainerNode(Symbol, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class VectorNode : public Node
{
private:
	std::vector<std::unique_ptr<Node>> args;
	bool scoped;

public:
	VectorNode(std::vector<std::unique_ptr<Node>>, bool, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
	std::vector<std::unique_ptr<Node>> getChildren();
};

class BreakNode : public Node
{
public:
	BreakNode(const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class IDNode : public Node
{
private:
	hashcode_t key;

public:
	IDNode(hashcode_t, const Token);
	hashcode_t getKey() const;
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class BIDNode : public Node
{
private:
	std::string key;

public:
	BIDNode(const std::string &, const Token);
	const std::string getKey() const;
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class DefineNode : public Node
{
private:
	hashcode_t key;
	ValueType ftype;
	std::vector<std::pair<LexerTokenType, hashcode_t>> params;
	std::vector<std::unique_ptr<Node>> body;

public:
	DefineNode(hashcode_t, ValueType, std::vector<std::pair<LexerTokenType, hashcode_t>>, std::vector<std::unique_ptr<Node>>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class NewNode : public Node
{
private:
	std::unique_ptr<Node> object;
	std::unique_ptr<Node> params;

public:
	NewNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ClassNode : public Node
{
private:
	hashcode_t key;
	int type;
	std::vector<std::unique_ptr<Node>> body;
	std::unique_ptr<Node> extends;

public:
	ClassNode(hashcode_t, int, std::vector<std::unique_ptr<Node>>, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class VarNode : public Node
{
private:
	hashcode_t key;

public:
	VarNode(hashcode_t, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class CallNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::vector<std::unique_ptr<Node>> args;

public:
	CallNode(std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	std::unique_ptr<Node> getCallee();
	std::vector<std::unique_ptr<Node>> getArgs();
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ExternCallNode : public Node
{
private:
	std::string id;
	std::vector<std::unique_ptr<Node>> args;

public:
	ExternCallNode(const std::string &, std::vector<std::unique_ptr<Node>>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class CallBuiltNode : public Node
{
private:
	LexerTokenType t;
	std::unique_ptr<Node> arg;

public:
	CallBuiltNode(LexerTokenType, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ReturnNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReturnNode(std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ReferNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReferNode(std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class IndexNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	IndexNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class BinOpNode : public Node
{
private:
	std::string op;
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;

public:
	BinOpNode(const std::string &, std::unique_ptr<Node>, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	const std::string &getOp() const;
	std::unique_ptr<Node> getA();
	std::unique_ptr<Node> getB();
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class UnOpNode : public Node
{
private:
	std::string op;
	std::unique_ptr<Node> a;

public:
	UnOpNode(const std::string &, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class CastToNode : public Node
{
private:
	ValueType convert;
	std::unique_ptr<Node> a;

public:
	CastToNode(ValueType, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class InsNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	InsNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	std::unique_ptr<Node> getCallee();
	std::unique_ptr<Node> getArg();
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class IfElseNode : public Node
{
private:
	std::unique_ptr<Node> ifs;
	std::unique_ptr<Node> body;
	std::unique_ptr<Node> elses = nullptr;

public:
	IfElseNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token);
	void setElse(std::unique_ptr<Node>);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class WhileNode : public Node
{
private:
	std::unique_ptr<Node> whiles;
	std::vector<std::unique_ptr<Node>> body;

public:
	WhileNode(std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ForNode : public Node
{
private:
	hashcode_t id;
	std::unique_ptr<Node> fors;
	std::vector<std::unique_ptr<Node>> body;

public:
	ForNode(hashcode_t, std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class UntilNode : public Node
{
private:
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;

public:
	UntilNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class MapNode : public Node
{
private:
	std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>> args;

public:
	MapNode(std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class SwitchNode : public Node
{
private:
	std::unique_ptr<Node> switchs;
	std::map<Symbol, std::unique_ptr<Node>> cases;
	std::unique_ptr<Node> elses;

public:
	SwitchNode(std::unique_ptr<Node>, std::map<Symbol, std::unique_ptr<Node>>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	void setElse(std::unique_ptr<Node>);
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class TryCatchNode : public Node
{
private:
	std::unique_ptr<Node> trys;
	std::unique_ptr<Node> catchs;
	hashcode_t key;

public:
	TryCatchNode(std::unique_ptr<Node>, std::unique_ptr<Node>, hashcode_t, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ThrowNode : public Node
{
private:
	std::unique_ptr<Node> throws;

public:
	ThrowNode(std::unique_ptr<Node>, const Token);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

#endif