#include "../../bin/include/Rossa.h"

using namespace rossa;

Scope::Scope()
	: parent{ nullptr }
	, type{ SCOPE_O }
	, hashed_key{ 0 }
	//, name_trace{ {} }
{}

Scope::Scope(const std::shared_ptr<Scope> &parent, const hash_ull &key)
	: parent{ parent }
	, type{ SCOPE_O }
{
	traceName(key);
}

Scope::Scope(const std::shared_ptr<Scope> &parent, const ObjectType &type, const std::shared_ptr<Instruction> &body, const hash_ull &key, const std::shared_ptr<Scope> &ex, const std::vector<type_sll> &extensions)
	: parent{ parent }
	, type{ type }
	, body{ body }
{
	if (ex != NULL) {
		this->extensions = ex->extensions;
		this->extensions.push_back(ex->getHashedKey());
	} else {
		this->extensions = extensions;
	}
	traceName(key);
}

Scope::Scope(const std::shared_ptr<Scope> &parent, const ObjectType &type, const std::shared_ptr<Instruction> &body, const hash_ull &hashed_key, const std::vector<type_sll> &extensions, const std::vector<type_sll> &name_trace)
	: parent{ parent }
	, type{ type }
	, body{ body }
	, hashed_key{ hashed_key }
	, extensions{ extensions }
	, name_trace{ name_trace }
{}

void Scope::traceName(const hash_ull &key)
{
	if (parent != NULL)
		name_trace = parent->name_trace;
	if (key != 0) {
		std::string path = "";
		name_trace.push_back(key);
		size_t i = 0;
		for (auto &p : name_trace) {
			if (i++ > 0)
				path += ".";
			path += ROSSA_DEHASH(p);
		}
		hashed_key = ROSSA_HASH(path);
	} else {
		hashed_key = Rossa::HASH_BLANK;
	}
}

const Symbol Scope::instantiate(const sym_vec_t &params, const Token *token, trace_t &stack_trace) const
{
	if (type != STRUCT_O)
		throw RTError(_FAILURE_INSTANTIATE_OBJECT_, *token, stack_trace);

	auto o = std::make_shared<Scope>(parent, INSTANCE_O, body, hashed_key, extensions, name_trace);
	o->body->evaluate(o, stack_trace);
	auto d = Symbol(o);
	o->getVariable(Rossa::HASH_INIT, token, stack_trace).call(params, token, stack_trace);
	return d;
}

const hash_ull Scope::getHashedKey() const
{
	return hashed_key;
}

const Symbol &Scope::getVariable(const hash_ull &key, const Token *token, trace_t &stack_trace) const
{
	if (values.find(key) != values.end())
		return values.at(key);
	if (parent != NULL)
		return parent->getVariable(key, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_VARIABLE_ERROR_, { ROSSA_DEHASH(key) }), *token, stack_trace);
}

const Symbol &Scope::createVariable(const hash_ull &key, const Token *token)
{
	values[key] = Symbol();
	return values[key];
}

const Symbol &Scope::createVariable(const hash_ull &key, const Symbol &d, const Token *token)
{
	if (values.find(key) != values.end() && values[key].getValueType() == FUNCTION)
		values[key].addFunctions(&d, token);
	else
		values[key] = d;

	return values[key];
}

void Scope::clear()
{
	values.clear();
}

const std::shared_ptr<Scope> &Scope::getParent() const
{
	return this->parent;
}

const bool Scope::extendsObject(const type_sll &ex) const
{
	return std::find(extensions.begin(), extensions.end(), ex) != extensions.end();
}

const ObjectType Scope::getType() const
{
	return type;
}

const std::shared_ptr<Instruction> Scope::getBody() const
{
	return body;
}
const Symbol Scope::getThis(const Token *token, trace_t &stack_trace)
{
	if (type != SCOPE_O)
		return Symbol(shared_from_this());
	if (parent != NULL)
		return parent->getThis(token, stack_trace);

	throw RTError(format::format(_UNDECLARED_VARIABLE_ERROR_, { KEYWORD_THIS }), *token, stack_trace);
}

const bool Scope::hasValue(const hash_ull &key) const
{
	return values.find(key) != values.end();
}

Scope::~Scope()
{
	if (hasValue(Rossa::HASH_DELETER)) {
		trace_t stack_trace;
		values[Rossa::HASH_DELETER].call({}, NULL, stack_trace);
	}
}