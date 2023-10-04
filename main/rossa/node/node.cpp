#include "node.h"

#include "../instruction/instruction.h"
#include "../global/global.h"
#include "../object/object.h"
#include "../parser/parser.h"

Node::Node(const std::vector<node_scope_t> &path, const type_t &type, const token_t &token)
	: path(path), type(type), token(token)
{
}

const Node::type_t Node::getType() const
{
	return type;
}

const token_t Node::getToken() const
{
	return token;
}

//------------------------------------------------------------------------------------------------------

ContainerNode::ContainerNode(const std::vector<node_scope_t> &path, const symbol_t &s, const token_t &token)
	: Node(path, CONTAINER_NODE, token), s(s)
{
}

ptr_instruction_t ContainerNode::genParser() const
{
	return std::make_shared<ContainerI>(s, token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "CONTAINER : " << s.toCodeString() << "\n";
}

const ptr_node_t ContainerNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<ContainerNode>(path, s, token);
}

//------------------------------------------------------------------------------------------------------

VectorNode::VectorNode(
	const std::vector<node_scope_t> &path,
	const std::vector<ptr_node_t> &args,
	const bool &scoped,
	const token_t &token) : Node(path, VECTOR_NODE,
								 token),
							args(args),
							scoped(scoped)
{
}

ptr_instruction_t VectorNode::genParser() const
{
	std::vector<ptr_instruction_t> ins;
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "ARRAY : " << scoped << "\n";
	for (size_t i = 0; i < args.size(); i++)
		if (args[i] != nullptr)
			args[i]->printTree(indent, i == args.size() - 1);
}

const ptr_node_t VectorNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (isConst())
	{
		auto i = genParser();
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	std::vector<ptr_node_t> nargs;
	for (auto &c : args)
		if (c != nullptr)
			nargs.push_back(c->fold(consts));
	return std::make_shared<VectorNode>(path, nargs, scoped, token);
}

const std::vector<ptr_node_t> &VectorNode::getChildren()
{
	return args;
}

//------------------------------------------------------------------------------------------------------

BreakNode::BreakNode(
	const std::vector<node_scope_t> &path,
	const token_t &token) : Node(path, BREAK_NODE,
								 token)
{
}

ptr_instruction_t BreakNode::genParser() const
{
	return std::make_shared<ContainerI>(symbol_t(symbol_t::type_t::ID_BREAK), token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "BREAK\n";
}

const ptr_node_t BreakNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<ContainerNode>(path, symbol_t(symbol_t::type_t::ID_BREAK), token);
}

//------------------------------------------------------------------------------------------------------

ContinueNode::ContinueNode(
	const std::vector<node_scope_t> &path,
	const token_t &token) : Node(path, CONTINUE_NODE,
								 token)
{
}

ptr_instruction_t ContinueNode::genParser() const
{
	return std::make_shared<ContainerI>(symbol_t(symbol_t::type_t::ID_CONTINUE), token);
}

bool ContinueNode::isConst() const
{
	return true;
}

void ContinueNode::printTree(std::string indent, bool last) const
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "CONTINUE\n";
}

const ptr_node_t ContinueNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<ContainerNode>(path, symbol_t(symbol_t::type_t::ID_CONTINUE), token);
}

//------------------------------------------------------------------------------------------------------

IDNode::IDNode(
	const std::vector<node_scope_t> &path,
	const hash_ull &key,
	const token_t &token) : Node(path, ID_NODE,
								 token),
							key(key)
{
}

hash_ull IDNode::getKey() const
{
	return key;
}

ptr_instruction_t IDNode::genParser() const
{
	if (key == parser_t::HASH_THIS)
		return std::make_shared<GetThisI>(token);
	return std::make_shared<VariableI>(key, token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "ID : " << ROSSA_DEHASH(key) << "\n";
}

const ptr_node_t IDNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	bool flag = true;
	for (auto &p : path)
	{
		if (std::find(p.var_ids.begin(), p.var_ids.end(), key) != p.var_ids.end())
		{
			flag = false;
			break;
		}
	}

	if (flag)
	{
		for (auto &c : consts)
		{
			std::vector<hash_ull> tpath;
			for (auto &p : path)
				tpath.push_back(p.id);
			while (true)
			{
				auto temp = tpath;
				temp.push_back({key});

				if (c.first == temp)
					return std::make_shared<ContainerNode>(path, c.second, token);
				if (tpath.empty())
					break;
				tpath.pop_back();
			}
		}
	}

	return std::make_shared<IDNode>(path, key, token);
}

//------------------------------------------------------------------------------------------------------

BIDNode::BIDNode(
	const std::vector<node_scope_t> &path,
	const std::string &key,
	const token_t &token) : Node(path, BID_NODE,
								 token),
							key(key)
{
}

const std::string BIDNode::getKey() const
{
	return key;
}

ptr_instruction_t BIDNode::genParser() const
{
	return std::make_shared<VariableI>(ROSSA_HASH(key), token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "BID : " << key << "\n";
}

const ptr_node_t BIDNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<BIDNode>(path, key, token);
}

//------------------------------------------------------------------------------------------------------

DefineNode::DefineNode(
	const std::vector<node_scope_t> &path,
	const hash_ull &key,
	const signature_t &ftype,
	const std::vector<std::pair<token_type_enum, hash_ull>> &params,
	const ptr_node_t &body,
	const std::vector<hash_ull> &captures,
	const token_t &token) : Node(path, DEFINE_NODE,
								 token),
							key(key),
							ftype(ftype),
							params(params),
							body(body),
							captures(captures)
{
}

ptr_instruction_t DefineNode::genParser() const
{
	return std::make_shared<DefineI>(key, ftype, params, body->genParser(), captures, token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "DEFINE : " << (key > 0 ? ROSSA_DEHASH(key) : "<LAMBDA>") << ", " << ftype.toString() << "\n";
	body->printTree(indent, true);
}

const ptr_node_t DefineNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (isConst())
	{
		auto i = genParser();
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	return std::make_shared<DefineNode>(path, key, ftype, params, body->fold(consts), captures, token);
}

//------------------------------------------------------------------------------------------------------

VargDefineNode::VargDefineNode(
	const std::vector<node_scope_t> &path,
	const hash_ull &key,
	const ptr_node_t &body,
	const std::vector<hash_ull> &captures,
	const token_t &token) : Node(path, VARG_DEFINE_NODE,
								 token),
							key(key),
							body(body),
							captures(captures)
{
}

ptr_instruction_t VargDefineNode::genParser() const
{
	return std::make_shared<VargDefineI>(key, body->genParser(), captures, token);
}

bool VargDefineNode::isConst() const
{
	return false;
}

void VargDefineNode::printTree(std::string indent, bool last) const
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "DEFINE : " << (key > 0 ? ROSSA_DEHASH(key) : "<LAMBDA>") << "\n";
	body->printTree(indent, true);
}

const ptr_node_t VargDefineNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (isConst())
	{
		auto i = genParser();
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	return std::make_shared<VargDefineNode>(path, key, body->fold(consts), captures, token);
}

//------------------------------------------------------------------------------------------------------

NewNode::NewNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &object,
	const ptr_node_t &params,
	const token_t &token) : Node(path, NEW_NODE,
								 token),
							object(object),
							params(params)
{
}

ptr_instruction_t NewNode::genParser() const
{
	auto paramsI = params->genParser();
	return std::make_shared<NewI>(object->genParser(), paramsI, token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "NEW\n";
	object->printTree(indent, false);
	params->printTree(indent, true);
}

const ptr_node_t NewNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<NewNode>(path, object->fold(consts), params->fold(consts), token);
}

//------------------------------------------------------------------------------------------------------

ClassNode::ClassNode(
	const std::vector<node_scope_t> &path,
	const hash_ull &key,
	const int &type,
	const std::vector<ptr_node_t> &body,
	const ptr_node_t &extends,
	const token_t &token) : Node(path, CLASS_NODE,
								 token),
							key(key),
							type(type),
							body(body),
							extends(extends)
{
}

ptr_instruction_t ClassNode::genParser() const
{
	std::vector<ptr_instruction_t> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = std::make_shared<ScopeI>(is, token);
	trace_t stack_trace;

	scope_type_enum ot;
	switch (type)
	{
	case TOK_STRUCT:
		ot = scope_type_enum::SCOPE_STRUCT;
		break;
	case TOK_STATIC:
		ot = scope_type_enum::SCOPE_STATIC;
		break;
	case TOK_VIRTUAL:
		ot = scope_type_enum::SCOPE_VIRTUAL;
		break;
	default:
		throw rossa_error_t(_INVALID_OBJECT_TYPE_, token, stack_trace);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "CLASS : " << ROSSA_DEHASH(key) << ", " << std::to_string(type) << "\n";
	if (extends != nullptr)
		extends->printTree(indent, false);
	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

const ptr_node_t ClassNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	std::vector<ptr_node_t> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold(consts));

	if (extends)
		return std::make_shared<ClassNode>(path, key, type, nbody, extends->fold(consts), token);
	else
		return std::make_shared<ClassNode>(path, key, type, nbody, nullptr, token);
}

//------------------------------------------------------------------------------------------------------

VarNode::VarNode(
	const std::vector<node_scope_t> &path,
	const std::vector<hash_ull> &keys,
	const token_t &token) : Node(path, VAR_NODE,
								 token),
							keys(keys)
{
}

ptr_instruction_t VarNode::genParser() const
{
	return std::make_shared<DeclareVarsI>(keys, token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "VAR : " << keys.size() << "\n";
}

const ptr_node_t VarNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<VarNode>(path, keys, token);
}

//------------------------------------------------------------------------------------------------------

CallNode::CallNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &callee,
	const std::vector<ptr_node_t> &args,
	const token_t &token) : Node(path, CALL_NODE,
								 token),
							callee(callee),
							args(args)
{
}

ptr_instruction_t CallNode::genParser() const
{
	auto fcallee = callee->genParser();
	std::vector<ptr_instruction_t> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	if (fcallee->getType() == INNER) {
		return std::make_shared<CallWithInnerI>(fcallee, std::make_shared<SequenceI>(fargs, token), token);
	}
	return std::make_shared<CallI>(fcallee, std::make_shared<SequenceI>(fargs, token), token);
}

ptr_node_t CallNode::getCallee() const
{
	return (callee);
}

std::vector<ptr_node_t> CallNode::getArgs() const
{
	return (args);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "CALL\n";
	callee->printTree(indent, args.empty());
	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

const ptr_node_t CallNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	std::vector<ptr_node_t> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold(consts));

	return std::make_shared<CallNode>(path, callee->fold(consts), nargs, token);
}

//------------------------------------------------------------------------------------------------------

ExternCallNode::ExternCallNode(
	const std::vector<node_scope_t> &path,
	const std::string &libname,
	const std::string &fname,
	const std::vector<ptr_node_t> &args,
	const token_t &token) : Node(path, EXTERN_CALL_NODE,
								 token),
							libname(libname),
							fname(fname),
							args(args)
{
}

ptr_instruction_t ExternCallNode::genParser() const
{
	std::vector<ptr_instruction_t> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<ExternI>(libname, fname, std::make_shared<SequenceI>(fargs, token), token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "EXTERN_CALL : " << libname << "::" << fname << "\n";
	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

const ptr_node_t ExternCallNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	std::vector<ptr_node_t> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold(consts));

	return std::make_shared<ExternCallNode>(path, libname, fname, nargs, token);
}

//------------------------------------------------------------------------------------------------------

CallBuiltNode::CallBuiltNode(
	const std::vector<node_scope_t> &path,
	const token_type_enum &t,
	const std::vector<ptr_node_t> &args,
	const token_t &token) : Node(path, CALL_BUILT_NODE,
								 token),
							t(t),
							args(args)
{
}
CallBuiltNode::CallBuiltNode(
	const std::vector<node_scope_t> &path,
	const token_type_enum &t,
	const ptr_node_t &arg,
	const token_t &token) : Node(path, CALL_BUILT_NODE,
								 token),
							t(t),
							args({arg})
{
}

ptr_instruction_t CallBuiltNode::genParser() const
{
	trace_t stack_trace;
	switch (t)
	{
	case TOK_LENGTH:
		if (args.size() > 1)
		{
			throw rossa_error_t(global::format(_TOO_MANY_ARGUMENTS_, {KEYWORD_LENGTH}), token, stack_trace);
		}
		return std::make_shared<LengthI>(args[0]->genParser(), token);
	case TOK_ALLOC:
		if (args.size() > 2)
		{
			throw rossa_error_t(global::format(_TOO_MANY_ARGUMENTS_, {KEYWORD_ALLOC}), token, stack_trace);
		}
		return std::make_shared<AllocI>(args[0]->genParser(), args.size() > 1 ? args[1]->genParser() : nullptr, token);
	case TOK_PARSE:
		if (args.size() > 1)
		{
			throw rossa_error_t(global::format(_TOO_MANY_ARGUMENTS_, {KEYWORD_PARSE}), token, stack_trace);
		}
		return std::make_shared<ParseI>(args[0]->genParser(), token);
	case TOK_CHARN:
		if (args.size() > 1)
		{
			throw rossa_error_t(global::format(_TOO_MANY_ARGUMENTS_, {KEYWORD_CHAR_N}), token, stack_trace);
		}
		return std::make_shared<CharNI>(args[0]->genParser(), token);
	case TOK_CHARS:
		if (args.size() > 1)
		{
			throw rossa_error_t(global::format(_TOO_MANY_ARGUMENTS_, {KEYWORD_CHAR_S}), token, stack_trace);
		}
		return std::make_shared<CharSI>(args[0]->genParser(), token);
	default:
		break;
	}

	throw rossa_error_t(global::format(_UNKNOWN_BUILT_CALL_, {std::to_string(t)}), token, stack_trace);
	return nullptr;
}

bool CallBuiltNode::isConst() const
{
	for (auto &arg : args)
	{
		if (!arg->isConst())
		{
			return false;
		}
	}
	return true;
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "CALL_BUILT : " << std::to_string(t) << "\n";
	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

const ptr_node_t CallBuiltNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (isConst())
	{
		auto i = genParser();
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	std::vector<ptr_node_t> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold(consts));

	return std::make_shared<CallBuiltNode>(path, t, nargs, token);
}

//------------------------------------------------------------------------------------------------------

ReturnNode::ReturnNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &a,
	const token_t &token) : Node(path, REFER_NODE,
								 token),
							a(a)
{
}

ptr_instruction_t ReturnNode::genParser() const
{
	return std::make_shared<ReturnI>(a->genParser(), token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "RETURN\n";
	a->printTree(indent, true);
}

const ptr_node_t ReturnNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<ReturnNode>(path, a->fold(consts), token);
}

//------------------------------------------------------------------------------------------------------

ReferNode::ReferNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &a,
	const token_t &token) : Node(path, RETURN_NODE,
								 token),
							a(a)
{
}

ptr_instruction_t ReferNode::genParser() const
{
	return std::make_shared<ReferI>(a->genParser(), token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "REFER\n";
	a->printTree(indent, true);
}

const ptr_node_t ReferNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<ReferNode>(path, a->fold(consts), token);
}

//------------------------------------------------------------------------------------------------------

BinOpNode::BinOpNode(
	const std::vector<node_scope_t> &path,
	const std::string &op,
	const ptr_node_t &a,
	const ptr_node_t &b,
	const token_t &token) : Node(path, BIN_OP_NODE,
								 token),
							op(op),
							a(a),
							b(b)
{
}

ptr_instruction_t BinOpNode::genParser() const
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
	if (op == "//")
		return std::make_shared<FDivI>(a->genParser(), b->genParser(), token);
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
	if (op == "++")
		return std::make_shared<ConcatI>(a->genParser(), b->genParser(), token);

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

	if (op == ".=")
		return std::make_shared<SetIndexI>(a->genParser(), b->genParser(), token);

	if (op == "=")
	{
		if (a->isConst())
			throw rossa_error_t("Cannot reassign constant value", token, stack_trace);
		return std::make_shared<SetI>(a->genParser(), b->genParser(), token);
	}
	if (op == ":=")
	{
		if (a->getType() != ID_NODE && a->getType() != BID_NODE)
			throw rossa_error_t("Only non-const variables may be declared with `:=`", token, stack_trace);
		hash_ull t;
		if (a->getType() == ID_NODE)
			t = ((IDNode *)a.get())->getKey();
		else
			t = ROSSA_HASH(((BIDNode *)a.get())->getKey());
		return std::make_shared<DeclareI>(t, b->genParser(), b->isConst(), token);
	}

	if (op == "[]")
		return std::make_shared<IndexI>(a->genParser(), b->genParser(), token);
	if (op == "->")
		return std::make_shared<CastToI>(a->genParser(), b->genParser(), token);
	if (op == "delete")
		return std::make_shared<DeleteI>(a->genParser(), b->genParser(), token);

	throw rossa_error_t(global::format(_UNKNOWN_BINARY_OP_, {op}), token, stack_trace);
}

const std::string &BinOpNode::getOp() const
{
	return op;
}

const ptr_node_t BinOpNode::getA() const
{
	return a;
};

const ptr_node_t BinOpNode::getB() const
{
	return b;
};

void BinOpNode::setA(const ptr_node_t &a)
{
	this->a = (a);
}

void BinOpNode::setB(const ptr_node_t &b)
{
	this->b = (b);
}

bool BinOpNode::isConst() const
{
	if (a->isConst() && b->isConst())
	{
		try
		{
			object_t newScope(static_cast<hash_ull>(0));
			trace_t stack_trace;
			genParser()->evaluate(&newScope, stack_trace);
			return true;
		}
		catch (const rossa_error_t &e)
		{
			return false;
		}
	}
	return false;
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "BINOP : " << op << "\n";
	a->printTree(indent, false);
	b->printTree(indent, true);
}

const ptr_node_t BinOpNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (op == "+=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "+", a, b, token), token)->fold(consts);
	if (op == "-=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "-", a, b, token), token)->fold(consts);
	if (op == "*=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "*", a, b, token), token)->fold(consts);
	if (op == "/=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "/", a, b, token), token)->fold(consts);
	if (op == "//=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "//", a, b, token), token)->fold(consts);
	if (op == "%=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "%", a, b, token), token)->fold(consts);
	if (op == "**=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "**", a, b, token), token)->fold(consts);
	if (op == "|=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "|", a, b, token), token)->fold(consts);
	if (op == "&=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "&", a, b, token), token)->fold(consts);
	if (op == "^=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "^", a, b, token), token)->fold(consts);
	if (op == "<<=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "<<", a, b, token), token)->fold(consts);
	if (op == ">>=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, ">>", a, b, token), token)->fold(consts);
	if (op == "++=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "++", a, b, token), token)->fold(consts);
	if (op == "&&=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "&&", a, b, token), token)->fold(consts);
	if (op == "||=")
		return std::make_shared<BinOpNode>(path, "=", a, std::make_shared<BinOpNode>(path, "||", a, b, token), token)->fold(consts);

	auto na = a->fold(consts);
	auto nb = b->fold(consts);

	bool constmod = false;

	if (a->isConst() && b->isConst())
	{
		try
		{
			object_t newScope(static_cast<hash_ull>(0));
			trace_t stack_trace;
			genParser()->evaluate(&newScope, stack_trace);
			constmod = true;
		}
		catch (const rossa_error_t &e)
		{
		}
	}

	if (constmod)
	{
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto evalA = na->genParser()->evaluate(&newScope, stack_trace);
		auto evalB = nb->genParser()->evaluate(&newScope, stack_trace);
		auto r = genParser()->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	return std::make_shared<BinOpNode>(path, op, na, b->fold(consts), token);
}

//------------------------------------------------------------------------------------------------------

UnOpNode::UnOpNode(
	const std::vector<node_scope_t> &path,
	const std::string &op,
	const ptr_node_t &a,
	const token_t &token) : Node(path, UN_OP_NODE,
								 token),
							op(op),
							a(a)
{
}

ptr_instruction_t UnOpNode::genParser() const
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
	if (op == "@")
		return std::make_shared<HashI>(a->genParser(), token);

	trace_t stack_trace;
	throw rossa_error_t(global::format(_UNKNOWN_UNARY_OP_, {op}), token, stack_trace);
	return nullptr;
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "UNOP : " << op << "\n";
	a->printTree(indent, true);
}

const ptr_node_t UnOpNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	auto na = a->fold(consts);
	auto ru = std::make_shared<UnOpNode>(path, op, na, token);
	if (na->isConst())
	{
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = ru->genParser()->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	return ru;
}

//------------------------------------------------------------------------------------------------------

ParenNode::ParenNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &a,
	const token_t &token) : Node(path, PAREN_NODE,
								 token),
							a(a)
{
}

ptr_instruction_t ParenNode::genParser() const
{
	return a->genParser();
}

bool ParenNode::isConst() const
{
	return a->isConst();
}

void ParenNode::printTree(std::string indent, bool last) const
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "PAREN\n";
	a->printTree(indent, true);
}

const ptr_node_t ParenNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return a->fold(consts);
}

//------------------------------------------------------------------------------------------------------

InsNode::InsNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &callee,
	const ptr_node_t &arg,
	const token_t &token) : Node(path, INS_NODE,
								 token),
							callee(callee),
							arg(arg)
{
}

ptr_instruction_t InsNode::genParser() const
{
	return std::make_shared<InnerI>(callee->genParser(), arg->genParser(), token);
}

const ptr_node_t InsNode::getCallee() const
{
	return (callee);
}

const ptr_node_t InsNode::getArg() const
{
	return (arg);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "INS\n";
	callee->printTree(indent, false);
	arg->printTree(indent, true);
}

const ptr_node_t InsNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (arg->getType() == ID_NODE)
	{
		std::vector<hash_ull> apath;
		auto curr = callee;
		while (true)
		{
			if (curr->getType() == ID_NODE)
			{
				apath.push_back(reinterpret_cast<IDNode *>(curr.get())->getKey());
				break;
			}
			else if (curr->getType() == INS_NODE)
			{
				InsNode *inn = reinterpret_cast<InsNode *>(curr.get());
				if (inn->getArg()->getType() == ID_NODE)
				{
					apath.push_back(reinterpret_cast<IDNode *>(inn->getArg().get())->getKey());
					curr = inn->getCallee();
				}
				else
				{
					return std::make_shared<InsNode>(path, callee->fold(consts), arg->fold(consts), token);
				}
			}
			else
			{
				return std::make_shared<InsNode>(path, callee->fold(consts), arg->fold(consts), token);
			}
		}
		std::vector<hash_ull> fpath;
		while (!apath.empty())
		{
			fpath.push_back(apath.back());
			apath.pop_back();
		}
		fpath.push_back(reinterpret_cast<IDNode *>(arg.get())->getKey());

		for (auto &c : consts)
		{
			std::vector<hash_ull> tpath;
			for (auto &p : path)
				tpath.push_back(p.id);
			while (true)
			{
				auto temp = tpath;
				for (auto &p : fpath)
					temp.push_back(p);

				if (c.first == temp)
					return std::make_shared<ContainerNode>(path, c.second, token);
				if (tpath.empty())
					break;
				tpath.pop_back();
			}
		}
	}

	return std::make_shared<InsNode>(path, callee->fold(consts), arg->fold(consts), token);
}

//------------------------------------------------------------------------------------------------------

IfElseNode::IfElseNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &ifs,
	const ptr_node_t &body,
	const token_t &token) : Node(path, IF_ELSE_NODE,
								 token),
							ifs(ifs),
							body(body)
{
}

void IfElseNode::setElse(const ptr_node_t &elses)
{
	this->elses = (elses);
}

ptr_instruction_t IfElseNode::genParser() const
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "IF_ELSE\n";
	ifs->printTree(indent, false);
	body->printTree(indent, elses == nullptr);
	if (elses != nullptr)
		elses->printTree(indent, true);
}

const ptr_node_t IfElseNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	auto nifs = ifs->fold(consts);
	auto nbody = body->fold(consts);
	ptr_node_t nelses = nullptr;
	if (elses)
		nelses = elses->fold(consts);

	if (nifs->isConst() && nbody->isConst() && (!nelses || nelses->isConst()))
	{
		auto i = genParser();
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	auto ret = std::make_shared<IfElseNode>(path, nifs, nbody, token);
	if (nelses)
		ret->setElse(nelses);
	return ret;
}

//------------------------------------------------------------------------------------------------------

WhileNode::WhileNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &whiles,
	const std::vector<ptr_node_t> &body,
	const token_t &token) : Node(path, WHILE_NODE,
								 token),
							whiles(whiles),
							body(body)
{
}

ptr_instruction_t WhileNode::genParser() const
{
	std::vector<ptr_instruction_t> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());

	return std::make_shared<WhileI>(whiles->genParser(), is, token);
}

bool WhileNode::isConst() const
{
	return false;
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "WHILE\n";
	whiles->printTree(indent, false);
	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

const ptr_node_t WhileNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	std::vector<ptr_node_t> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold(consts));
	return std::make_shared<WhileNode>(path, whiles->fold(consts), nbody, token);
}

//------------------------------------------------------------------------------------------------------

ForNode::ForNode(
	const std::vector<node_scope_t> &path,
	const hash_ull &id,
	const ptr_node_t &fors,
	const std::vector<ptr_node_t> &body,
	const token_t &token) : Node(path, FOR_NODE,
								 token),
							id(id),
							fors(fors),
							body(body)
{
}

ptr_instruction_t ForNode::genParser() const
{
	std::vector<ptr_instruction_t> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());

	return std::make_shared<ForI>(id, fors->genParser(), is, token);
}

bool ForNode::isConst() const
{
	return false;
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "FOR : " << ROSSA_DEHASH(id) << "\n";
	fors->printTree(indent, false);
	for (size_t i = 0; i < body.size(); i++)
		body[i]->printTree(indent, i == body.size() - 1);
}

const ptr_node_t ForNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	std::vector<ptr_node_t> nbody;
	for (auto &c : body)
		nbody.push_back(c->fold(consts));
	return std::make_shared<ForNode>(path, id, fors->fold(consts), nbody, token);
}

//------------------------------------------------------------------------------------------------------

UntilNode::UntilNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &a,
	const ptr_node_t &b,
	const ptr_node_t &step,
	const bool &inclusive,
	const token_t &token) : Node(path, UNTIL_NODE,
								 token),
							a(a),
							b(b),
							step(step),
							inclusive(inclusive)
{
}

ptr_instruction_t UntilNode::genParser() const
{
	if (step == nullptr)
		return std::make_shared<UntilI>(a->genParser(), b->genParser(), nullptr, inclusive, token);
	else
		return std::make_shared<UntilI>(a->genParser(), b->genParser(), step->genParser(), inclusive, token);
}

bool UntilNode::isConst() const
{
	return false;
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "UNTIL\n";
	a->printTree(indent, false);
	b->printTree(indent, step == nullptr);
	if (step != nullptr)
		step->printTree(indent, true);
}

const ptr_node_t UntilNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (step == nullptr)
		return std::make_shared<UntilNode>(path, a->fold(consts), b->fold(consts), nullptr, inclusive, token);
	else
		return std::make_shared<UntilNode>(path, a->fold(consts), b->fold(consts), step->fold(consts), inclusive, token);
}

//------------------------------------------------------------------------------------------------------

MapNode::MapNode(
	const std::vector<node_scope_t> &path,
	const std::vector<std::pair<std::string, ptr_node_t>> &args,
	const token_t &token) : Node(path, MAP_NODE,
								 token),
							args(args)
{
}

ptr_instruction_t MapNode::genParser() const
{
	std::map<std::string, ptr_instruction_t> is;
	for (auto &e : this->args)
	{
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "MAP\n";
	for (size_t i = 0; i < args.size(); i++)
		args[i].second->printTree(indent, i == args.size() - 1);
}

const ptr_node_t MapNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (isConst())
	{
		auto i = genParser();
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	std::vector<std::pair<std::string, ptr_node_t>> nargs;
	for (auto &c : args)
		nargs.push_back({c.first, c.second->fold(consts)});
	return std::make_shared<MapNode>(path, nargs, token);
}

//------------------------------------------------------------------------------------------------------

SwitchNode::SwitchNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &switchs,
	const std::map<ptr_node_t, size_t> &cases,
	const std::vector<ptr_node_t> &gotos,
	const token_t &token) : Node(path, SWITCH_NODE,
								 token),
							switchs(switchs),
							cases(cases),
							gotos(gotos)
{
}

ptr_instruction_t SwitchNode::genParser() const
{
	std::map<symbol_t, size_t> cases_solved;
	std::map<ptr_instruction_t, size_t> cases_unsolved;
	std::vector<ptr_instruction_t> goto_cases;
	trace_t stack_trace;
	for (auto &e : this->cases)
	{
		if (e.first->isConst())
		{
			object_t newScope(static_cast<hash_ull>(0));
			trace_t stack_trace;
			auto key = e.first->genParser()->evaluate(&newScope, stack_trace);
			cases_solved[key] = e.second;
		}
		else
		{
			cases_unsolved[e.first->genParser()] = e.second;
		}
	}
	for (auto &e : this->gotos)
	{
		goto_cases.push_back(e->genParser());
	}
	if (elses)
		return std::make_shared<SwitchI>(switchs->genParser(), cases_solved, cases_unsolved, goto_cases, elses->genParser(), token);
	return std::make_shared<SwitchI>(switchs->genParser(), cases_solved, cases_unsolved, goto_cases, nullptr, token);
}

void SwitchNode::setElse(const ptr_node_t &elses)
{
	this->elses = (elses);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "SWITCH\n";
	switchs->printTree(indent, gotos.empty() && cases.empty());
	size_t i = 0;
	for (auto &e : cases)
	{
		e.first->printTree(indent, i == (cases.size() + gotos.size()) - 1 && !elses);
		i++;
	}
	for (auto &e : gotos)
	{
		e->printTree(indent, i == (cases.size() + gotos.size()) - 1 && !elses);
		i++;
	}
	if (elses)
		elses->printTree(indent, true);
}

const ptr_node_t SwitchNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	if (isConst())
	{
		auto i = genParser();
		object_t newScope(static_cast<hash_ull>(0));
		trace_t stack_trace;
		auto r = i->evaluate(&newScope, stack_trace);
		return std::make_shared<ContainerNode>(path, r, token);
	}

	std::map<ptr_node_t, size_t> ncases;
	std::vector<ptr_node_t> ngotos;
	for (auto &c : cases)
		ncases[c.first->fold(consts)] = c.second;
	for (auto &e : gotos)
		ngotos.push_back(e->fold(consts));
	auto ret = std::make_shared<SwitchNode>(path, switchs->fold(consts), ncases, ngotos, token);
	if (elses)
		ret->setElse(elses->fold(consts));
	return ret;
}

//------------------------------------------------------------------------------------------------------

TryCatchNode::TryCatchNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &trys,
	const ptr_node_t &catchs,
	const hash_ull &key,
	const token_t &token) : Node(path, TRY_CATCH_NODE,
								 token),
							trys(trys),
							catchs(catchs),
							key(key)
{
}

ptr_instruction_t TryCatchNode::genParser() const
{
	return std::make_shared<TryCatchI>(trys->genParser(), catchs->genParser(), key, token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "TRY_CATCH : " << ROSSA_DEHASH(key) << "\n";
	trys->printTree(indent, false);
	catchs->printTree(indent, true);
}

const ptr_node_t TryCatchNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<TryCatchNode>(path, trys->fold(consts), catchs->fold(consts), key, token);
}

//------------------------------------------------------------------------------------------------------

ThrowNode::ThrowNode(
	const std::vector<node_scope_t> &path,
	const ptr_node_t &throws,
	const token_t &token) : Node(path, THROW_NODE, token),
							throws(throws)
{
}

ptr_instruction_t ThrowNode::genParser() const
{
	return std::make_shared<ThrowI>(throws->genParser(), token);
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "THROW\n";
	throws->printTree(indent, true);
}

const ptr_node_t ThrowNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<ThrowNode>(path, throws->fold(consts), token);
}

//------------------------------------------------------------------------------------------------------

CallOpNode::CallOpNode(
	const std::vector<node_scope_t> &path,
	const size_t &id,
	const std::vector<ptr_node_t> &args,
	const token_t &token) : Node(path, CALL_OP_NODE,
								 token),
							id(id),
							args(args)
{
}

ptr_instruction_t CallOpNode::genParser() const
{
	std::vector<ptr_instruction_t> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return std::make_shared<CallOpI>(id, fargs, token);
}

bool CallOpNode::isConst() const
{
	return false;
}

void CallOpNode::printTree(std::string indent, bool last) const
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "CALL_OP : " << id << "\n";
	for (size_t i = 0; i < args.size(); i++)
		args[i]->printTree(indent, i == args.size() - 1);
}

const ptr_node_t CallOpNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	std::vector<ptr_node_t> nargs;
	for (auto &c : args)
		nargs.push_back(c->fold(consts));

	return std::make_shared<CallOpNode>(path, id, nargs, token);
}

//------------------------------------------------------------------------------------------------------

EachNode::EachNode(
	const std::vector<node_scope_t> &path,
	const hash_ull &id,
	const ptr_node_t &eachs,
	const ptr_node_t &wheres,
	const ptr_node_t &body,
	const token_t &token) : Node(path, EACH_NODE,
								 token),
							id(id),
							eachs(eachs),
							wheres(wheres),
							body(body)
{
}

ptr_instruction_t EachNode::genParser() const
{
	return std::make_shared<EachI>(id, eachs->genParser(), wheres ? wheres->genParser() : nullptr, body ? body->genParser() : nullptr, token);
}

bool EachNode::isConst() const
{
	return false;
}

void EachNode::printTree(std::string indent, bool last) const
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
	printc(global::deHashVec(path) + " ", RED_TEXT);
	std::cout << "FOR : " << ROSSA_DEHASH(id) << "\n";
	eachs->printTree(indent, wheres == nullptr && body == nullptr);
	if (wheres)
		wheres->printTree(indent, body == nullptr);
	if (body)
		body->printTree(indent, true);
}

const ptr_node_t EachNode::fold(const std::vector<std::pair<std::vector<hash_ull>, symbol_t>> &consts) const
{
	return std::make_shared<EachNode>(path, id, eachs->fold(consts), wheres ? wheres->fold(consts) : nullptr, body ? body->fold(consts) : nullptr, token);
}