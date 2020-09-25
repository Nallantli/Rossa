#ifndef NODE_H
#define NODE_H

#include "Ruota.h"

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
	UNTIL_NODE,
	MAP_NODE,
	CONTAINER_NODE,
	BREAK_NODE,
	REFER_NODE
};

class NodeParser;

class Node;
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
class MapNode;
class ContainerNode;
class BreakNode;
class ReferNode;

class Node
{
protected:
	NODE_TYPE type;

public:
	Node(NODE_TYPE type) : type(type) {}
	virtual Instruction *genParser() const = 0;
	virtual bool isConst() const = 0;
	virtual void printTree(std::string indent, bool last) const = 0;
	virtual std::unique_ptr<Node> fold() const = 0;
	NODE_TYPE getType() const { return type; }
};

class ContainerNode : public Node
{
private:
	Symbol s;

public:
	ContainerNode(Symbol s) : Node(CONTAINER_NODE), s(s.setMutable(false)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CONTAINER"
				  << " : " << s.getValueType() << ", " << s.toCodeString() << "\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ContainerNode>(s);
	}
};

class VectorNode : public Node
{
private:
	std::vector<std::unique_ptr<Node>> args;

public:
	VectorNode(std::vector<std::unique_ptr<Node>> args) : Node(VECTOR_NODE), args(std::move(args)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		for (auto &c : args)
			if (!c->isConst())
				return false;
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "VECTOR"
				  << "\n"
				  << colorASCII(RESET_TEXT);

		for (size_t i = 0; i < args.size(); i++)
			args[i]->printTree(indent, i == args.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::unique_ptr<Node>> nargs;
		for (auto &c : args)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nargs.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nargs.push_back(c->fold());
			}
		}
		return std::make_unique<VectorNode>(std::move(nargs));
	}
	std::vector<std::unique_ptr<Node>> getChildren() { return std::move(args); }
};

class NumNode : public Node
{
private:
	NUMBER_TYPE numberValue;

public:
	NumNode(NUMBER_TYPE numberValue) : Node(NUM_NODE), numberValue(numberValue) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "NUMBER"
				  << " : " << NUMBER_STRING(numberValue) << "\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ContainerNode>(Symbol(numberValue));
	}
};

class NilNode : public Node
{
public:
	NilNode() : Node(NIL_NODE) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "NIL"
				  << " : nil\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ContainerNode>(Symbol());
	}
};

class BreakNode : public Node
{
public:
	BreakNode() : Node(BREAK_NODE) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "BREAK\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ContainerNode>(Symbol(ID_BREAK));
	}
};

class StringNode : public Node
{
private:
	std::string stringValue;

public:
	StringNode(const std::string &stringValue) : Node(STRING_NODE), stringValue(stringValue) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "STRING"
				  << " : " << stringValue << "\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ContainerNode>(Symbol(stringValue));
	}
};

class BoolNode : public Node
{
private:
	bool boolValue;

public:
	BoolNode(bool boolValue) : Node(BOOL_NODE), boolValue(boolValue) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "NUMBER"
				  << " : " << (boolValue ? "true" : "false") << "\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ContainerNode>(Symbol(boolValue));
	}
};

class IDNode : public Node
{
private:
	hashcode_t key;

public:
	IDNode(hashcode_t key) : Node(ID_NODE), key(key) {}
	const hashcode_t getKey() const
	{
		return key;
	}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "ID"
				  << " : " << hash.deHash(key) << "\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<IDNode>(key);
	}
};

class DefineNode : public Node
{
private:
	hashcode_t key;
	D_TYPE ftype;
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params;
	std::vector<std::unique_ptr<Node>> body;

public:
	DefineNode(hashcode_t key, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params, std::vector<std::unique_ptr<Node>> body) : Node(DEFINE_NODE), key(key), ftype(ftype), params(params), body(std::move(body)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "DEFINE"
				  << " : " << (key > 0 ? hash.deHash(key) : "<LAMBDA>") << ", " << std::to_string(ftype) << "\n"
				  << colorASCII(RESET_TEXT);

		for (size_t i = 0; i < body.size(); i++)
			body[i]->printTree(indent, i == body.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::unique_ptr<Node>> nbody;
		for (auto &c : body)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nbody.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nbody.push_back(c->fold());
			}
		}
		return std::make_unique<DefineNode>(key, ftype, params, std::move(nbody));
	}
};

class NewNode : public Node
{
private:
	std::unique_ptr<Node> object;
	std::unique_ptr<Node> params;

public:
	NewNode(std::unique_ptr<Node> object, std::unique_ptr<Node> params) : Node(NEW_NODE), object(std::move(object)), params(std::move(params)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << "NEW(" << (isConst() ? "true" : "false") << "\n"
				  << colorASCII(RESET_TEXT);
		object->printTree(indent, false);
		params->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<NewNode>(object->fold(), params->fold());
	}
};

class ClassNode : public Node
{
private:
	hashcode_t key;
	int type;
	std::vector<std::unique_ptr<Node>> body;
	std::unique_ptr<Node> extends;

public:
	ClassNode(hashcode_t key, int type, std::vector<std::unique_ptr<Node>> body, std::unique_ptr<Node> extends) : Node(CLASS_NODE), key(key), type(type), body(std::move(body)), extends(std::move(extends)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CLASS"
				  << " : " << hash.deHash(key) << ", " << std::to_string(type) << "\n"
				  << colorASCII(RESET_TEXT);
		if (extends != nullptr)
			extends->printTree(indent, false);
		for (size_t i = 0; i < body.size(); i++)
			body[i]->printTree(indent, i == body.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::unique_ptr<Node>> nbody;
		for (auto &c : body)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nbody.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nbody.push_back(c->fold());
			}
		}

		if (extends)
			return std::make_unique<ClassNode>(key, type, std::move(nbody), extends->fold());
		else
			return std::make_unique<ClassNode>(key, type, std::move(nbody), nullptr);
	}
};

class VarNode : public Node
{
private:
	hashcode_t key;

public:
	VarNode(hashcode_t key) : Node(VAR_NODE), key(key) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "VAR"
				  << " : " << hash.deHash(key) << "\n"
				  << colorASCII(RESET_TEXT);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<VarNode>(key);
	}
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
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CALL"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		callee->printTree(indent, false);
		for (size_t i = 0; i < args.size(); i++)
			args[i]->printTree(indent, i == args.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::unique_ptr<Node>> nargs;
		for (auto &c : args)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nargs.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nargs.push_back(c->fold());
			}
		}

		return std::make_unique<CallNode>(callee->fold(), std::move(nargs));
	}
};

class ExternCallNode : public Node
{
private:
	std::string id;
	std::vector<std::unique_ptr<Node>> args;

public:
	ExternCallNode(const std::string &id, std::vector<std::unique_ptr<Node>> args) : Node(EXTERN_CALL_NODE), id(id), args(std::move(args)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "EXTERN_CALL"
				  << " : " << id << "\n"
				  << colorASCII(RESET_TEXT);
		for (size_t i = 0; i < args.size(); i++)
			args[i]->printTree(indent, i == args.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		std::vector<std::unique_ptr<Node>> nargs;
		for (auto &c : args)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nargs.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nargs.push_back(c->fold());
			}
		}

		return std::make_unique<ExternCallNode>(id, std::move(nargs));
	}
};

class CallBuiltNode : public Node
{
private:
	LEX_TOKEN_TYPE t;
	std::unique_ptr<Node> arg;

public:
	CallBuiltNode(LEX_TOKEN_TYPE t, std::unique_ptr<Node> arg) : Node(CALL_BUILT_NODE), t(t), arg(std::move(arg)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CALL_BUILT"
				  << " : " << std::to_string(t) << "\n"
				  << colorASCII(RESET_TEXT);
		arg->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<CallBuiltNode>(t, arg->fold());
	}
};

class ReturnNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReturnNode(std::unique_ptr<Node> a) : Node(REFER_NODE), a(std::move(a)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "RETURN"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		a->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ReturnNode>(a->fold());
	}
};

class ReferNode : public Node
{
private:
	std::unique_ptr<Node> a;

public:
	ReferNode(std::unique_ptr<Node> a) : Node(RETURN_NODE), a(std::move(a)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return false;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "REFER"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		a->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		return std::make_unique<ReferNode>(a->fold());
	}
};

class IndexNode : public Node
{
private:
	std::unique_ptr<Node> callee;
	std::unique_ptr<Node> arg;

public:
	IndexNode(std::unique_ptr<Node> callee, std::unique_ptr<Node> arg) : Node(INDEX_NODE), callee(std::move(callee)), arg(std::move(arg)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return callee->isConst() && arg->isConst();
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "INDEX"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		callee->printTree(indent, false);
		arg->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		return std::make_unique<IndexNode>(callee->fold(), arg->fold());
	}
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
	bool isConst() const override
	{
		return a->isConst() && b->isConst();
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "BINOP"
				  << " : " << op << "\n"
				  << colorASCII(RESET_TEXT);
		a->printTree(indent, false);
		b->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		return std::make_unique<BinOpNode>(op, a->fold(), b->fold());
	}
};

class UnOpNode : public Node
{
private:
	std::string op;
	std::unique_ptr<Node> a;

public:
	UnOpNode(const std::string &op, std::unique_ptr<Node> a) : Node(UN_OP_NODE), op(op), a(std::move(a)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return a->isConst();
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "UNOP"
				  << " : " << op << "\n"
				  << colorASCII(RESET_TEXT);
		a->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		return std::make_unique<UnOpNode>(op, a->fold());
	}
};

class CastToNode : public Node
{
private:
	D_TYPE convert;
	std::unique_ptr<Node> a;

public:
	CastToNode(D_TYPE convert, std::unique_ptr<Node> a) : Node(CAST_TO_NODE), convert(convert), a(std::move(a)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return a->isConst();
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CAST"
				  << " : " << std::to_string(convert) << "\n"
				  << colorASCII(RESET_TEXT);
		a->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		return std::make_unique<CastToNode>(convert, a->fold());
	}
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
	bool isConst() const override
	{
		return callee->isConst() && arg->isConst();
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "INS"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		callee->printTree(indent, false);
		arg->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		return std::make_unique<InsNode>(callee->fold(), arg->fold());
	}
};

class IfElseNode : public Node
{
private:
	std::unique_ptr<Node> ifs;
	std::vector<std::unique_ptr<Node>> body;
	std::unique_ptr<Node> elses = nullptr;

public:
	IfElseNode(std::unique_ptr<Node> ifs, std::vector<std::unique_ptr<Node>> body) : Node(IF_ELSE_NODE), ifs(std::move(ifs)), body(std::move(body)) {}
	void setElse(std::unique_ptr<Node> elses) { this->elses = std::move(elses); }
	Instruction *genParser() const override;
	bool isConst() const override
	{
		if (!ifs->isConst())
			return false;
		for (auto &c : body)
			if (!c->isConst())
				return false;
		if (elses != nullptr && !elses->isConst())
			return false;
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "IF_ELSE"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		ifs->printTree(indent, false);
		for (size_t i = 0; i < body.size(); i++)
			body[i]->printTree(indent, i == body.size() - 1 && elses == nullptr);
		if (elses != nullptr)
			elses->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::unique_ptr<Node>> nbody;
		for (auto &c : body)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nbody.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nbody.push_back(c->fold());
			}
		}
		auto ret = std::make_unique<IfElseNode>(ifs->fold(), std::move(nbody));
		if (elses)
			ret->setElse(elses->fold());
		return ret;
	}
};

class WhileNode : public Node
{
private:
	std::unique_ptr<Node> whiles;
	std::vector<std::unique_ptr<Node>> body;

public:
	WhileNode(std::unique_ptr<Node> whiles, std::vector<std::unique_ptr<Node>> body) : Node(WHILE_NODE), whiles(std::move(whiles)), body(std::move(body)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		if (!whiles->isConst())
			return false;
		for (auto &c : body)
			if (!c->isConst())
				return false;
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "WHILE"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		whiles->printTree(indent, false);
		for (size_t i = 0; i < body.size(); i++)
			body[i]->printTree(indent, i == body.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::unique_ptr<Node>> nbody;
		for (auto &c : body)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nbody.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nbody.push_back(c->fold());
			}
		}
		return std::make_unique<WhileNode>(whiles->fold(), std::move(nbody));
	}
};

class ForNode : public Node
{
private:
	hashcode_t id;
	std::unique_ptr<Node> fors;
	std::vector<std::unique_ptr<Node>> body;

public:
	ForNode(hashcode_t id, std::unique_ptr<Node> fors, std::vector<std::unique_ptr<Node>> body) : Node(FOR_NODE), id(id), fors(std::move(fors)), body(std::move(body)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		if (!fors->isConst())
			return false;
		for (auto &c : body)
			if (!c->isConst())
				return false;
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "FOR"
				  << " : " << hash.deHash(id) << "\n"
				  << colorASCII(RESET_TEXT);
		fors->printTree(indent, false);
		for (size_t i = 0; i < body.size(); i++)
			body[i]->printTree(indent, i == body.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::unique_ptr<Node>> nbody;
		for (auto &c : body)
		{
			if (c->isConst())
			{
				auto i = c->genParser();
				Scope scope;
				auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
				nbody.push_back(std::move(nn));
				delete i;
			}
			else
			{
				nbody.push_back(c->fold());
			}
		}
		return std::make_unique<ForNode>(id, fors->fold(), std::move(nbody));
	}
};

class UntilNode : public Node
{
private:
	std::unique_ptr<Node> a;
	std::unique_ptr<Node> b;

public:
	UntilNode(std::unique_ptr<Node> a, std::unique_ptr<Node> b) : Node(UNTIL_NODE), a(std::move(a)), b(std::move(b)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		return a->isConst() && b->isConst();
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "UNTIL"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		a->printTree(indent, false);
		b->printTree(indent, true);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		return std::make_unique<UntilNode>(a->fold(), b->fold());
	}
};

class MapNode : public Node
{
private:
	std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>> args;

public:
	MapNode(std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>> args) : Node(MAP_NODE), args(std::move(args)) {}
	Instruction *genParser() const override;
	bool isConst() const override
	{
		for (auto &c : args)
			if (!c.second->isConst())
				return false;
		return true;
	}
	void printTree(std::string indent, bool last) const override
	{
		std::cout << indent;
		if (last)
		{
			std::cout << "└─";
			indent += "  ";
		}
		else
		{
			std::cout << "├─";
			indent += "│ ";
		}
		std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "MAP"
				  << "\n"
				  << colorASCII(RESET_TEXT);
		for (size_t i = 0; i < args.size(); i++)
			args[i].second->printTree(indent, i == args.size() - 1);
	}
	std::unique_ptr<Node> fold() const override
	{
		if (isConst())
		{
			auto i = genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(scope)));
			delete i;
			return nn;
		}

		std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>> nargs;
		for (auto &c : args)
		{
			if (c.second->isConst())
			{
				auto i = c.second->genParser();
				Scope scope;
				nargs.push_back({c.first, std::make_unique<ContainerNode>(i->evaluate(scope))});
				delete i;
			}
			else
			{
				nargs.push_back({c.first, c.second->fold()});
			}
		}
		return std::make_unique<MapNode>(std::move(nargs));
	}
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
	std::unique_ptr<Node> parseTrailingNode(std::unique_ptr<Node>, bool);
	std::unique_ptr<Node> parseInsNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseUntilNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCastToNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseBinOpNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseCallNode(std::unique_ptr<Node>);
	std::unique_ptr<Node> parseIndexNode(std::unique_ptr<Node>);

	std::unique_ptr<Node> logErrorN(const std::string &, Token);
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> logErrorSN(const std::string &, Token);

public:
	NodeParser(std::vector<Token> tokens, std::map<std::string, signed int> bOperators, std::map<std::string, signed int> uOperators, boost::filesystem::path currentDir) : tokens(tokens), bOperators(bOperators), uOperators(uOperators), currentDir(currentDir) {}
	std::unique_ptr<Node> parse();
	static Instruction *genParser(std::unique_ptr<Node>);
};

#endif