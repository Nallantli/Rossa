#ifndef NODE_H
#define NODE_H

#include "../rossa.h"

#include "../signature/signature.h"
#include "../symbol/symbol.h"

class Node
{
	friend class node_parser_t;

protected:
	const std::vector<node_scope_t> path;

	enum type_t
	{
		ENTRY_NODE,
		CALL_NODE,
		CALL_BUILT_NODE,
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
		VAR_NODE,
		CLASS_NODE,
		EXTERN_CALL_NODE,
		NEW_NODE,
		CAST_TO_NODE,
		UNTIL_NODE,
		MAP_NODE,
		CONTAINER_NODE,
		BREAK_NODE,
		CONTINUE_NODE,
		REFER_NODE,
		SWITCH_NODE,
		BID_NODE,
		TRY_CATCH_NODE,
		THROW_NODE,
		PAREN_NODE,
		CALL_OP_NODE,
		VARG_DEFINE_NODE,
		EACH_NODE
	} const type;
	const token_t token;

public:
	Node(const std::vector<node_scope_t> &, const type_t &, const token_t &);
	const type_t getType() const;
	const token_t getToken() const;

	virtual ptr_instruction_t genParser() const = 0;
	virtual bool isConst() const = 0;
	virtual void printTree(std::string, bool) const = 0;
	virtual const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const = 0;
};

class ContainerNode : public Node
{
private:
	const symbol_t s;

public:
	ContainerNode(const std::vector<node_scope_t> &, const symbol_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class VectorNode : public Node
{
private:
	const std::vector<ptr_node_t> args;
	bool scoped;

public:
	VectorNode(const std::vector<node_scope_t> &, const std::vector<ptr_node_t> &, const bool &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
	const std::vector<ptr_node_t> &getChildren();
};

class BreakNode : public Node
{
public:
	BreakNode(const std::vector<node_scope_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ContinueNode : public Node
{
public:
	ContinueNode(const std::vector<node_scope_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class IDNode : public Node
{
private:
	const hash_ull key;

public:
	IDNode(const std::vector<node_scope_t> &, const hash_ull &, const token_t &);
	hash_ull getKey() const;
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class BIDNode : public Node
{
private:
	const std::string key;

public:
	BIDNode(const std::vector<node_scope_t> &, const std::string &, const token_t &);
	const std::string getKey() const;
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class DefineNode : public Node
{
private:
	const hash_ull key;
	const signature_t ftype;
	const std::vector<std::pair<token_type_enum, hash_ull>> params;
	const ptr_node_t body;
	const std::vector<hash_ull> captures;

public:
	DefineNode(const std::vector<node_scope_t> &, const hash_ull &, const signature_t &, const std::vector<std::pair<token_type_enum, hash_ull>> &, const ptr_node_t &, const std::vector<hash_ull> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class VargDefineNode : public Node
{
private:
	const hash_ull key;
	const ptr_node_t body;
	const std::vector<hash_ull> captures;

public:
	VargDefineNode(const std::vector<node_scope_t> &, const hash_ull &, const ptr_node_t &, const std::vector<hash_ull> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class NewNode : public Node
{
private:
	const ptr_node_t object;
	const ptr_node_t params;

public:
	NewNode(const std::vector<node_scope_t> &, const ptr_node_t &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ClassNode : public Node
{
private:
	const hash_ull key;
	const int type;
	const std::vector<ptr_node_t> body;
	const ptr_node_t extends;

public:
	ClassNode(const std::vector<node_scope_t> &, const hash_ull &, const int &, const std::vector<ptr_node_t> &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class VarNode : public Node
{
private:
	const std::vector<hash_ull> keys;

public:
	VarNode(const std::vector<node_scope_t> &, const std::vector<hash_ull> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class CallNode : public Node
{
private:
	const ptr_node_t callee;
	const std::vector<ptr_node_t> args;

public:
	CallNode(const std::vector<node_scope_t> &, const ptr_node_t &, const std::vector<ptr_node_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	ptr_node_t getCallee() const;
	std::vector<ptr_node_t> getArgs() const;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ExternCallNode : public Node
{
private:
	const std::string libname;
	const std::string fname;
	const std::vector<ptr_node_t> args;

public:
	ExternCallNode(const std::vector<node_scope_t> &, const std::string &, const std::string &, const std::vector<ptr_node_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class CallBuiltNode : public Node
{
private:
	const token_type_enum t;
	const ptr_node_t arg;

public:
	CallBuiltNode(const std::vector<node_scope_t> &, const token_type_enum &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ReturnNode : public Node
{
private:
	const ptr_node_t a;

public:
	ReturnNode(const std::vector<node_scope_t> &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ReferNode : public Node
{
private:
	const ptr_node_t a;

public:
	ReferNode(const std::vector<node_scope_t> &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class BinOpNode : public Node
{
private:
	const std::string op;
	ptr_node_t a;
	ptr_node_t b;

public:
	BinOpNode(const std::vector<node_scope_t> &, const std::string &, const ptr_node_t &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	const std::string &getOp() const;
	const ptr_node_t getA() const;
	const ptr_node_t getB() const;
	void setA(const ptr_node_t &);
	void setB(const ptr_node_t &);
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class UnOpNode : public Node
{
private:
	const std::string op;
	const ptr_node_t a;

public:
	UnOpNode(const std::vector<node_scope_t> &, const std::string &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ParenNode : public Node
{
private:
	const ptr_node_t a;

public:
	ParenNode(const std::vector<node_scope_t> &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class InsNode : public Node
{
private:
	const ptr_node_t callee;
	const ptr_node_t arg;

public:
	InsNode(const std::vector<node_scope_t> &, const ptr_node_t &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	const ptr_node_t getCallee() const;
	const ptr_node_t getArg() const;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class IfElseNode : public Node
{
private:
	const ptr_node_t ifs;
	const ptr_node_t body;
	ptr_node_t elses = nullptr;

public:
	IfElseNode(const std::vector<node_scope_t> &, const ptr_node_t &, const ptr_node_t &, const token_t &);
	void setElse(const ptr_node_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class WhileNode : public Node
{
private:
	const ptr_node_t whiles;
	const std::vector<ptr_node_t> body;

public:
	WhileNode(const std::vector<node_scope_t> &, const ptr_node_t &, const std::vector<ptr_node_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ForNode : public Node
{
private:
	hash_ull id;
	ptr_node_t fors;
	std::vector<ptr_node_t> body;

public:
	ForNode(const std::vector<node_scope_t> &, const hash_ull &, const ptr_node_t &, const std::vector<ptr_node_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class UntilNode : public Node
{
private:
	const ptr_node_t a;
	const ptr_node_t b;
	const ptr_node_t step;
	const bool inclusive;

public:
	UntilNode(const std::vector<node_scope_t> &, const ptr_node_t &, const ptr_node_t &, const ptr_node_t &, const bool &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class MapNode : public Node
{
private:
	const std::vector<std::pair<std::string, ptr_node_t>> args;

public:
	MapNode(const std::vector<node_scope_t> &, const std::vector<std::pair<std::string, ptr_node_t>> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class SwitchNode : public Node
{
private:
	const ptr_node_t switchs;
	const std::map<ptr_node_t, size_t> cases;
	const std::vector<ptr_node_t> gotos;
	ptr_node_t elses;

public:
	SwitchNode(const std::vector<node_scope_t> &, const ptr_node_t &, const std::map<ptr_node_t, size_t> &, const std::vector<ptr_node_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	void setElse(const ptr_node_t &);
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class TryCatchNode : public Node
{
private:
	const ptr_node_t trys;
	const ptr_node_t catchs;
	const hash_ull key;

public:
	TryCatchNode(const std::vector<node_scope_t> &, const ptr_node_t &, const ptr_node_t &, const hash_ull &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class ThrowNode : public Node
{
private:
	const ptr_node_t throws;

public:
	ThrowNode(const std::vector<node_scope_t> &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class CallOpNode : public Node
{
private:
	const size_t id;
	const std::vector<ptr_node_t> args;

public:
	CallOpNode(const std::vector<node_scope_t> &, const size_t &, const std::vector<ptr_node_t> &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

class EachNode : public Node
{
private:
	hash_ull id;
	ptr_node_t eachs;
	ptr_node_t wheres;
	ptr_node_t body;

public:
	EachNode(const std::vector<node_scope_t> &, const hash_ull &, const ptr_node_t &, const ptr_node_t &, const ptr_node_t &, const token_t &);
	ptr_instruction_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const ptr_node_t fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &) const override;
};

#endif