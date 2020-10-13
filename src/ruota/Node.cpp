#include "Node.h"
#include "Parser.h"
#include <iostream>

Node::Node(
	const NodeType &type,
	const Token &token) : type(type),
	token(token)
{}

const NodeType Node::getType() const
{
	return type;
}

const Token Node::getToken() const
{
	return token;
}

//------------------------------------------------------------------------------------------------------

ContainerNode::ContainerNode(
	const Symbol &s,
	const Token &token) : Node(CONTAINER_NODE,
		token),
	s(s)
{}

std::shared_ptr<Instruction> ContainerNode::genParser() const
{
	return std::make_shared<ContainerI>(s, token);
}

bool ContainerNode::isConst() const
{
	return true;
}

void ContainerNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	bool scoped,
	const Token &token) : Node(VECTOR_NODE,
		token),
	args(std::move(args)),
	scoped(scoped)
{}

std::shared_ptr<Instruction> VectorNode::genParser() const
{
	std::vector<std::shared_ptr<Instruction>> ins;
	for (auto &n : args)
		ins.push_back(n->genParser());
	if (scoped)
		return std::make_shared<ScopeI>(ins, token);
	return std::make_shared<SequenceI>(ins, token);
}

bool VectorNode::isConst() const
{
	for (auto &c : args)
		if (!c->isConst())
			return false;
	return true;
}

void VectorNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "ARRAY : " << scoped
		<< "\n"
		<< colorASCII(RESET_TEXT);

	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

std::unique_ptr<Node> VectorNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	std::vector<std::unique_ptr<Node>> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold());
	return std::make_unique<VectorNode>(std::move(nargs), scoped, token);
}

const std::vector<std::unique_ptr<Node>> &VectorNode::getChildren()
{
	return std::move(args);
}

//------------------------------------------------------------------------------------------------------

BreakNode::BreakNode(
	const Token &token) : Node(BREAK_NODE,
		token)
{}

std::shared_ptr<Instruction> BreakNode::genParser() const
{
	return std::make_shared<ContainerI>(Symbol(ID_BREAK), token);
}

bool BreakNode::isConst() const
{
	return true;
}

void BreakNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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

IDNode::IDNode(
	hash_ull key,
	const Token &token) : Node(ID_NODE,
		token),
	key(key)
{}

hash_ull IDNode::getKey() const
{
	return key;
}

std::shared_ptr<Instruction> IDNode::genParser() const
{
	return std::make_shared<VariableI>(key, token);
}

bool IDNode::isConst() const
{
	return false;
}

void IDNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "ID : " << MAIN_HASH.deHash(key) << "\n"
		<< colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> IDNode::fold() const
{
	return std::make_unique<IDNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

BIDNode::BIDNode(
	const string &key,
	const Token &token) : Node(BID_NODE,
		token),
	key(key)
{}

const string BIDNode::getKey() const
{
	return key;
}

std::shared_ptr<Instruction> BIDNode::genParser() const
{
	return std::make_shared<VariableI>(MAIN_HASH.hashString(key), token);
}

bool BIDNode::isConst() const
{
	return false;
}

void BIDNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	hash_ull key,
	Signature ftype,
	std::vector<std::pair<LexerTokenType, hash_ull>> params,
	std::unique_ptr<Node> body,
	const Token &token) : Node(DEFINE_NODE,
		token),
	key(key),
	ftype(ftype),
	params(params),
	body(std::move(body))
{}

std::shared_ptr<Instruction> DefineNode::genParser() const
{
	return std::make_shared<DefineI>(key, ftype, params, body->genParser(), token);
}

bool DefineNode::isConst() const
{
	return false;
}

void DefineNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "DEFINE : " << (key > 0 ? MAIN_HASH.deHash(key) : "<LAMBDA>") << ", " << ftype.values.size() << "\n"
		<< colorASCII(RESET_TEXT);

	body->printTree(indent, true);
}

std::unique_ptr<Node> DefineNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	return std::make_unique<DefineNode>(key, ftype, params, body->fold(), token);
}

//------------------------------------------------------------------------------------------------------

NewNode::NewNode(
	std::unique_ptr<Node> object,
	std::unique_ptr<Node> params,
	const Token &token) : Node(NEW_NODE,
		token),
	object(std::move(object)),
	params(std::move(params))
{}

std::shared_ptr<Instruction> NewNode::genParser() const
{
	auto paramsI = params->genParser();
	return std::make_shared<NewI>(object->genParser(), paramsI, token);
}

bool NewNode::isConst() const
{
	return false;
}

void NewNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	hash_ull key,
	int type,
	std::vector<std::unique_ptr<Node>> body,
	std::unique_ptr<Node> extends,
	const Token &token) : Node(CLASS_NODE,
		token),
	key(key),
	type(type),
	body(std::move(body)),
	extends(std::move(extends))
{}

std::shared_ptr<Instruction> ClassNode::genParser() const
{
	std::vector<std::shared_ptr<Instruction>> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = std::make_shared<ScopeI>(is, token);

	ObjectType ot;
	switch (type) {
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
			throw RuotaError(_INVALID_OBJECT_TYPE_, token);
	}

	if (extends == nullptr)
		return std::make_shared<ClassI>(key, ot, bodyI, nullptr, token);
	else
		return std::make_shared<ClassI>(key, ot, bodyI, extends->genParser(), token);
}

bool ClassNode::isConst() const
{
	return false;
}

void ClassNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CLASS : " << MAIN_HASH.deHash(key) << ", " << std::to_string(type) << "\n"
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
		nbody.push_back(c->fold());

	if (extends)
		return std::make_unique<ClassNode>(key, type, std::move(nbody), extends->fold(), token);
	else
		return std::make_unique<ClassNode>(key, type, std::move(nbody), nullptr, token);
}

//------------------------------------------------------------------------------------------------------

VarNode::VarNode(
	std::vector<hash_ull> keys,
	const Token &token) : Node(VAR_NODE,
		token),
	keys(keys)
{}

std::shared_ptr<Instruction> VarNode::genParser() const
{
	return std::make_shared<DeclareVarsI>(keys, token);
}

bool VarNode::isConst() const
{
	return false;
}

void VarNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "VAR : " << keys.size() << "\n"
		<< colorASCII(RESET_TEXT);
}

std::unique_ptr<Node> VarNode::fold() const
{
	return std::make_unique<VarNode>(keys, token);
}

//------------------------------------------------------------------------------------------------------

CallNode::CallNode(
	std::unique_ptr<Node> callee,
	std::vector<std::unique_ptr<Node>> args,
	const Token &token) : Node(CALL_NODE,
		token),
	callee(std::move(callee)),
	args(std::move(args))
{}

std::shared_ptr<Instruction> CallNode::genParser() const
{
	auto fcallee = callee->genParser();
	std::vector<std::shared_ptr<Instruction>> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<CallI>(fcallee, std::make_shared<SequenceI>(fargs, token), token);
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

void CallNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
		nargs.push_back(c->fold());

	return std::make_unique<CallNode>(callee->fold(), std::move(nargs), token);
}

//------------------------------------------------------------------------------------------------------

ExternCallNode::ExternCallNode(
	const string &id,
	std::vector<std::unique_ptr<Node>> args,
	const Token &token) : Node(EXTERN_CALL_NODE,
		token),
	id(id),
	args(std::move(args))
{}

std::shared_ptr<Instruction> ExternCallNode::genParser() const
{
	std::vector<std::shared_ptr<Instruction>> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<ExternI>(id, std::make_shared<SequenceI>(fargs, token), token);
}

bool ExternCallNode::isConst() const
{
	return false;
}

void ExternCallNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
		nargs.push_back(c->fold());

	return std::make_unique<ExternCallNode>(id, std::move(nargs), token);
}

//------------------------------------------------------------------------------------------------------

CallBuiltNode::CallBuiltNode(
	LexerTokenType t,
	std::unique_ptr<Node> arg,
	const Token &token) : Node(CALL_BUILT_NODE,
		token),
	t(t),
	arg(std::move(arg))
{}

std::shared_ptr<Instruction> CallBuiltNode::genParser() const
{
	switch (t) {
		case TOK_SIZE:
			return std::make_shared<SizeI>(arg->genParser(), token);
		case TOK_LENGTH:
			return std::make_shared<LengthI>(arg->genParser(), token);
		case TOK_ALLOC:
			return std::make_shared<AllocI>(arg->genParser(), token);
		case TOK_CHARN:
			return std::make_shared<CharNI>(arg->genParser(), token);
		case TOK_CHARS:
			return std::make_shared<CharSI>(arg->genParser(), token);
		default:
			break;
	}

	throw RuotaError((boost::format(_UNKNOWN_BUILT_CALL_) % t).str(), token);
	return nullptr;
}

bool CallBuiltNode::isConst() const
{
	return arg->isConst();
}

void CallBuiltNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CALL_BUILT : " << std::to_string(t) << "\n"
		<< colorASCII(RESET_TEXT);

	arg->printTree(indent, true);
}

std::unique_ptr<Node> CallBuiltNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	return std::make_unique<CallBuiltNode>(t, arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ReturnNode::ReturnNode(
	std::unique_ptr<Node> a,
	const Token &token) : Node(REFER_NODE,
		token),
	a(std::move(a))
{}

std::shared_ptr<Instruction> ReturnNode::genParser() const
{
	return std::make_shared<ReturnI>(a->genParser(), token);
}

bool ReturnNode::isConst() const
{
	return false;
}

void ReturnNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	std::unique_ptr<Node> a,
	const Token &token) : Node(RETURN_NODE,
		token),
	a(std::move(a))
{}

std::shared_ptr<Instruction> ReferNode::genParser() const
{
	return std::make_shared<ReferI>(a->genParser(), token);
}

bool ReferNode::isConst() const
{
	return false;
}

void ReferNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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

BinOpNode::BinOpNode(
	const string &op,
	std::unique_ptr<Node> a,
	std::unique_ptr<Node> b,
	const Token &token) : Node(BIN_OP_NODE,
		token),
	op(op),
	a(std::move(a)),
	b(std::move(b))
{}

std::shared_ptr<Instruction> BinOpNode::genParser() const
{
	if (op == "+")
		return std::make_shared<AddI>(a->genParser(), b->genParser(), token);
	if (op == "-")
		return std::make_shared<SubI>(a->genParser(), b->genParser(), token);
	if (op == "*")
		return std::make_shared<MulI>(a->genParser(), b->genParser(), token);
	if (op == "/")
		return std::make_shared<DivI>(a->genParser(), b->genParser(), token);
	if (op == "%")
		return std::make_shared<ModI>(a->genParser(), b->genParser(), token);
	if (op == "**")
		return std::make_shared<PowI>(a->genParser(), b->genParser(), token);
	if (op == "|")
		return std::make_shared<BOrI>(a->genParser(), b->genParser(), token);
	if (op == "&")
		return std::make_shared<BAndI>(a->genParser(), b->genParser(), token);
	if (op == "^")
		return std::make_shared<BXOrI>(a->genParser(), b->genParser(), token);
	if (op == "<<")
		return std::make_shared<BShiftLeftI>(a->genParser(), b->genParser(), token);
	if (op == ">>")
		return std::make_shared<BShiftRightI>(a->genParser(), b->genParser(), token);

	if (op == "+=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<AddI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "-=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<SubI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "*=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<MulI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "/=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<DivI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "%=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<ModI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "**=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<PowI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "|=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<BOrI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "&=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<BAndI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "^=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<BXOrI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "<<=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<BShiftLeftI>(a->genParser(), b->genParser(), token), false, token);
	if (op == ">>=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<BShiftRightI>(a->genParser(), b->genParser(), token), false, token);

	if (op == "<")
		return std::make_shared<LessI>(a->genParser(), b->genParser(), token);
	if (op == ">")
		return std::make_shared<MoreI>(a->genParser(), b->genParser(), token);
	if (op == "<=")
		return std::make_shared<ELessI>(a->genParser(), b->genParser(), token);
	if (op == ">=")
		return std::make_shared<EMoreI>(a->genParser(), b->genParser(), token);
	if (op == "==")
		return std::make_shared<EqualsI>(a->genParser(), b->genParser(), token);
	if (op == "!=")
		return std::make_shared<NEqualsI>(a->genParser(), b->genParser(), token);
	if (op == "===")
		return std::make_shared<PureEqualsI>(a->genParser(), b->genParser(), token);
	if (op == "!==")
		return std::make_shared<PureNEqualsI>(a->genParser(), b->genParser(), token);
	if (op == "&&")
		return std::make_shared<AndI>(a->genParser(), b->genParser(), token);
	if (op == "||")
		return std::make_shared<OrI>(a->genParser(), b->genParser(), token);

	if (op == "=")
		return std::make_shared<SetI>(a->genParser(), b->genParser(), b->isConst(), token);
	if (op == ":=") {
		if (a->getType() != ID_NODE && a->getType() != BID_NODE)
			throw RuotaError("Only variables may be declared with `:=`", token);
		hash_ull t;
		if (a->getType() == ID_NODE)
			t = ((IDNode *) a.get())->getKey();
		else
			t = MAIN_HASH.hashString(((BIDNode *) a.get())->getKey());
		return std::make_shared<DeclareI>(t, 0, b->genParser(), b->isConst(), token);
	}

	if (op == "[]")
		return std::make_shared<IndexI>(a->genParser(), b->genParser(), token);

	throw RuotaError((boost::format(_UNKNOWN_BINARY_OP_) % op).str(), token);
}

const string &BinOpNode::getOp() const
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
	if (a->isConst() && b->isConst()) {
		try {
			Scope temp;
			genParser()->evaluate(&temp);
			return true;
		} catch (const RuotaError &e) {
			return false;
		}
	}
	return false;
}

void BinOpNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	if (isConst()) {
		Scope scope;
		auto evalA = a->genParser()->evaluate(&scope);
		auto evalB = b->genParser()->evaluate(&scope);
		return std::make_unique<ContainerNode>(genParser()->evaluate(&scope), token);
	}

	return std::make_unique<BinOpNode>(op, a->fold(), b->fold(), token);
}

//------------------------------------------------------------------------------------------------------

UnOpNode::UnOpNode(
	const string &op,
	std::unique_ptr<Node> a,
	const Token &token) : Node(UN_OP_NODE,
		token),
	op(op),
	a(std::move(a))
{}

std::shared_ptr<Instruction> UnOpNode::genParser() const
{
	if (op == "+")
		return a->genParser();
	if (op == "-")
		return std::make_shared<SubI>(std::make_unique<ContainerNode>(Symbol(CNumber::Long(0)), token)->genParser(), a->genParser(), token);
	if (op == "!")
		return std::make_shared<EqualsI>(std::make_unique<ContainerNode>(Symbol(false), token)->genParser(), a->genParser(), token);

	throw RuotaError((boost::format(_UNKNOWN_UNARY_OP_) % op).str(), token);
	return nullptr;
}

bool UnOpNode::isConst() const
{
	return a->isConst();
}

void UnOpNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "UNOP : " << op << "\n"
		<< colorASCII(RESET_TEXT);
	a->printTree(indent, true);
}

std::unique_ptr<Node> UnOpNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	return std::make_unique<UnOpNode>(op, a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

CastToNode::CastToNode(
	ValueType convert,
	std::unique_ptr<Node> a,
	const Token &token) : Node(CAST_TO_NODE,
		token),
	convert(convert),
	a(std::move(a))
{}

std::shared_ptr<Instruction> CastToNode::genParser() const
{
	return std::make_shared<CastToI>(a->genParser(), convert, token);
}

bool CastToNode::isConst() const
{
	return a->isConst();
}

void CastToNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "CAST : " << std::to_string(convert) << "\n"
		<< colorASCII(RESET_TEXT);
	a->printTree(indent, true);
}

std::unique_ptr<Node> CastToNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	return std::make_unique<CastToNode>(convert, a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

InsNode::InsNode(
	std::unique_ptr<Node> callee,
	std::unique_ptr<Node> arg,
	const Token &token) : Node(INS_NODE,
		token),
	callee(std::move(callee)),
	arg(std::move(arg))
{}

std::shared_ptr<Instruction> InsNode::genParser() const
{
	return std::make_shared<InnerI>(callee->genParser(), arg->genParser(), token);
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
void InsNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	return std::make_unique<InsNode>(callee->fold(), arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

IfElseNode::IfElseNode(
	std::unique_ptr<Node> ifs,
	std::unique_ptr<Node> body,
	const Token &token) : Node(IF_ELSE_NODE,
		token),
	ifs(std::move(ifs)),
	body(std::move(body))
{}

void IfElseNode::setElse(std::unique_ptr<Node> elses)
{
	this->elses = std::move(elses);
}

std::shared_ptr<Instruction> IfElseNode::genParser() const
{
	if (elses)
		return std::make_shared<IfElseI>(ifs->genParser(), body->genParser(), elses->genParser(), token);
	return std::make_shared<IfElseI>(ifs->genParser(), body->genParser(), nullptr, token);
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

void IfElseNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	auto ret = std::make_unique<IfElseNode>(ifs->fold(), body->fold(), token);
	if (elses)
		ret->setElse(elses->fold());
	return ret;
}

//------------------------------------------------------------------------------------------------------

WhileNode::WhileNode(
	std::unique_ptr<Node> whiles,
	std::vector<std::unique_ptr<Node>> body,
	const Token &token) : Node(WHILE_NODE,
		token),
	whiles(std::move(whiles)),
	body(std::move(body))
{}

std::shared_ptr<Instruction> WhileNode::genParser() const
{
	if (body.size() != 1) {
		std::vector<std::shared_ptr<Instruction>> is;
		for (auto &e : this->body)
			is.push_back(e->genParser());
		auto bodyI = std::make_shared<ScopeI>(is, token);

		return std::make_shared<WhileI>(whiles->genParser(), bodyI, token);
	} else {
		return std::make_shared<WhileI>(whiles->genParser(), body[0]->genParser(), token);
	}
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

void WhileNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	std::vector<std::unique_ptr<Node>> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold());
	return std::make_unique<WhileNode>(whiles->fold(), std::move(nbody), token);
}

//------------------------------------------------------------------------------------------------------

ForNode::ForNode(
	hash_ull id,
	std::unique_ptr<Node> fors,
	std::vector<std::unique_ptr<Node>> body,
	const Token &token) : Node(FOR_NODE,
		token),
	id(id),
	fors(std::move(fors)),
	body(std::move(body))
{}

std::shared_ptr<Instruction> ForNode::genParser() const
{
	if (body.size() != 1) {
		std::vector<std::shared_ptr<Instruction>> is;
		for (auto &e : this->body)
			is.push_back(e->genParser());
		auto bodyI = std::make_shared<ScopeI>(is, token);

		return std::make_shared<ForI>(id, fors->genParser(), bodyI, token);
	} else {
		return std::make_shared<ForI>(id, fors->genParser(), body[0]->genParser(), token);
	}
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

void ForNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "FOR : " << MAIN_HASH.deHash(id) << "\n"
		<< colorASCII(RESET_TEXT);
	fors->printTree(indent, false);
	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

std::unique_ptr<Node> ForNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	std::vector<std::unique_ptr<Node>> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold());
	return std::make_unique<ForNode>(id, fors->fold(), std::move(nbody), token);
}

//------------------------------------------------------------------------------------------------------

UntilNode::UntilNode(
	std::unique_ptr<Node> a,
	std::unique_ptr<Node> b,
	std::unique_ptr<Node> step,
	bool inclusive,
	const Token &token) : Node(UNTIL_NODE,
		token),
	a(std::move(a)),
	b(std::move(b)),
	step(std::move(step)),
	inclusive(inclusive)
{}

std::shared_ptr<Instruction> UntilNode::genParser() const
{
	if (step == nullptr)
		return std::make_shared<UntilI>(a->genParser(), b->genParser(), nullptr, inclusive, token);
	else
		return std::make_shared<UntilI>(a->genParser(), b->genParser(), step->genParser(), inclusive, token);
}

bool UntilNode::isConst() const
{
	bool flag = false;
	if (step == nullptr)
		flag = a->isConst() && b->isConst();
	else
		flag = a->isConst() && b->isConst() && step->isConst();
	if (flag) {
		try {
			Scope temp;
			genParser()->evaluate(&temp);
			return true;
		} catch (const RuotaError &e) {
			return false;
		}
	}
	return false;
}

void UntilNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "UNTIL\n"
		<< colorASCII(RESET_TEXT);

	a->printTree(indent, false);
	b->printTree(indent, step == nullptr);
	if (step != nullptr)
		step->printTree(indent, true);
}

std::unique_ptr<Node> UntilNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	if (step == nullptr)
		return std::make_unique<UntilNode>(a->fold(), b->fold(), nullptr, inclusive, token);
	else
		return std::make_unique<UntilNode>(a->fold(), b->fold(), step->fold(), inclusive, token);
}

//------------------------------------------------------------------------------------------------------

MapNode::MapNode(
	std::vector<std::pair<hash_ull, std::unique_ptr<Node>>> args,
	const Token &token) : Node(MAP_NODE,
		token),
	args(std::move(args))
{}

std::shared_ptr<Instruction> MapNode::genParser() const
{
	std::map<hash_ull, std::shared_ptr<Instruction>> is;
	for (auto &e : this->args) {
		is[e.first] = e.second->genParser();
	}
	return std::make_shared<MapI>(is, token);
}

bool MapNode::isConst() const
{
	for (auto &c : args)
		if (!c.second->isConst())
			return false;
	return true;
}

void MapNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	std::vector<std::pair<hash_ull, std::unique_ptr<Node>>> nargs;
	for (auto &c : args)
		nargs.push_back({ c.first, c.second->fold() });
	return std::make_unique<MapNode>(std::move(nargs), token);
}

//------------------------------------------------------------------------------------------------------

SwitchNode::SwitchNode(
	std::unique_ptr<Node> switchs,
	std::map<std::unique_ptr<Node>, std::unique_ptr<Node>> cases,
	const Token &token) : Node(SWITCH_NODE,
		token),
	switchs(std::move(switchs)),
	cases(std::move(cases))
{}

std::shared_ptr<Instruction> SwitchNode::genParser() const
{
	std::map<Symbol, std::shared_ptr<Instruction>> cases_solved;
	std::map<std::shared_ptr<Instruction>, std::shared_ptr<Instruction>> cases_unsolved;
	for (auto &e : this->cases) {
		if (e.first->isConst()) {
			Scope temp;
			auto key = e.first->genParser()->evaluate(&temp);
			cases_solved[key] = e.second->genParser();
		} else {
			cases_unsolved[e.first->genParser()] = e.second->genParser();
		}
	}
	if (elses)
		return std::make_shared<SwitchI>(switchs->genParser(), cases_solved, cases_unsolved, elses->genParser(), token);
	return std::make_shared<SwitchI>(switchs->genParser(), cases_solved, cases_unsolved, nullptr, token);
}

void SwitchNode::setElse(std::unique_ptr<Node> elses)
{
	this->elses = std::move(elses);
}

bool SwitchNode::isConst() const
{
	return false;
}

void SwitchNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "SWITCH"
		<< "\n"
		<< colorASCII(RESET_TEXT);
	switchs->printTree(indent, cases.empty());
	size_t i = 0;
	for (auto &e : cases) {
		e.second->printTree(indent, i == cases.size() - 1 && !elses);
		i++;
	}
	if (elses)
		elses->printTree(indent, true);
}

std::unique_ptr<Node> SwitchNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		Scope scope;
		return std::make_unique<ContainerNode>(i->evaluate(&scope), token);
	}

	std::map<std::unique_ptr<Node>, std::unique_ptr<Node>> ncases;
	for (auto &c : cases)
		ncases[c.first->fold()] = c.second->fold();
	auto ret = std::make_unique<SwitchNode>(switchs->fold(), std::move(ncases), token);
	if (elses)
		ret->setElse(elses->fold());
	return ret;
}

//------------------------------------------------------------------------------------------------------

TryCatchNode::TryCatchNode(
	std::unique_ptr<Node> trys,
	std::unique_ptr<Node> catchs,
	hash_ull key,
	const Token &token) : Node(TRY_CATCH_NODE,
		token),
	trys(std::move(trys)),
	catchs(std::move(catchs)),
	key(key)
{}

std::shared_ptr<Instruction> TryCatchNode::genParser() const
{
	return std::make_shared<TryCatchI>(trys->genParser(), catchs->genParser(), key, token);
}

bool TryCatchNode::isConst() const
{
	return false;
}

void TryCatchNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
		std::cout << "├─";
		indent += "│ ";
	}
	std::cout << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT)) << "TRY_CATCH : " << MAIN_HASH.deHash(key) << "\n"
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
	std::unique_ptr<Node> throws,
	const Token &token) : Node(THROW_NODE, token),
	throws(std::move(throws))
{}

std::shared_ptr<Instruction> ThrowNode::genParser() const
{
	return std::make_shared<ThrowI>(throws->genParser(), token);
}

bool ThrowNode::isConst() const
{
	return false;
}

void ThrowNode::printTree(string indent, bool last) const
{
	std::cout << indent;
	if (last) {
		std::cout << "└─";
		indent += "  ";
	} else {
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