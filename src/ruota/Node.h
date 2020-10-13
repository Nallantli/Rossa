#ifndef NODE_H
#define NODE_H

#include "Ruota.h"

inline string colorASCII(const TextColor &color)
{
	return "\033[" + std::to_string(color) + "m";
}

class ContainerNode : public Node
{
private:
	const Symbol s;

public:
	ContainerNode(const Symbol &, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class VectorNode : public Node
{
private:
	const std::vector<std::unique_ptr<Node>> args;
	bool scoped;

public:
	VectorNode(std::vector<std::unique_ptr<Node>>, bool, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
	const std::vector<std::unique_ptr<Node>> &getChildren();
};

class BreakNode : public Node
{
public:
	BreakNode(const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class IDNode : public Node
{
private:
	hash_ull key;

public:
	IDNode(hash_ull, const Token &);
	hash_ull getKey() const;
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class BIDNode : public Node
{
private:
	string key;

public:
	BIDNode(const string &, const Token &);
	const string getKey() const;
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class DefineNode : public Node
{
private:
	hash_ull key;
	Signature ftype;
	std::vector<std::pair<LexerTokenType, hash_ull>> params;
	std::unique_ptr<Node> body;

public:
	DefineNode(hash_ull, Signature, std::vector<std::pair<LexerTokenType, hash_ull>>, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class NewNode : public Node
{
private:
	std::unique_ptr<Node> object;
	std::unique_ptr<Node> params;

public:
	NewNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ClassNode : public Node
{
private:
	hash_ull key;
	int type;
	std::vector<std::unique_ptr<Node>> body;
	std::unique_ptr<Node> extends;

public:
	ClassNode(hash_ull, int, std::vector<std::unique_ptr<Node>>, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class VarNode : public Node
{
private:
	std::vector<hash_ull> keys;

public:
	VarNode(std::vector<hash_ull>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class CallNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::vector<std::unique_ptr<Node>> args;

public:
	CallNode(std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	std::unique_ptr<Node> getCallee();
	std::vector<std::unique_ptr<Node>> getArgs();
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ExternCallNode : public Node
{
private:
	string id;
	std::vector<std::unique_ptr<Node>> args;

public:
	ExternCallNode(const string &, std::vector<std::unique_ptr<Node>>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class CallBuiltNode : public Node
{
private:
	LexerTokenType t;
	std::unique_ptr<Node> arg;

public:
	CallBuiltNode(LexerTokenType, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ReturnNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReturnNode(std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ReferNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReferNode(std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class BinOpNode : public Node
{
private:
	string op;
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;

public:
	BinOpNode(const string &, std::unique_ptr<Node>, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	const string &getOp() const;
	std::unique_ptr<Node> getA();
	std::unique_ptr<Node> getB();
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class UnOpNode : public Node
{
private:
	string op;
	std::unique_ptr<Node> a;

public:
	UnOpNode(const string &, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class CastToNode : public Node
{
private:
	ValueType convert;
	std::unique_ptr<Node> a;

public:
	CastToNode(ValueType, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class InsNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	InsNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	std::unique_ptr<Node> getCallee();
	std::unique_ptr<Node> getArg();
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class IfElseNode : public Node
{
private:
	std::unique_ptr<Node> ifs;
	std::unique_ptr<Node> body;
	std::unique_ptr<Node> elses = nullptr;

public:
	IfElseNode(std::unique_ptr<Node>, std::unique_ptr<Node>, const Token &);
	void setElse(std::unique_ptr<Node>);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class WhileNode : public Node
{
private:
	std::unique_ptr<Node> whiles;
	std::vector<std::unique_ptr<Node>> body;

public:
	WhileNode(std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ForNode : public Node
{
private:
	hash_ull id;
	std::unique_ptr<Node> fors;
	std::vector<std::unique_ptr<Node>> body;

public:
	ForNode(hash_ull, std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class UntilNode : public Node
{
private:
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;
	std::unique_ptr<Node> step = nullptr;
	bool inclusive;

public:
	UntilNode(std::unique_ptr<Node>, std::unique_ptr<Node>, std::unique_ptr<Node>, bool, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class MapNode : public Node
{
private:
	std::vector<std::pair<hash_ull, std::unique_ptr<Node>>> args;

public:
	MapNode(std::vector<std::pair<hash_ull, std::unique_ptr<Node>>>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class SwitchNode : public Node
{
private:
	std::unique_ptr<Node> switchs;
	std::map<std::unique_ptr<Node>, std::unique_ptr<Node>> cases;
	std::unique_ptr<Node> elses;

public:
	SwitchNode(std::unique_ptr<Node>, std::map<std::unique_ptr<Node>, std::unique_ptr<Node>>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	void setElse(std::unique_ptr<Node>);
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class TryCatchNode : public Node
{
private:
	std::unique_ptr<Node> trys;
	std::unique_ptr<Node> catchs;
	hash_ull key;

public:
	TryCatchNode(std::unique_ptr<Node>, std::unique_ptr<Node>, hash_ull, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

class ThrowNode : public Node
{
private:
	std::unique_ptr<Node> throws;

public:
	ThrowNode(std::unique_ptr<Node>, const Token &);
	std::shared_ptr<Instruction> genParser() const override;
	bool isConst() const override;
	void printTree(string, bool) const override;
	std::unique_ptr<Node> fold() const override;
};

#endif