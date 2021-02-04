#include "Rossa.h"

Node::Node(
	const type_t &type,
	const token_t &token) : type(type),
	token(token)
{}

const Node::type_t Node::getType() const
{
	return type;
}

const token_t Node::getToken() const
{
	return token;
}

//------------------------------------------------------------------------------------------------------

ContainerNode::ContainerNode(
	const sym_t &s,
	const token_t &token) : Node(CONTAINER_NODE,
		token),
	s(s)
{}

i_ptr_t ContainerNode::genParser() const
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
	ss << "CONTAINER : " << s.toCodeString() << "\n";
	return ss;
}

const node_ptr_t ContainerNode::fold() const
{
	return std::make_unique<ContainerNode>(s, token);
}

//------------------------------------------------------------------------------------------------------

VectorNode::VectorNode(
	const node_vec_t &args,
	const bool &scoped,
	const token_t &token) : Node(VECTOR_NODE,
		token),
	args(args),
	scoped(scoped)
{}

i_ptr_t VectorNode::genParser() const
{
	i_vec_t ins;
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
	ss << "ARRAY : " << scoped << "\n";
	for (size_t i = 0; i < args.size(); i++)
		if (args[i] != nullptr)
			ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

const node_ptr_t VectorNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	node_vec_t nargs;
	for (auto &c : args)
		if (c != nullptr)
			nargs.push_back(c->fold());
	return std::make_unique<VectorNode>(nargs, scoped, token);
}

const node_vec_t &VectorNode::getChildren()
{
	return args;
}

//------------------------------------------------------------------------------------------------------

BreakNode::BreakNode(
	const token_t &token) : Node(BREAK_NODE,
		token)
{}

i_ptr_t BreakNode::genParser() const
{
	return std::make_shared<ContainerI>(sym_t(sym_t::type_t::ID_BREAK), token);
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
	ss << "BREAK\n";
	return ss;
}

const node_ptr_t BreakNode::fold() const
{
	return std::make_unique<ContainerNode>(sym_t(sym_t::type_t::ID_BREAK), token);
}

//------------------------------------------------------------------------------------------------------

ContinueNode::ContinueNode(
	const token_t &token) : Node(CONTINUE_NODE,
		token)
{}

i_ptr_t ContinueNode::genParser() const
{
	return std::make_shared<ContainerI>(sym_t(sym_t::type_t::ID_CONTINUE), token);
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
	ss << "CONTINUE\n";
	return ss;
}

const node_ptr_t ContinueNode::fold() const
{
	return std::make_unique<ContainerNode>(sym_t(sym_t::type_t::ID_CONTINUE), token);
}

//------------------------------------------------------------------------------------------------------

IDNode::IDNode(
	const hash_ull &key,
	const token_t &token) : Node(ID_NODE,
		token),
	key(key)
{}

hash_ull IDNode::getKey() const
{
	return key;
}

i_ptr_t IDNode::genParser() const
{
	if (key == Rossa::HASH_THIS)
		return std::make_shared<GetThisI>(token);
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
	ss << "ID : " << ROSSA_DEHASH(key) << "\n";
	return ss;
}

const node_ptr_t IDNode::fold() const
{
	return std::make_unique<IDNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

BIDNode::BIDNode(
	const std::string &key,
	const token_t &token) : Node(BID_NODE,
		token),
	key(key)
{}

const std::string BIDNode::getKey() const
{
	return key;
}

i_ptr_t BIDNode::genParser() const
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
	ss << "BID : " << key << "\n";
	return ss;
}

const node_ptr_t BIDNode::fold() const
{
	return std::make_unique<BIDNode>(key, token);
}

//------------------------------------------------------------------------------------------------------

DefineNode::DefineNode(
	const hash_ull &key,
	const fsig_t &ftype,
	const std::vector<std::pair<LexerTokenType, hash_ull>> &params,
	const node_ptr_t &body,
	const std::vector<hash_ull> &captures,
	const token_t &token) : Node(DEFINE_NODE,
		token),
	key(key),
	ftype(ftype),
	params(params),
	body(body),
	captures(captures)
{}

i_ptr_t DefineNode::genParser() const
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
	ss << "DEFINE : " << (key > 0 ? ROSSA_DEHASH(key) : "<LAMBDA>") << ", " << ftype.toString() << "\n";
	ss << body->printTree(indent, true).str();
	return ss;
}

const node_ptr_t DefineNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<DefineNode>(key, ftype, params, body->fold(), captures, token);
}

//------------------------------------------------------------------------------------------------------

VargDefineNode::VargDefineNode(
	const hash_ull &key,
	const node_ptr_t &body,
	const std::vector<hash_ull> &captures,
	const token_t &token) : Node(VARG_DEFINE_NODE,
		token),
	key(key),
	body(body),
	captures(captures)
{}

i_ptr_t VargDefineNode::genParser() const
{
	return std::make_shared<VargDefineI>(key, body->genParser(), captures, token);
}

bool VargDefineNode::isConst() const
{
	return false;
}

std::stringstream VargDefineNode::printTree(std::string indent, bool last) const
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
	ss << "DEFINE : " << (key > 0 ? ROSSA_DEHASH(key) : "<LAMBDA>") << "\n";
	ss << body->printTree(indent, true).str();
	return ss;
}

const node_ptr_t VargDefineNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<VargDefineNode>(key, body->fold(), captures, token);
}

//------------------------------------------------------------------------------------------------------

NewNode::NewNode(
	const node_ptr_t &object,
	const node_ptr_t &params,
	const token_t &token) : Node(NEW_NODE,
		token),
	object(object),
	params(params)
{}

i_ptr_t NewNode::genParser() const
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
	ss << "NEW\n";
	ss << object->printTree(indent, false).str();
	ss << params->printTree(indent, true).str();
	return ss;
}

const node_ptr_t NewNode::fold() const
{
	return std::make_unique<NewNode>(object->fold(), params->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ClassNode::ClassNode(
	const hash_ull &key,
	const int &type,
	const node_vec_t &body,
	const node_ptr_t &extends,
	const token_t &token) : Node(CLASS_NODE,
		token),
	key(key),
	type(type),
	body(body),
	extends(extends)
{}

i_ptr_t ClassNode::genParser() const
{
	i_vec_t is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = std::make_shared<ScopeI>(is, token);
	trace_t stack_trace;

	Scope::type_t ot;
	switch (type) {
		case TOK_STRUCT:
			ot = Scope::type_t::STRUCT_O;
			break;
		case TOK_STATIC:
			ot = Scope::type_t::STATIC_O;
			break;
		case TOK_VIRTUAL:
			ot = Scope::type_t::VIRTUAL_O;
			break;
		default:
			throw rossa_error(_INVALID_OBJECT_TYPE_, token, stack_trace);
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
	ss << "CLASS : " << ROSSA_DEHASH(key) << ", " << std::to_string(type) << "\n";
	if (extends != nullptr)
		ss << extends->printTree(indent, false).str();
	for (size_t i = 0; i < body.size(); i++)
		ss << body[i]->printTree(indent, i == body.size() - 1).str();
	return ss;
}

const node_ptr_t ClassNode::fold() const
{
	node_vec_t nbody;
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
	const token_t &token) : Node(VAR_NODE,
		token),
	keys(keys)
{}

i_ptr_t VarNode::genParser() const
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
	ss << "VAR : " << keys.size() << "\n";
	return ss;
}

const node_ptr_t VarNode::fold() const
{
	return std::make_unique<VarNode>(keys, token);
}

//------------------------------------------------------------------------------------------------------

CallNode::CallNode(
	const node_ptr_t &callee,
	const node_vec_t &args,
	const token_t &token) : Node(CALL_NODE,
		token),
	callee(callee),
	args(args)
{}

i_ptr_t CallNode::genParser() const
{
	auto fcallee = callee->genParser();
	i_vec_t fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<CallI>(fcallee, std::make_shared<SequenceI>(fargs, token), token);
}

node_ptr_t CallNode::getCallee() const
{
	return (callee);
}

node_vec_t CallNode::getArgs() const
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
	ss << "CALL\n";
	ss << callee->printTree(indent, args.empty()).str();
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

const node_ptr_t CallNode::fold() const
{
	node_vec_t nargs;
	for (auto &c : args)
		nargs.push_back(c->fold());

	return std::make_unique<CallNode>(callee->fold(), nargs, token);
}

//------------------------------------------------------------------------------------------------------

ExternCallNode::ExternCallNode(
	const std::string &libname,
	const std::string &fname,
	const node_vec_t &args,
	const token_t &token) : Node(EXTERN_CALL_NODE,
		token),
	libname(libname),
	fname(fname),
	args(args)
{}

i_ptr_t ExternCallNode::genParser() const
{
	i_vec_t fargs;
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
	ss << "EXTERN_CALL : " << libname << "::" << fname << "\n";
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

const node_ptr_t ExternCallNode::fold() const
{
	node_vec_t nargs;
	for (auto &c : args)
		nargs.push_back(c->fold());

	return std::make_unique<ExternCallNode>(libname, fname, nargs, token);
}

//------------------------------------------------------------------------------------------------------

CallBuiltNode::CallBuiltNode(
	const LexerTokenType &t,
	const node_ptr_t &arg,
	const token_t &token) : Node(CALL_BUILT_NODE,
		token),
	t(t),
	arg(arg)
{}

i_ptr_t CallBuiltNode::genParser() const
{
	switch (t) {
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

	trace_t stack_trace;
	throw rossa_error(format::format(_UNKNOWN_BUILT_CALL_, { std::to_string(t) }), token, stack_trace);
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
	ss << "CALL_BUILT : " << std::to_string(t) << "\n";
	ss << arg->printTree(indent, true).str();
	return ss;
}

const node_ptr_t CallBuiltNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<CallBuiltNode>(t, arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ReturnNode::ReturnNode(
	const node_ptr_t &a,
	const token_t &token) : Node(REFER_NODE,
		token),
	a(a)
{}

i_ptr_t ReturnNode::genParser() const
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
	ss << "RETURN\n";
	ss << a->printTree(indent, true).str();
	return ss;
}

const node_ptr_t ReturnNode::fold() const
{
	return std::make_unique<ReturnNode>(a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ReferNode::ReferNode(
	const node_ptr_t &a,
	const token_t &token) : Node(RETURN_NODE,
		token),
	a(a)
{}

i_ptr_t ReferNode::genParser() const
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
	ss << "REFER\n";
	ss << a->printTree(indent, true).str();
	return ss;
}

const node_ptr_t ReferNode::fold() const
{
	return std::make_unique<ReferNode>(a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

BinOpNode::BinOpNode(
	const std::string &op,
	const node_ptr_t &a,
	const node_ptr_t &b,
	const token_t &token) : Node(BIN_OP_NODE,
		token),
	op(op),
	a(a),
	b(b)
{}

i_ptr_t BinOpNode::genParser() const
{
	trace_t stack_trace;

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
			throw rossa_error("Only variables may be declared with `:=`", token, stack_trace);
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

	throw rossa_error(format::format(_UNKNOWN_BINARY_OP_, { op }), token, stack_trace);
}

const std::string &BinOpNode::getOp() const
{
	return op;
}

const node_ptr_t BinOpNode::getA() const
{
	return a;
};

const node_ptr_t BinOpNode::getB() const
{
	return b;
};

void BinOpNode::setA(const node_ptr_t &a)
{
	this->a = (a);
}

void BinOpNode::setB(const node_ptr_t &b)
{
	this->b = (b);
}

bool BinOpNode::isConst() const
{
	if (a->isConst() && b->isConst()) {
		try {
			scope_t newScope(static_cast<hash_ull>(0));
			trace_t stack_trace;
			genParser()->evaluate(&newScope, stack_trace);
			return true;
		} catch (const rossa_error &e) {
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
	ss << "BINOP : " << op << "\n";
	ss << a->printTree(indent, false).str();
	ss << b->printTree(indent, true).str();
	return ss;
}

const node_ptr_t BinOpNode::fold() const
{
	if (isConst()) {
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto evalA = a->genParser()->evaluate(&newScope, stack_trace);
		auto evalB = b->genParser()->evaluate(&newScope, stack_trace);
		auto r = genParser()->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<BinOpNode>(op, a->fold(), b->fold(), token);
}

//------------------------------------------------------------------------------------------------------

UnOpNode::UnOpNode(
	const std::string &op,
	const node_ptr_t &a,
	const token_t &token) : Node(UN_OP_NODE,
		token),
	op(op),
	a(a)
{}

i_ptr_t UnOpNode::genParser() const
{
	if (op == "+")
		return std::make_shared<UnAddI>(a->genParser(), token);
	if (op == "-")
		return std::make_shared<NegI>(a->genParser(), token);
	if (op == "!")
		return std::make_shared<NotI>(a->genParser(), token);
	if (op == "$")
		return std::make_shared<TypeI>(a->genParser(), token);
	if (op == "~")
		return std::make_shared<BNotI>(a->genParser(), token);

	trace_t stack_trace;
	throw rossa_error(format::format(_UNKNOWN_UNARY_OP_, { op }), token, stack_trace);
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
	ss << "UNOP : " << op << "\n";
	ss << a->printTree(indent, true).str();
	return ss;
}

const node_ptr_t UnOpNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<UnOpNode>(op, a->fold(), token);
}

//------------------------------------------------------------------------------------------------------

ParenNode::ParenNode(
	const node_ptr_t &a,
	const token_t &token) : Node(PAREN_NODE,
		token),
	a(a)
{}

i_ptr_t ParenNode::genParser() const
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
	ss << "PAREN\n";
	ss << a->printTree(indent, true).str();
	return ss;
}

const node_ptr_t ParenNode::fold() const
{
	/*if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(newScope, stack_trace);
		newScope->clear();
		return std::make_unique<ContainerNode>(r, token);
	}*/

	return a->fold();
}

//------------------------------------------------------------------------------------------------------

InsNode::InsNode(
	const node_ptr_t &callee,
	const node_ptr_t &arg,
	const token_t &token) : Node(INS_NODE,
		token),
	callee(callee),
	arg(arg)
{}

i_ptr_t InsNode::genParser() const
{
	return std::make_shared<InnerI>(callee->genParser(), arg->genParser(), token);
}

const node_ptr_t InsNode::getCallee() const
{
	return (callee);
}

const node_ptr_t InsNode::getArg() const
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
	ss << "INS\n";
	ss << callee->printTree(indent, false).str();
	ss << arg->printTree(indent, true).str();
	return ss;
}

const node_ptr_t InsNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	return std::make_unique<InsNode>(callee->fold(), arg->fold(), token);
}

//------------------------------------------------------------------------------------------------------

IfElseNode::IfElseNode(
	const node_ptr_t &ifs,
	const node_ptr_t &body,
	const token_t &token) : Node(IF_ELSE_NODE,
		token),
	ifs(ifs),
	body(body)
{}

void IfElseNode::setElse(const node_ptr_t &elses)
{
	this->elses = (elses);
}

i_ptr_t IfElseNode::genParser() const
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
	ss << "IF_ELSE\n";
	ss << ifs->printTree(indent, false).str();
	ss << body->printTree(indent, elses == nullptr).str();
	if (elses != nullptr)
		ss << elses->printTree(indent, true).str();
	return ss;
}

const node_ptr_t IfElseNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	auto ret = std::make_unique<IfElseNode>(ifs->fold(), body->fold(), token);
	if (elses)
		ret->setElse(elses->fold());
	return ret;
}

//------------------------------------------------------------------------------------------------------

WhileNode::WhileNode(
	const node_ptr_t &whiles,
	const node_vec_t &body,
	const token_t &token) : Node(WHILE_NODE,
		token),
	whiles(whiles),
	body(body)
{}

i_ptr_t WhileNode::genParser() const
{
	i_vec_t is;
	for (auto &e : this->body)
		is.push_back(e->genParser());

	return std::make_shared<WhileI>(whiles->genParser(), is, token);
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
	ss << "WHILE\n";
	ss << whiles->printTree(indent, false).str();
	for (size_t i = 0; i < body.size(); i++)
		ss << body[i]->printTree(indent, i == body.size() - 1).str();
	return ss;
}

const node_ptr_t WhileNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	node_vec_t nbody;
	for (auto &c : body)
		nbody.push_back(c->fold());
	return std::make_unique<WhileNode>(whiles->fold(), nbody, token);
}

//------------------------------------------------------------------------------------------------------

ForNode::ForNode(
	const hash_ull &id,
	const node_ptr_t &fors,
	const node_vec_t &body,
	const token_t &token) : Node(FOR_NODE,
		token),
	id(id),
	fors(fors),
	body(body)
{}

i_ptr_t ForNode::genParser() const
{
	i_vec_t is;
	for (auto &e : this->body)
		is.push_back(e->genParser());

	return std::make_shared<ForI>(id, fors->genParser(), is, token);
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
	ss << "FOR : " << ROSSA_DEHASH(id) << "\n";
	ss << fors->printTree(indent, false).str();
	for (size_t i = 0; i < body.size(); i++)
		ss << body[i]->printTree(indent, i == body.size() - 1).str();
	return ss;
}

const node_ptr_t ForNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	node_vec_t nbody;
	for (auto &c : body)
		nbody.push_back(c->fold());
	return std::make_unique<ForNode>(id, fors->fold(), nbody, token);
}

//------------------------------------------------------------------------------------------------------

UntilNode::UntilNode(
	const node_ptr_t &a,
	const node_ptr_t &b,
	const node_ptr_t &step,
	const bool &inclusive,
	const token_t &token) : Node(UNTIL_NODE,
		token),
	a(a),
	b(b),
	step(step),
	inclusive(inclusive)
{}

i_ptr_t UntilNode::genParser() const
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
			scope_t newScope(static_cast<hash_ull>(0));
			trace_t stack_trace;
			i->evaluate(&newScope, stack_trace);
			return true;
		} catch (const rossa_error &e) {
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
	ss << "UNTIL\n";
	ss << a->printTree(indent, false).str();
	ss << b->printTree(indent, step == nullptr).str();
	if (step != nullptr)
		ss << step->printTree(indent, true).str();
	return ss;
}

const node_ptr_t UntilNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	if (step == nullptr)
		return std::make_unique<UntilNode>(a->fold(), b->fold(), nullptr, inclusive, token);
	else
		return std::make_unique<UntilNode>(a->fold(), b->fold(), step->fold(), inclusive, token);
}

//------------------------------------------------------------------------------------------------------

MapNode::MapNode(
	const std::vector<std::pair<std::string, node_ptr_t>> &args,
	const token_t &token) : Node(MAP_NODE,
		token),
	args(args)
{}

i_ptr_t MapNode::genParser() const
{
	std::map<std::string, i_ptr_t> is;
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
	ss << "MAP\n";
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i].second->printTree(indent, i == args.size() - 1).str();
	return ss;
}

const node_ptr_t MapNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	std::vector<std::pair<std::string, node_ptr_t>> nargs;
	for (auto &c : args)
		nargs.push_back({ c.first, c.second->fold() });
	return std::make_unique<MapNode>(nargs, token);
}

//------------------------------------------------------------------------------------------------------

SwitchNode::SwitchNode(
	const node_ptr_t &switchs,
	const std::map<node_ptr_t, size_t> &cases,
	const node_vec_t &gotos,
	const token_t &token) : Node(SWITCH_NODE,
		token),
	switchs(switchs),
	cases(cases),
	gotos(gotos)
{}

i_ptr_t SwitchNode::genParser() const
{
	std::map<sym_t, size_t> cases_solved;
	std::map<i_ptr_t, size_t> cases_unsolved;
	i_vec_t goto_cases;
	trace_t stack_trace;
	for (auto &e : this->cases) {
		if (e.first->isConst()) {
			scope_t newScope(static_cast<hash_ull>(0));
			trace_t stack_trace;
			auto key = e.first->genParser()->evaluate(&newScope, stack_trace);
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

void SwitchNode::setElse(const node_ptr_t &elses)
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
	ss << "SWITCH\n";
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

const node_ptr_t SwitchNode::fold() const
{
	if (isConst()) {
		auto i = genParser();
		scope_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_unique<ContainerNode>(r, token);
	}

	std::map<node_ptr_t, size_t> ncases;
	node_vec_t ngotos;
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
	const node_ptr_t &trys,
	const node_ptr_t &catchs,
	const hash_ull &key,
	const token_t &token) : Node(TRY_CATCH_NODE,
		token),
	trys(trys),
	catchs(catchs),
	key(key)
{}

i_ptr_t TryCatchNode::genParser() const
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
	ss << "TRY_CATCH : " << ROSSA_DEHASH(key) << "\n";
	ss << trys->printTree(indent, false).str();
	ss << catchs->printTree(indent, true).str();
	return ss;
}

const node_ptr_t TryCatchNode::fold() const
{
	return std::make_unique<TryCatchNode>(trys->fold(), catchs->fold(), key, token);
}

//------------------------------------------------------------------------------------------------------

ThrowNode::ThrowNode(
	const node_ptr_t &throws,
	const token_t &token) : Node(THROW_NODE, token),
	throws(throws)
{}

i_ptr_t ThrowNode::genParser() const
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
	ss << "THROW\n";
	ss << throws->printTree(indent, true).str();
	return ss;
}

const node_ptr_t ThrowNode::fold() const
{
	return std::make_unique<ThrowNode>(throws->fold(), token);
}

//------------------------------------------------------------------------------------------------------

CallOpNode::CallOpNode(
	const size_t &id,
	const node_vec_t &args,
	const token_t &token) : Node(CALL_OP_NODE,
		token),
	id(id),
	args(args)
{}

i_ptr_t CallOpNode::genParser() const
{
	i_vec_t fargs;
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
	ss << "CALL_OP : " << id << "\n";
	for (size_t i = 0; i < args.size(); i++)
		ss << args[i]->printTree(indent, i == args.size() - 1).str();
	return ss;
}

const node_ptr_t CallOpNode::fold() const
{
	node_vec_t nargs;
	for (auto &c : args)
		nargs.push_back(c->fold());

	return std::make_unique<CallOpNode>(id, nargs, token);
}

//------------------------------------------------------------------------------------------------------

DeleteNode::DeleteNode(
	const node_ptr_t &del,
	const token_t &token) : Node(DELETE_NODE, token),
	del(del)
{}

i_ptr_t DeleteNode::genParser() const
{
	return std::make_shared<DeleteI>(del->genParser(), token);
}

bool DeleteNode::isConst() const
{
	return false;
}

std::stringstream DeleteNode::printTree(std::string indent, bool last) const
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
	ss << "DELETE\n";
	ss << del->printTree(indent, true).str();
	return ss;
}

const node_ptr_t DeleteNode::fold() const
{
	return std::make_unique<DeleteNode>(del->fold(), token);
}