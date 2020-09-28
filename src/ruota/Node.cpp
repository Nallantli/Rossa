#include "Node.h"
#include "Parser.h"
#include <iostream>

Node::Node(NODE_TYPE type, Token *token) : type(type), token(token) {}

NODE_TYPE Node::getType() const
{
	return type;
}

Token *Node::getToken() const
{
	return token;
}

//------------------------------------------------------------------------------------------------------

ContainerNode::ContainerNode(
	Symbol s, Token *token) : Node(CONTAINER_NODE, token),
							  s(s)
{
	this->s.setMutable(false);
}

Instruction *ContainerNode::genParser() const
{
	return new Container(s, token);
}

bool ContainerNode::isConst() const
{
	return true;
}

void ContainerNode::printTree(std::string indent, bool last) const
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
std::unique_ptr<Node> ContainerNode::fold() const
{
	return std::make_unique<ContainerNode>(s, token);
}

//------------------------------------------------------------------------------------------------------

VectorNode::VectorNode(
	std::vector<std::unique_ptr<Node>> args,
	bool scoped, Token *token) : Node(VECTOR_NODE, token),
								 args(std::move(args)),
								 scoped(scoped) {}

Instruction *VectorNode::genParser() const
{
	std::vector<Instruction *> ins;
	for (auto &n : args)
		ins.push_back(n->genParser());
	if (scoped)
		return new ScopeI(ins, token);
	return new Sequence(ins, token);
}

bool VectorNode::isConst() const
{
	for (auto &c : args)
		if (!c->isConst())
			return false;
	return true;
}

void VectorNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "VECTOR : " << scoped
			  << "\n"
			  << colorASCII(RESET_TEXT);

	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

std::unique_ptr<Node> VectorNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
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
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
			nargs.push_back(std::move(nn));
			delete i;
		}
		else
		{
			nargs.push_back(c->fold());
		}
	}
	return std::make_unique<VectorNode>(std::move(nargs), scoped, token);
}

std::vector<std::unique_ptr<Node>> VectorNode::getChildren()
{
	return std::move(args);
}

//------------------------------------------------------------------------------------------------------

/*NumNode::NumNode(
	NUMBER_TYPE numberValue, Token * token) : Node(NUM_NODE, token),
							   numberValue(numberValue) {}

Instruction *NumNode::genParser() const
{
	return new Container(Symbol(numberValue, token));
}

bool NumNode::isConst() const
{
	return true;
}

void NumNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "NUMBER : " << NUMBER_STRING(numberValue) << "\n"
			  << colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> NumNode::fold() const
{
	return std::make_unique<ContainerNode>(Symbol(numberValue), token);
}*/

//------------------------------------------------------------------------------------------------------

/*NilNode::NilNode(, Token * token) : Node(NIL_NODE, token) {}

Instruction *NilNode::genParser() const
{
	return new Container(Symbol(, token));
}

bool NilNode::isConst() const
{
	return true;
}

void NilNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "NIL : nil\n"
			  << colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> NilNode::fold() const
{
	return std::make_unique<ContainerNode>(Symbol(), token);
}*/

//------------------------------------------------------------------------------------------------------

BreakNode::BreakNode(Token *token) : Node(BREAK_NODE, token) {}

Instruction *BreakNode::genParser() const
{
	return new Container(Symbol(ID_BREAK), token);
}

bool BreakNode::isConst() const
{
	return true;
}

void BreakNode::printTree(std::string indent, bool last) const
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

std::unique_ptr<Node> BreakNode::fold() const
{
	return std::make_unique<ContainerNode>(Symbol(ID_BREAK), token);
}

//------------------------------------------------------------------------------------------------------

/*StringNode::StringNode(
	const std::string &stringValue, Token * token) : Node(STRING_NODE, token),
									  stringValue(stringValue) {}

Instruction *StringNode::genParser() const
{
	return new Container(Symbol(stringValue, token));
}

bool StringNode::isConst() const
{
	return true;
}

void StringNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "STRING : " << stringValue << "\n"
			  << colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> StringNode::fold() const
{
	return std::make_unique<ContainerNode>(Symbol(stringValue), token);
}*/

//------------------------------------------------------------------------------------------------------

/*BoolNode::BoolNode(
	bool boolValue, Token * token) : Node(BOOL_NODE, token),
					  boolValue(boolValue) {}

Instruction *BoolNode::genParser() const
{
	return new Container(Symbol(boolValue, token));
}

bool BoolNode::isConst() const
{
	return true;
}

void BoolNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "BOOL : " << (boolValue ? "true" : "false") << "\n"
			  << colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> BoolNode::fold() const
{
	return std::make_unique<ContainerNode>(Symbol(boolValue), token);
}*/

//------------------------------------------------------------------------------------------------------

IDNode::IDNode(
	hashcode_t key, Token *token) : Node(ID_NODE, token),
									key(key) {}

hashcode_t IDNode::getKey() const
{
	return key;
}

Instruction *IDNode::genParser() const
{
	return new VariableI(key, token);
}

bool IDNode::isConst() const
{
	return false;
}

void IDNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "ID : " << hash.deHash(key) << "\n"
			  << colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> IDNode::fold() const
{
	return std::make_unique<IDNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

BIDNode::BIDNode(
	const std::string &key, Token *token) : Node(BID_NODE, token),
											key(key) {}

const std::string BIDNode::getKey() const
{
	return key;
}

Instruction *BIDNode::genParser() const
{
	return new VariableI(hash.hashString(key), token);
}

bool BIDNode::isConst() const
{
	return false;
}

void BIDNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "BID : " << key << "\n"
			  << colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> BIDNode::fold() const
{
	return std::make_unique<BIDNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

DefineNode::DefineNode(
	hashcode_t key,
	D_TYPE ftype,
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params,
	std::vector<std::unique_ptr<Node>> body, Token *token) : Node(DEFINE_NODE, token),
															 key(key),
															 ftype(ftype),
															 params(params),
															 body(std::move(body)) {}

Instruction *DefineNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto fbody = std::make_shared<ScopeI>(is, token);
	return new DefineI(key, ftype, params, fbody, token);
}

bool DefineNode::isConst() const
{
	return false;
}

void DefineNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "DEFINE : " << (key > 0 ? hash.deHash(key) : "<LAMBDA>") << ", " << std::to_string(ftype) << "\n"
			  << colorASCII(RESET_TEXT);

	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

std::unique_ptr<Node> DefineNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
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
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
			nbody.push_back(std::move(nn));
			delete i;
		}
		else
		{
			nbody.push_back(c->fold());
		}
	}
	return std::make_unique<DefineNode>(key, ftype, params, std::move(nbody), token);
}

//------------------------------------------------------------------------------------------------------

NewNode::NewNode(
	std::unique_ptr<Node> object,
	std::unique_ptr<Node> params, Token *token) : Node(NEW_NODE, token),
												  object(std::move(object)),
												  params(std::move(params)) {}

Instruction *NewNode::genParser() const
{
	auto paramsI = params->genParser();
	return new NewI(object->genParser(), paramsI, token);
}

bool NewNode::isConst() const
{
	return false;
}

void NewNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "NEW\n"
			  << colorASCII(RESET_TEXT);

	object->printTree(indent, false);
	params->printTree(indent, true);
}

std::unique_ptr<Node> NewNode::fold() const
{
	return std::make_unique<NewNode>(object->fold(), params->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ClassNode::ClassNode(
	hashcode_t key,
	int type,
	std::vector<std::unique_ptr<Node>> body,
	std::unique_ptr<Node> extends, Token *token) : Node(CLASS_NODE, token),
												   key(key),
												   type(type),
												   body(std::move(body)),
												   extends(std::move(extends)) {}

Instruction *ClassNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = std::make_shared<ScopeI>(is, token);

	OBJECT_TYPE ot;
	switch (type)
	{
	case TOK_STRUCT:
		ot = STRUCT_O;
		break;
	case TOK_STATIC:
		ot = STATIC_O;
		break;
	case TOK_VIRTUAL:
		ot = VIRTUAL_O;
		break;
	default:
		throwError("Invalid Object type", token);
	}

	if (extends == nullptr)
		return new ClassI(key, ot, bodyI, NULL, token);
	else
		return new ClassI(key, ot, bodyI, extends->genParser(), token);
}

bool ClassNode::isConst() const
{
	return false;
}

void ClassNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CLASS : " << hash.deHash(key) << ", " << std::to_string(type) << "\n"
			  << colorASCII(RESET_TEXT);
	if (extends != nullptr)
		extends->printTree(indent, false);
	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

std::unique_ptr<Node> ClassNode::fold() const
{
	std::vector<std::unique_ptr<Node>> nbody;
	for (auto &c : body)
	{
		if (c->isConst())
		{
			auto i = c->genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
			nbody.push_back(std::move(nn));
			delete i;
		}
		else
		{
			nbody.push_back(c->fold());
		}
	}

	if (extends)
		return std::make_unique<ClassNode>(key, type, std::move(nbody), extends->fold(), token);
	else
		return std::make_unique<ClassNode>(key, type, std::move(nbody), nullptr, token);
}

//------------------------------------------------------------------------------------------------------

VarNode::VarNode(
	hashcode_t key, Token *token) : Node(VAR_NODE, token),
									key(key) {}

Instruction *VarNode::genParser() const
{
	return new DeclareI(key, token);
}

bool VarNode::isConst() const
{
	return false;
}

void VarNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "VAR : " << hash.deHash(key) << "\n"
			  << colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> VarNode::fold() const
{
	return std::make_unique<VarNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

CallNode::CallNode(
	std::unique_ptr<Node> callee,
	std::vector<std::unique_ptr<Node>> args, Token *token) : Node(CALL_NODE, token),
															 callee(std::move(callee)),
															 args(std::move(args)) {}

Instruction *CallNode::genParser() const
{
	auto fcallee = callee->genParser();
	std::vector<Instruction *> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return new CallI(fcallee, new Sequence(fargs, token), token);
}

std::unique_ptr<Node> CallNode::getCallee()
{
	return std::move(callee);
}

std::vector<std::unique_ptr<Node>> CallNode::getArgs()
{
	return std::move(args);
}

bool CallNode::isConst() const
{
	return false;
}

void CallNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CALL\n"
			  << colorASCII(RESET_TEXT);

	callee->printTree(indent, args.empty());
	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

std::unique_ptr<Node> CallNode::fold() const
{
	std::vector<std::unique_ptr<Node>> nargs;
	for (auto &c : args)
	{
		if (c->isConst())
		{
			auto i = c->genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
			nargs.push_back(std::move(nn));
			delete i;
		}
		else
		{
			nargs.push_back(c->fold());
		}
	}

	return std::make_unique<CallNode>(callee->fold(), std::move(nargs), token);
}

//------------------------------------------------------------------------------------------------------

ExternCallNode::ExternCallNode(
	const std::string &id,
	std::vector<std::unique_ptr<Node>> args, Token *token) : Node(EXTERN_CALL_NODE, token),
															 id(id),
															 args(std::move(args)) {}

Instruction *ExternCallNode::genParser() const
{
	std::vector<Instruction *> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return new ExternI(id, new Sequence(fargs, token), token);
}

bool ExternCallNode::isConst() const
{
	return false;
}

void ExternCallNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "EXTERN_CALL : " << id << "\n"
			  << colorASCII(RESET_TEXT);
	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

std::unique_ptr<Node> ExternCallNode::fold() const
{
	std::vector<std::unique_ptr<Node>> nargs;
	for (auto &c : args)
	{
		if (c->isConst())
		{
			auto i = c->genParser();
			Scope scope;
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
			nargs.push_back(std::move(nn));
			delete i;
		}
		else
		{
			nargs.push_back(c->fold());
		}
	}

	return std::make_unique<ExternCallNode>(id, std::move(nargs), token);
}

//------------------------------------------------------------------------------------------------------

CallBuiltNode::CallBuiltNode(
	LEX_TOKEN_TYPE t,
	std::unique_ptr<Node> arg, Token *token) : Node(CALL_BUILT_NODE, token),
											   t(t),
											   arg(std::move(arg)) {}

Instruction *CallBuiltNode::genParser() const
{
	switch (t)
	{
	case TOK_SIZE:
		return new SizeI(arg->genParser(), token);
	case TOK_LENGTH:
		return new LengthI(arg->genParser(), token);
	case TOK_ALLOC:
		return new AllocI(arg->genParser(), token);
	case TOK_CHARN:
		return new CharNI(arg->genParser(), token);
	case TOK_CHARS:
		return new CharSI(arg->genParser(), token);
	default:
		break;
	}

	throwError("Unknown built in function: " + std::to_string(t), token);
	return NULL;
}

bool CallBuiltNode::isConst() const
{
	return arg->isConst();
}

void CallBuiltNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CALL_BUILT : " << std::to_string(t) << "\n"
			  << colorASCII(RESET_TEXT);

	arg->printTree(indent, true);
}

std::unique_ptr<Node> CallBuiltNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	return std::make_unique<CallBuiltNode>(t, arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ReturnNode::ReturnNode(
	std::unique_ptr<Node> a, Token *token) : Node(REFER_NODE, token),
											 a(std::move(a)) {}

Instruction *ReturnNode::genParser() const
{
	return new ReturnI(a->genParser(), token);
}

bool ReturnNode::isConst() const
{
	return false;
}

void ReturnNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "RETURN\n"
			  << colorASCII(RESET_TEXT);

	a->printTree(indent, true);
}

std::unique_ptr<Node> ReturnNode::fold() const
{
	return std::make_unique<ReturnNode>(a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ReferNode::ReferNode(
	std::unique_ptr<Node> a, Token *token) : Node(RETURN_NODE, token),
											 a(std::move(a)) {}

Instruction *ReferNode::genParser() const
{
	return new ReferI(a->genParser(), token);
}

bool ReferNode::isConst() const
{
	return false;
}

void ReferNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "REFER\n"
			  << colorASCII(RESET_TEXT);
	a->printTree(indent, true);
}

std::unique_ptr<Node> ReferNode::fold() const
{
	return std::make_unique<ReferNode>(a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

IndexNode::IndexNode(
	std::unique_ptr<Node> callee,
	std::unique_ptr<Node> arg, Token *token) : Node(INDEX_NODE, token),
											   callee(std::move(callee)),
											   arg(std::move(arg)) {}

Instruction *IndexNode::genParser() const
{
	return new IndexI(callee->genParser(), arg->genParser(), token);
}

bool IndexNode::isConst() const
{
	return callee->isConst() && arg->isConst();
}

void IndexNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "INDEX\n"
			  << colorASCII(RESET_TEXT);
	callee->printTree(indent, false);
	arg->printTree(indent, true);
}

std::unique_ptr<Node> IndexNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	return std::make_unique<IndexNode>(callee->fold(), arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

BinOpNode::BinOpNode(
	const std::string &op,
	std::unique_ptr<Node> a,
	std::unique_ptr<Node> b, Token *token) : Node(BIN_OP_NODE, token),
											 op(op),
											 a(std::move(a)),
											 b(std::move(b)) {}

Instruction *BinOpNode::genParser() const
{
	if (op == "+")
		return new AddI(a->genParser(), b->genParser(), token);
	if (op == "-")
		return new SubI(a->genParser(), b->genParser(), token);
	if (op == "*")
		return new MulI(a->genParser(), b->genParser(), token);
	if (op == "/")
		return new DivI(a->genParser(), b->genParser(), token);
	if (op == "%")
		return new ModI(a->genParser(), b->genParser(), token);
	if (op == "**")
		return new PowI(a->genParser(), b->genParser(), token);
	if (op == "|")
		return new BOrI(a->genParser(), b->genParser(), token);
	if (op == "&")
		return new BAndI(a->genParser(), b->genParser(), token);
	if (op == "^")
		return new BXOrI(a->genParser(), b->genParser(), token);
	if (op == "<<")
		return new BShiftLeft(a->genParser(), b->genParser(), token);
	if (op == ">>")
		return new BShiftRight(a->genParser(), b->genParser(), token);

	if (op == "+=")
		return new SetI(a->genParser(), new AddI(a->genParser(), b->genParser(), token), token);
	if (op == "-=")
		return new SetI(a->genParser(), new SubI(a->genParser(), b->genParser(), token), token);
	if (op == "*=")
		return new SetI(a->genParser(), new MulI(a->genParser(), b->genParser(), token), token);
	if (op == "/=")
		return new SetI(a->genParser(), new DivI(a->genParser(), b->genParser(), token), token);
	if (op == "%=")
		return new SetI(a->genParser(), new ModI(a->genParser(), b->genParser(), token), token);
	if (op == "**=")
		return new SetI(a->genParser(), new PowI(a->genParser(), b->genParser(), token), token);
	if (op == "|=")
		return new SetI(a->genParser(), new BOrI(a->genParser(), b->genParser(), token), token);
	if (op == "&=")
		return new SetI(a->genParser(), new BAndI(a->genParser(), b->genParser(), token), token);
	if (op == "^=")
		return new SetI(a->genParser(), new BXOrI(a->genParser(), b->genParser(), token), token);
	if (op == "<<=")
		return new SetI(a->genParser(), new BShiftLeft(a->genParser(), b->genParser(), token), token);
	if (op == ">>=")
		return new SetI(a->genParser(), new BShiftRight(a->genParser(), b->genParser(), token), token);

	if (op == "=")
		return new SetI(a->genParser(), b->genParser(), token);

	if (op == "<")
		return new LessI(a->genParser(), b->genParser(), token);
	if (op == ">")
		return new MoreI(a->genParser(), b->genParser(), token);
	if (op == "<=")
		return new ELessI(a->genParser(), b->genParser(), token);
	if (op == ">=")
		return new EMoreI(a->genParser(), b->genParser(), token);
	if (op == "==")
		return new Equals(a->genParser(), b->genParser(), token);
	if (op == "!=")
		return new NEquals(a->genParser(), b->genParser(), token);
	if (op == "===")
		return new PureEquals(a->genParser(), b->genParser(), token);
	if (op == "!==")
		return new PureNEquals(a->genParser(), b->genParser(), token);
	if (op == "&&")
		return new AndI(a->genParser(), b->genParser(), token);
	if (op == "||")
		return new OrI(a->genParser(), b->genParser(), token);

	throwError("Unknown binary operator: " + op, token);
	return NULL;
}

const std::string &BinOpNode::getOp() const
{
	return op;
}

std::unique_ptr<Node> BinOpNode::getA()
{
	return std::move(a);
};

std::unique_ptr<Node> BinOpNode::getB()
{
	return std::move(b);
};

bool BinOpNode::isConst() const
{
	return a->isConst() && b->isConst();
}

void BinOpNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "BINOP : " << op << "\n"
			  << colorASCII(RESET_TEXT);
	a->printTree(indent, false);
	b->printTree(indent, true);
}

std::unique_ptr<Node> BinOpNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	return std::make_unique<BinOpNode>(op, a->fold(), b->fold(), token);
}

//------------------------------------------------------------------------------------------------------

UnOpNode::UnOpNode(
	const std::string &op,
	std::unique_ptr<Node> a, Token *token) : Node(UN_OP_NODE, token),
											 op(op),
											 a(std::move(a)) {}

Instruction *UnOpNode::genParser() const
{
	if (op == "+")
		return a->genParser();
	if (op == "-")
		return new SubI(std::make_unique<ContainerNode>(Symbol(NUMBER_NEW_LONG(0)), token)->genParser(), a->genParser(), token);
	if (op == "!")
		return new Equals(std::make_unique<ContainerNode>(Symbol(false), token)->genParser(), a->genParser(), token);

	throwError("Unknown unary operator: " + op, token);
	return NULL;
}

bool UnOpNode::isConst() const
{
	return a->isConst();
}

void UnOpNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "UNOP : " << op << "\n"
			  << colorASCII(RESET_TEXT);
	a->printTree(indent, true);
}

std::unique_ptr<Node> UnOpNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	return std::make_unique<UnOpNode>(op, a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

CastToNode::CastToNode(
	D_TYPE convert,
	std::unique_ptr<Node> a, Token *token) : Node(CAST_TO_NODE, token),
											 convert(convert),
											 a(std::move(a)) {}

Instruction *CastToNode::genParser() const
{
	return new CastToI(a->genParser(), convert, token);
}

bool CastToNode::isConst() const
{
	return a->isConst();
}

void CastToNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CAST : " << std::to_string(convert) << "\n"
			  << colorASCII(RESET_TEXT);
	a->printTree(indent, true);
}

std::unique_ptr<Node> CastToNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	return std::make_unique<CastToNode>(convert, a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

InsNode::InsNode(
	std::unique_ptr<Node> callee,
	std::unique_ptr<Node> arg, Token *token) : Node(INS_NODE, token),
											   callee(std::move(callee)),
											   arg(std::move(arg)) {}

Instruction *InsNode::genParser() const
{
	return new InnerI(callee->genParser(), arg->genParser(), token);
}

std::unique_ptr<Node> InsNode::getCallee()
{
	return std::move(callee);
}

std::unique_ptr<Node> InsNode::getArg()
{
	return std::move(arg);
}

bool InsNode::isConst() const
{
	return callee->isConst() && arg->isConst();
}
void InsNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "INS\n"
			  << colorASCII(RESET_TEXT);

	callee->printTree(indent, false);
	arg->printTree(indent, true);
}

std::unique_ptr<Node> InsNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	return std::make_unique<InsNode>(callee->fold(), arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

IfElseNode::IfElseNode(
	std::unique_ptr<Node> ifs,
	std::unique_ptr<Node> body, Token *token) : Node(IF_ELSE_NODE, token),
												ifs(std::move(ifs)),
												body(std::move(body)) {}

void IfElseNode::setElse(std::unique_ptr<Node> elses)
{
	this->elses = std::move(elses);
}

Instruction *IfElseNode::genParser() const
{
	if (elses)
		return new IfElseI(ifs->genParser(), body->genParser(), elses->genParser(), token);
	return new IfElseI(ifs->genParser(), body->genParser(), NULL, token);
}

bool IfElseNode::isConst() const
{
	if (!ifs->isConst())
		return false;
	if (!body->isConst())
		return false;
	if (elses != nullptr && !elses->isConst())
		return false;
	return true;
}

void IfElseNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "IF_ELSE\n"
			  << colorASCII(RESET_TEXT);

	ifs->printTree(indent, false);
	body->printTree(indent, elses == nullptr);
	if (elses != nullptr)
		elses->printTree(indent, true);
}

std::unique_ptr<Node> IfElseNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	auto ret = std::make_unique<IfElseNode>(ifs->fold(), body->fold(), token);
	if (elses)
		ret->setElse(elses->fold());
	return ret;
}

//------------------------------------------------------------------------------------------------------

WhileNode::WhileNode(
	std::unique_ptr<Node> whiles,
	std::vector<std::unique_ptr<Node>> body, Token *token) : Node(WHILE_NODE, token),
															 whiles(std::move(whiles)),
															 body(std::move(body)) {}

Instruction *WhileNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = new ScopeI(is, token);

	return new WhileI(whiles->genParser(), bodyI, token);
}

bool WhileNode::isConst() const
{
	if (!whiles->isConst())
		return false;
	for (auto &c : body)
		if (!c->isConst())
			return false;
	return true;
}

void WhileNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "WHILE\n"
			  << colorASCII(RESET_TEXT);
	whiles->printTree(indent, false);
	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

std::unique_ptr<Node> WhileNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
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
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
			nbody.push_back(std::move(nn));
			delete i;
		}
		else
		{
			nbody.push_back(c->fold());
		}
	}
	return std::make_unique<WhileNode>(whiles->fold(), std::move(nbody), token);
}

//------------------------------------------------------------------------------------------------------

ForNode::ForNode(
	hashcode_t id,
	std::unique_ptr<Node> fors,
	std::vector<std::unique_ptr<Node>> body, Token *token) : Node(FOR_NODE, token),
															 id(id),
															 fors(std::move(fors)),
															 body(std::move(body)) {}

Instruction *ForNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = new ScopeI(is, token);

	return new ForI(id, fors->genParser(), bodyI, token);
}

bool ForNode::isConst() const
{
	if (!fors->isConst())
		return false;
	for (auto &c : body)
		if (!c->isConst())
			return false;
	return true;
}

void ForNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "FOR : " << hash.deHash(id) << "\n"
			  << colorASCII(RESET_TEXT);
	fors->printTree(indent, false);
	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

std::unique_ptr<Node> ForNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
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
			auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
			nbody.push_back(std::move(nn));
			delete i;
		}
		else
		{
			nbody.push_back(c->fold());
		}
	}
	return std::make_unique<ForNode>(id, fors->fold(), std::move(nbody), token);
}

//------------------------------------------------------------------------------------------------------

UntilNode::UntilNode(
	std::unique_ptr<Node> a,
	std::unique_ptr<Node> b, Token *token) : Node(UNTIL_NODE, token),
											 a(std::move(a)),
											 b(std::move(b)) {}

Instruction *UntilNode::genParser() const
{
	return new UntilI(a->genParser(), b->genParser(), token);
}

bool UntilNode::isConst() const
{
	return a->isConst() && b->isConst();
}

void UntilNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "UNTIL\n"
			  << colorASCII(RESET_TEXT);

	a->printTree(indent, false);
	b->printTree(indent, true);
}

std::unique_ptr<Node> UntilNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	return std::make_unique<UntilNode>(a->fold(), b->fold(), token);
}

//------------------------------------------------------------------------------------------------------

MapNode::MapNode(
	std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>> args, Token *token) : Node(MAP_NODE, token),
																					args(std::move(args)) {}

Instruction *MapNode::genParser() const
{
	std::map<hashcode_t, Instruction *> is;
	for (auto &e : this->args)
	{
		is[e.first] = e.second->genParser();
	}
	return new MapI(is, token);
}

bool MapNode::isConst() const
{
	for (auto &c : args)
		if (!c.second->isConst())
			return false;
	return true;
}

void MapNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "MAP\n"
			  << colorASCII(RESET_TEXT);

	for (size_t i = 0; i < args.size(); i++)
		args[i].second->printTree(indent, i == args.size() - 1);
}

std::unique_ptr<Node> MapNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
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
			nargs.push_back({c.first, std::make_unique<ContainerNode>(i->evaluate(&scope), token)});
			delete i;
		}
		else
		{
			nargs.push_back({c.first, c.second->fold()});
		}
	}
	return std::make_unique<MapNode>(std::move(nargs), token);
}

//------------------------------------------------------------------------------------------------------

SwitchNode::SwitchNode(
	std::unique_ptr<Node> switchs,
	std::map<Symbol, std::unique_ptr<Node>> cases, Token *token) : Node(SWITCH_NODE, token),
																   switchs(std::move(switchs)),
																   cases(std::move(cases)) {}

Instruction *SwitchNode::genParser() const
{
	std::map<Symbol, Instruction *> is;
	for (auto &e : this->cases)
	{
		is[e.first] = e.second->genParser();
	}
	if (elses)
		return new SwitchI(switchs->genParser(), is, elses->genParser(), token);
	return new SwitchI(switchs->genParser(), is, NULL, token);
}

void SwitchNode::setElse(std::unique_ptr<Node> elses)
{
	this->elses = std::move(elses);
}

bool SwitchNode::isConst() const
{
	return false;
}

void SwitchNode::printTree(std::string indent, bool last) const
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
	switchs->printTree(indent, cases.empty());
	size_t i = 0;
	for (auto &e : cases)
	{
		e.second->printTree(indent, i == cases.size() - 1 && !elses);
		i++;
	}
	if (elses)
		elses->printTree(indent, true);
}

std::unique_ptr<Node> SwitchNode::fold() const
{
	if (isConst())
	{
		auto i = genParser();
		Scope scope;
		auto nn = std::unique_ptr<Node>(new ContainerNode(i->evaluate(&scope), token));
		delete i;
		return nn;
	}

	std::map<Symbol, std::unique_ptr<Node>> ncases;
	for (auto &c : cases)
	{
		if (c.second->isConst())
		{
			auto i = c.second->genParser();
			Scope scope;
			ncases[c.first] = std::make_unique<ContainerNode>(i->evaluate(&scope), token);
			delete i;
		}
		else
		{
			ncases[c.first] = c.second->fold();
		}
	}
	auto ret = std::make_unique<SwitchNode>(switchs->fold(), std::move(ncases), token);
	if (elses)
		ret->setElse(elses->fold());
	return ret;
}

//------------------------------------------------------------------------------------------------------

TryCatchNode::TryCatchNode(
	std::unique_ptr<Node> trys,
	std::unique_ptr<Node> catchs,
	hashcode_t key, Token *token) : Node(TRY_CATCH_NODE, token),
									trys(std::move(trys)),
									catchs(std::move(catchs)),
									key(key) {}

Instruction *TryCatchNode::genParser() const
{
	return new TryCatchI(trys->genParser(), catchs->genParser(), key, token);
}

bool TryCatchNode::isConst() const
{
	return false;
}

void TryCatchNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "TRY_CATCH : " << hash.deHash(key) << "\n"
			  << colorASCII(RESET_TEXT);

	trys->printTree(indent, false);
	catchs->printTree(indent, true);
}

std::unique_ptr<Node> TryCatchNode::fold() const
{
	return std::make_unique<TryCatchNode>(trys->fold(), catchs->fold(), key, token);
}

//------------------------------------------------------------------------------------------------------

ThrowNode::ThrowNode(
	std::unique_ptr<Node> throws, Token *token) : Node(THROW_NODE, token),
												  throws(std::move(throws)) {}

Instruction *ThrowNode::genParser() const
{
	return new ThrowI(throws->genParser(), token);
}

bool ThrowNode::isConst() const
{
	return false;
}

void ThrowNode::printTree(std::string indent, bool last) const
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
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "THROW\n"
			  << colorASCII(RESET_TEXT);

	throws->printTree(indent, true);
}

std::unique_ptr<Node> ThrowNode::fold() const
{
	return std::make_unique<ThrowNode>(throws->fold(), token);
}