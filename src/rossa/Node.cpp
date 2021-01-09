#include "../../bin/include/Rossa.h"

using namespace rossa;

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

std::stringstream ContainerNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "CONTAINER : " << s.toCodeString() << "\n" << colorASCII(RESET_TEXT);
	return ss;
}

std::shared_ptr<Node> ContainerNode::fold() const
{
	return std::make_unique<ContainerNode>(s, token);
}

//------------------------------------------------------------------------------------------------------

VectorNode::VectorNode(
	const std::vector<std::shared_ptr<Node>> &args,
	const bool &scoped,
	const Token &token) : Node(VECTOR_NODE,
		token),
	args(args),
	scoped(scoped)
{}

std::shared_ptr<Instruction> VectorNode::genParser() const
{
	std::vector<std::shared_ptr<Instruction>> ins;
	for (auto &n : args)
		if (n != nullptr)
			ins.push_back(n->genParser());
	if (scoped)
		return std::make_shared<ScopeI>(ins, token);
	return std::make_shared<SequenceI>(ins, token);
}

bool VectorNode::isConst() const
{
	for (auto &c : args)
		if (c != nullptr && !c->isConst())
			return false;
	return true;
}

std::stringstream VectorNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "ARRAY : " << scoped << "\n" << colorASCII(RESET_TEXT);
	for (size_t i = 0; i < args.size(); i++)
		if (args[i] != nullptr)
			ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> VectorNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	std::vector<std::shared_ptr<Node>> nargs;
	for (auto &c : args)
		if (c != nullptr)
			nargs.push_back(c->fold());
	return std::make_unique<VectorNode>(nargs, scoped, token);
}

const std::vector<std::shared_ptr<Node>> &VectorNode::getChildren()
{
	return args;
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

std::stringstream BreakNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "BREAK\n" << colorASCII(RESET_TEXT);
	return ss;
}

std::shared_ptr<Node> BreakNode::fold() const
{
	return std::make_unique<ContainerNode>(Symbol(ID_BREAK), token);
}

//------------------------------------------------------------------------------------------------------

ContinueNode::ContinueNode(
	const Token &token) : Node(CONTINUE_NODE,
		token)
{}

std::shared_ptr<Instruction> ContinueNode::genParser() const
{
	return std::make_shared<ContainerI>(Symbol(ID_CONTINUE), token);
}

bool ContinueNode::isConst() const
{
	return true;
}

std::stringstream ContinueNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "CONTINUE\n" << colorASCII(RESET_TEXT);
	return ss;
}

std::shared_ptr<Node> ContinueNode::fold() const
{
	return std::make_unique<ContainerNode>(Symbol(ID_CONTINUE), token);
}

//------------------------------------------------------------------------------------------------------

IDNode::IDNode(
	const hash_ull &key,
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

std::stringstream IDNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "ID : " << ROSSA_DEHASH(key) << "\n" << colorASCII(RESET_TEXT);
	return ss;
}

std::shared_ptr<Node> IDNode::fold() const
{
	return std::make_unique<IDNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

BIDNode::BIDNode(
	const std::string &key,
	const Token &token) : Node(BID_NODE,
		token),
	key(key)
{}

const std::string BIDNode::getKey() const
{
	return key;
}

std::shared_ptr<Instruction> BIDNode::genParser() const
{
	return std::make_shared<VariableI>(ROSSA_HASH(key), token);
}

bool BIDNode::isConst() const
{
	return false;
}

std::stringstream BIDNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "BID : " << key << "\n" << colorASCII(RESET_TEXT);
	return ss;
}

std::shared_ptr<Node> BIDNode::fold() const
{
	return std::make_unique<BIDNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

DefineNode::DefineNode(
	const hash_ull &key,
	const sig_t &ftype,
	const std::vector<std::pair<LexerTokenType, hash_ull>> &params,
	const std::shared_ptr<Node> &body,
	const std::vector<hash_ull> &captures,
	const Token &token) : Node(DEFINE_NODE,
		token),
	key(key),
	ftype(ftype),
	params(params),
	body(body),
	captures(captures)
{}

std::shared_ptr<Instruction> DefineNode::genParser() const
{
	return std::make_shared<DefineI>(key, ftype, params, body->genParser(), captures, token);
}

bool DefineNode::isConst() const
{
	return false;
}

std::stringstream DefineNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "DEFINE : " << (key > 0 ? ROSSA_DEHASH(key) : "<LAMBDA>") << ", " << sig::toString(ftype) << "\n" << colorASCII(RESET_TEXT);
	ss << body->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> DefineNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<DefineNode>(key, ftype, params, body->fold(), captures, token);
}

//------------------------------------------------------------------------------------------------------

NewNode::NewNode(
	const std::shared_ptr<Node> &object,
	const std::shared_ptr<Node> &params,
	const Token &token) : Node(NEW_NODE,
		token),
	object(object),
	params(params)
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

std::stringstream NewNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "NEW\n" << colorASCII(RESET_TEXT);
	ss << object->printTree(indent, false).str();
	ss << params->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> NewNode::fold() const
{
	return std::make_unique<NewNode>(object->fold(), params->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ClassNode::ClassNode(
	const hash_ull &key,
	const int &type,
	const std::vector<std::shared_ptr<Node>> &body,
	const std::shared_ptr<Node> &extends,
	const Token &token) : Node(CLASS_NODE,
		token),
	key(key),
	type(type),
	body(body),
	extends(extends)
{}

std::shared_ptr<Instruction> ClassNode::genParser() const
{
	std::vector<std::shared_ptr<Instruction>> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = std::make_shared<ScopeI>(is, token);
	std::vector<Function> stack_trace;

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
			throw RTError(_INVALID_OBJECT_TYPE_, token, stack_trace);
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

std::stringstream ClassNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "CLASS : " << ROSSA_DEHASH(key) << ", " << std::to_string(type) << "\n" << colorASCII(RESET_TEXT);
	if (extends != nullptr)
		ss << extends->printTree(indent, false).str();
	for (size_t i = 0; i < body.size(); i++)
		ss << body[i]->printTree(indent, i == body.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> ClassNode::fold() const
{
	std::vector<std::shared_ptr<Node>> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold());

	if (extends)
		return std::make_unique<ClassNode>(key, type, nbody, extends->fold(), token);
	else
		return std::make_unique<ClassNode>(key, type, nbody, nullptr, token);
}

//------------------------------------------------------------------------------------------------------

VarNode::VarNode(
	const std::vector<hash_ull> &keys,
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

std::stringstream VarNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "VAR : " << keys.size() << "\n" << colorASCII(RESET_TEXT);
	return ss;
}

std::shared_ptr<Node> VarNode::fold() const
{
	return std::make_unique<VarNode>(keys, token);
}

//------------------------------------------------------------------------------------------------------

CallNode::CallNode(
	const std::shared_ptr<Node> &callee,
	const std::vector<std::shared_ptr<Node>> &args,
	const Token &token) : Node(CALL_NODE,
		token),
	callee(callee),
	args(args)
{}

std::shared_ptr<Instruction> CallNode::genParser() const
{
	auto fcallee = callee->genParser();
	std::vector<std::shared_ptr<Instruction>> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<CallI>(fcallee, std::make_shared<SequenceI>(fargs, token), token);
}

std::shared_ptr<Node> CallNode::getCallee()
{
	return (callee);
}

std::vector<std::shared_ptr<Node>> CallNode::getArgs()
{
	return (args);
}

bool CallNode::isConst() const
{
	return false;
}

std::stringstream CallNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "CALL\n" << colorASCII(RESET_TEXT);
	ss << callee->printTree(indent, args.empty()).str();
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> CallNode::fold() const
{
	std::vector<std::shared_ptr<Node>> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold());

	return std::make_unique<CallNode>(callee->fold(), nargs, token);
}

//------------------------------------------------------------------------------------------------------

ExternCallNode::ExternCallNode(
	const std::string &libname,
	const std::string &fname,
	const std::vector<std::shared_ptr<Node>> &args,
	const Token &token) : Node(EXTERN_CALL_NODE,
		token),
	libname(libname),
	fname(fname),
	args(args)
{}

std::shared_ptr<Instruction> ExternCallNode::genParser() const
{
	std::vector<std::shared_ptr<Instruction>> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<ExternI>(libname, fname, std::make_shared<SequenceI>(fargs, token), token);
}

bool ExternCallNode::isConst() const
{
	return false;
}

std::stringstream ExternCallNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "EXTERN_CALL : " << libname << "::" << fname << "\n" << colorASCII(RESET_TEXT);
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> ExternCallNode::fold() const
{
	std::vector<std::shared_ptr<Node>> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold());

	return std::make_unique<ExternCallNode>(libname, fname, nargs, token);
}

//------------------------------------------------------------------------------------------------------

CallBuiltNode::CallBuiltNode(
	const LexerTokenType &t,
	const std::shared_ptr<Node> &arg,
	const Token &token) : Node(CALL_BUILT_NODE,
		token),
	t(t),
	arg(arg)
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
		case TOK_PARSE:
			return std::make_shared<ParseI>(arg->genParser(), token);
		case TOK_CHARN:
			return std::make_shared<CharNI>(arg->genParser(), token);
		case TOK_CHARS:
			return std::make_shared<CharSI>(arg->genParser(), token);
		default:
			break;
	}

	std::vector<Function> stack_trace;
	throw RTError(format::format(_UNKNOWN_BUILT_CALL_, { std::to_string(t) }), token, stack_trace);
	return nullptr;
}

bool CallBuiltNode::isConst() const
{
	return arg->isConst();
}

std::stringstream CallBuiltNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "CALL_BUILT : " << std::to_string(t) << "\n" << colorASCII(RESET_TEXT);
	ss << arg->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> CallBuiltNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<CallBuiltNode>(t, arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ReturnNode::ReturnNode(
	const std::shared_ptr<Node> &a,
	const Token &token) : Node(REFER_NODE,
		token),
	a(a)
{}

std::shared_ptr<Instruction> ReturnNode::genParser() const
{
	return std::make_shared<ReturnI>(a->genParser(), token);
}

bool ReturnNode::isConst() const
{
	return false;
}

std::stringstream ReturnNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "RETURN\n" << colorASCII(RESET_TEXT);
	ss << a->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> ReturnNode::fold() const
{
	return std::make_unique<ReturnNode>(a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ReferNode::ReferNode(
	const std::shared_ptr<Node> &a,
	const Token &token) : Node(RETURN_NODE,
		token),
	a(a)
{}

std::shared_ptr<Instruction> ReferNode::genParser() const
{
	return std::make_shared<ReferI>(a->genParser(), token);
}

bool ReferNode::isConst() const
{
	return false;
}

std::stringstream ReferNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "REFER\n" << colorASCII(RESET_TEXT);
	ss << a->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> ReferNode::fold() const
{
	return std::make_unique<ReferNode>(a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

BinOpNode::BinOpNode(
	const std::string &op,
	const std::shared_ptr<Node> &a,
	const std::shared_ptr<Node> &b,
	const Token &token) : Node(BIN_OP_NODE,
		token),
	op(op),
	a(a),
	b(b)
{}

std::shared_ptr<Instruction> BinOpNode::genParser() const
{
	std::vector<Function> stack_trace;

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

	if (op == "&&=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<AndI>(a->genParser(), b->genParser(), token), false, token);
	if (op == "||=")
		return std::make_shared<SetI>(a->genParser(), std::make_shared<OrI>(a->genParser(), b->genParser(), token), false, token);

	if (op == "=")
		return std::make_shared<SetI>(a->genParser(), b->genParser(), b->isConst(), token);
	if (op == ":=") {
		if (a->getType() != ID_NODE && a->getType() != BID_NODE)
			throw RTError("Only variables may be declared with `:=`", token, stack_trace);
		hash_ull t;
		if (a->getType() == ID_NODE)
			t = ((IDNode *)a.get())->getKey();
		else
			t = ROSSA_HASH(((BIDNode *)a.get())->getKey());
		return std::make_shared<DeclareI>(t, 0, b->genParser(), b->isConst(), token);
	}

	if (op == "[]")
		return std::make_shared<IndexI>(a->genParser(), b->genParser(), token);
	if (op == "->")
		return std::make_shared<CastToI>(a->genParser(), b->genParser(), token);

	throw RTError(format::format(_UNKNOWN_BINARY_OP_, { op }), token, stack_trace);
}

const std::string &BinOpNode::getOp() const
{
	return op;
}

std::shared_ptr<Node> BinOpNode::getA() const
{
	return a;
};

std::shared_ptr<Node> BinOpNode::getB() const
{
	return b;
};

void BinOpNode::setA(const std::shared_ptr<Node> &a)
{
	this->a = (a);
}

void BinOpNode::setB(const std::shared_ptr<Node> &b)
{
	this->b = (b);
}

bool BinOpNode::isConst() const
{
	if (a->isConst() && b->isConst()) {
		try {
			auto newScope = std::make_shared<Scope>();
			std::vector<Function> stack_trace;
			genParser()->evaluate(newScope, stack_trace);
			newScope->clear();
			return true;
		} catch (const RTError &e) {
			return false;
		}
	}
	return false;
}

std::stringstream BinOpNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "BINOP : " << op << "\n" << colorASCII(RESET_TEXT);
	ss << a->printTree(indent, false).str();
	ss << b->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> BinOpNode::fold() const
{
	if (isConst()) {
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto evalA = a->genParser()->evaluate(newScope, stack_trace);
		auto evalB = b->genParser()->evaluate(newScope, stack_trace);
		auto r = genParser()->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<BinOpNode>(op, a->fold(), b->fold(), token);
}

//------------------------------------------------------------------------------------------------------

UnOpNode::UnOpNode(
	const std::string &op,
	const std::shared_ptr<Node> &a,
	const Token &token) : Node(UN_OP_NODE,
		token),
	op(op),
	a(a)
{}

std::shared_ptr<Instruction> UnOpNode::genParser() const
{
	if (op == "+")
		return a->genParser();
	if (op == "-")
		return std::make_shared<SubI>(std::make_unique<ContainerNode>(Symbol(RNumber::Long(0)), token)->genParser(), a->genParser(), token);
	if (op == "!")
		return std::make_shared<EqualsI>(std::make_unique<ContainerNode>(Symbol(false), token)->genParser(), a->genParser(), token);
	if (op == "$")
		return std::make_shared<TypeI>(a->genParser(), token);
	if (op == "|>") {
		sig_t s;
		std::vector<std::pair<LexerTokenType, hash_ull>> p;
		std::vector<hash_ull> c;
		return std::make_shared<DefineI>(0, s, p, a->genParser(), c, token);
	}

	std::vector<Function> stack_trace;
	throw RTError(format::format(_UNKNOWN_UNARY_OP_, { op }), token, stack_trace);
	return nullptr;
}

bool UnOpNode::isConst() const
{
	return a->isConst();
}

std::stringstream UnOpNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "UNOP : " << op << "\n" << colorASCII(RESET_TEXT);
	ss << a->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> UnOpNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<UnOpNode>(op, a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ParenNode::ParenNode(
	const std::shared_ptr<Node> &a,
	const Token &token) : Node(PAREN_NODE,
		token),
	a(a)
{}

std::shared_ptr<Instruction> ParenNode::genParser() const
{
	return a->genParser();
}

bool ParenNode::isConst() const
{
	return a->isConst();
}

std::stringstream ParenNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "PAREN\n" << colorASCII(RESET_TEXT);
	ss << a->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> ParenNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<ParenNode>(a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

InsNode::InsNode(
	const std::shared_ptr<Node> &callee,
	const std::shared_ptr<Node> &arg,
	const Token &token) : Node(INS_NODE,
		token),
	callee(callee),
	arg(arg)
{}

std::shared_ptr<Instruction> InsNode::genParser() const
{
	return std::make_shared<InnerI>(callee->genParser(), arg->genParser(), token);
}

std::shared_ptr<Node> InsNode::getCallee()
{
	return (callee);
}

std::shared_ptr<Node> InsNode::getArg()
{
	return (arg);
}

bool InsNode::isConst() const
{
	return callee->isConst() && arg->isConst();
}
std::stringstream InsNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "INS\n" << colorASCII(RESET_TEXT);
	ss << callee->printTree(indent, false).str();
	ss << arg->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> InsNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<InsNode>(callee->fold(), arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

IfElseNode::IfElseNode(
	const std::shared_ptr<Node> &ifs,
	const std::shared_ptr<Node> &body,
	const Token &token) : Node(IF_ELSE_NODE,
		token),
	ifs(ifs),
	body(body)
{}

void IfElseNode::setElse(const std::shared_ptr<Node> &elses)
{
	this->elses = (elses);
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

std::stringstream IfElseNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "IF_ELSE\n" << colorASCII(RESET_TEXT);
	ss << ifs->printTree(indent, false).str();
	ss << body->printTree(indent, elses == nullptr).str();
	if (elses != nullptr)
		ss << elses->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> IfElseNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	auto ret = std::make_unique<IfElseNode>(ifs->fold(), body->fold(), token);
	if (elses)
		ret->setElse(elses->fold());
	return ret;
}

//------------------------------------------------------------------------------------------------------

WhileNode::WhileNode(
	const std::shared_ptr<Node> &whiles,
	const std::vector<std::shared_ptr<Node>> &body,
	const Token &token) : Node(WHILE_NODE,
		token),
	whiles(whiles),
	body(body)
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

std::stringstream WhileNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "WHILE\n" << colorASCII(RESET_TEXT);
	ss << whiles->printTree(indent, false).str();
	for (size_t i = 0; i < body.size(); i++)
		ss << body[i]->printTree(indent, i == body.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> WhileNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	std::vector<std::shared_ptr<Node>> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold());
	return std::make_unique<WhileNode>(whiles->fold(), nbody, token);
}

//------------------------------------------------------------------------------------------------------

ForNode::ForNode(
	const hash_ull &id,
	const std::shared_ptr<Node> &fors,
	const std::vector<std::shared_ptr<Node>> &body,
	const Token &token) : Node(FOR_NODE,
		token),
	id(id),
	fors(fors),
	body(body)
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

std::stringstream ForNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "FOR : " << ROSSA_DEHASH(id) << "\n" << colorASCII(RESET_TEXT);
	ss << fors->printTree(indent, false).str();
	for (size_t i = 0; i < body.size(); i++)
		ss << body[i]->printTree(indent, i == body.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> ForNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	std::vector<std::shared_ptr<Node>> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold());
	return std::make_unique<ForNode>(id, fors->fold(), nbody, token);
}

//------------------------------------------------------------------------------------------------------

UntilNode::UntilNode(
	const std::shared_ptr<Node> &a,
	const std::shared_ptr<Node> &b,
	const std::shared_ptr<Node> &step,
	const bool &inclusive,
	const Token &token) : Node(UNTIL_NODE,
		token),
	a(a),
	b(b),
	step(step),
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
			auto i = genParser();
			auto newScope = std::make_shared<Scope>();
			std::vector<Function> stack_trace;
			i->evaluate(newScope, stack_trace);
			newScope->clear();
			return true;
		} catch (const RTError &e) {
			return false;
		}
	}
	return false;
}

std::stringstream UntilNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "UNTIL\n" << colorASCII(RESET_TEXT);
	ss << a->printTree(indent, false).str();
	ss << b->printTree(indent, step == nullptr).str();
	if (step != nullptr)
		ss << step->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> UntilNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	if (step == nullptr)
		return std::make_unique<UntilNode>(a->fold(), b->fold(), nullptr, inclusive, token);
	else
		return std::make_unique<UntilNode>(a->fold(), b->fold(), step->fold(), inclusive, token);
}

//------------------------------------------------------------------------------------------------------

MapNode::MapNode(
	const std::vector<std::pair<std::string, std::shared_ptr<Node>>> &args,
	const Token &token) : Node(MAP_NODE,
		token),
	args(args)
{}

std::shared_ptr<Instruction> MapNode::genParser() const
{
	std::map<std::string, std::shared_ptr<Instruction>> is;
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

std::stringstream MapNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "MAP\n" << colorASCII(RESET_TEXT);
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i].second->printTree(indent, i == args.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> MapNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	std::vector<std::pair<std::string, std::shared_ptr<Node>>> nargs;
	for (auto &c : args)
		nargs.push_back({ c.first, c.second->fold() });
	return std::make_unique<MapNode>(nargs, token);
}

//------------------------------------------------------------------------------------------------------

SwitchNode::SwitchNode(
	const std::shared_ptr<Node> &switchs,
	const std::map<std::shared_ptr<Node>, size_t> &cases,
	const std::vector<std::shared_ptr<Node>> &gotos,
	const Token &token) : Node(SWITCH_NODE,
		token),
	switchs(switchs),
	cases(cases),
	gotos(gotos)
{}

std::shared_ptr<Instruction> SwitchNode::genParser() const
{
	std::map<Symbol, size_t> cases_solved;
	std::map<std::shared_ptr<Instruction>, size_t> cases_unsolved;
	std::vector<std::shared_ptr<Instruction>> goto_cases;
	std::vector<Function> stack_trace;
	for (auto &e : this->cases) {
		if (e.first->isConst()) {
			auto newScope = std::make_shared<Scope>();
			std::vector<Function> stack_trace;
			auto key = e.first->genParser()->evaluate(newScope, stack_trace);
			newScope->clear();
			cases_solved[key] = e.second;
		} else {
			cases_unsolved[e.first->genParser()] = e.second;
		}
	}
	for (auto &e : this->gotos) {
		goto_cases.push_back(e->genParser());
	}
	if (elses)
		return std::make_shared<SwitchI>(switchs->genParser(), cases_solved, cases_unsolved, goto_cases, elses->genParser(), token);
	return std::make_shared<SwitchI>(switchs->genParser(), cases_solved, cases_unsolved, goto_cases, nullptr, token);
}

void SwitchNode::setElse(const std::shared_ptr<Node> &elses)
{
	this->elses = (elses);
}

bool SwitchNode::isConst() const
{
	return false;
}

std::stringstream SwitchNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "SWITCH\n" << colorASCII(RESET_TEXT);
	ss << switchs->printTree(indent, gotos.empty() && cases.empty()).str();
	size_t i = 0;
	for (auto &e : cases) {
		ss << e.first->printTree(indent, i == (cases.size() + gotos.size()) - 1 && !elses).str();
		i++;
	}
	for (auto &e : gotos) {
		ss << e->printTree(indent, i == (cases.size() + gotos.size()) - 1 && !elses).str();
		i++;
	}
	if (elses)
		ss << elses->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> SwitchNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		auto newScope = std::make_shared<Scope>();
		std::vector<Function> stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}

	std::map<std::shared_ptr<Node>, size_t> ncases;
	std::vector<std::shared_ptr<Node>> ngotos;
	for (auto &c : cases)
		ncases[c.first->fold()] = c.second;
	for (auto &e : gotos)
		ngotos.push_back(e->fold());
	auto ret = std::make_unique<SwitchNode>(switchs->fold(), ncases, ngotos, token);
	if (elses)
		ret->setElse(elses->fold());
	return ret;
}

//------------------------------------------------------------------------------------------------------

TryCatchNode::TryCatchNode(
	const std::shared_ptr<Node> &trys,
	const std::shared_ptr<Node> &catchs,
	const hash_ull &key,
	const Token &token) : Node(TRY_CATCH_NODE,
		token),
	trys(trys),
	catchs(catchs),
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

std::stringstream TryCatchNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "TRY_CATCH : " << ROSSA_DEHASH(key) << "\n" << colorASCII(RESET_TEXT);
	ss << trys->printTree(indent, false).str();
	ss << catchs->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> TryCatchNode::fold() const
{
	return std::make_unique<TryCatchNode>(trys->fold(), catchs->fold(), key, token);
}

//------------------------------------------------------------------------------------------------------

ThrowNode::ThrowNode(
	const std::shared_ptr<Node> &throws,
	const Token &token) : Node(THROW_NODE, token),
	throws(throws)
{}

std::shared_ptr<Instruction> ThrowNode::genParser() const
{
	return std::make_shared<ThrowI>(throws->genParser(), token);
}

bool ThrowNode::isConst() const
{
	return false;
}

std::stringstream ThrowNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "THROW\n" << colorASCII(RESET_TEXT);
	ss << throws->printTree(indent, true).str();
	return ss;
}

std::shared_ptr<Node> ThrowNode::fold() const
{
	return std::make_unique<ThrowNode>(throws->fold(), token);
}

//------------------------------------------------------------------------------------------------------

CallOpNode::CallOpNode(
	const size_t &id,
	const std::vector<std::shared_ptr<Node>> &args,
	const Token &token) : Node(CALL_OP_NODE,
		token),
	id(id),
	args(args)
{}

std::shared_ptr<Instruction> CallOpNode::genParser() const
{
	std::vector<std::shared_ptr<Instruction>> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<CallOpI>(id, fargs, token);
}

bool CallOpNode::isConst() const
{
	return false;
}

std::stringstream CallOpNode::printTree(std::string indent, bool last) const
{
	std::stringstream ss;
	ss << indent;
	if (last) {
		ss << "└─";
		indent += "  ";
	} else {
		ss << "├─";
		indent += "│ ";
	}
	ss << (isConst() ? colorASCII(CYAN_TEXT) : colorASCII(WHITE_TEXT));
	ss << "CALL_OP : " << id << "\n" << colorASCII(RESET_TEXT);
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

std::shared_ptr<Node> CallOpNode::fold() const
{
	std::vector<std::shared_ptr<Node>> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold());

	return std::make_unique<CallOpNode>(id, nargs, token);
}
