#include "../../bin/include/Rossa.h"

Scope::Scope()
	: parent{ nullptr }
	, hashed_key{ 0 }
	, type{ SCOPE_O }
{}

Scope::Scope(const scope_ptr_t &parent, const hash_ull &key)
	: parent{ parent }
	, type{ SCOPE_O }
{
	traceName(key);
}

Scope::Scope(const scope_ptr_t &parent, const Scope::type_t &type, const i_ptr_t &body, const hash_ull &key, const scope_ptr_t &ex, const std::vector<type_sll> &extensions)
	: parent{ parent }
	, body{ body }
	, type{ type }
{
	if (ex != NULL) {
		this->extensions = ex->extensions;
		this->extensions.push_back(ex->getHashedKey());
	} else {
		this->extensions = extensions;
	}
	traceName(key);
}

Scope::Scope(const scope_ptr_t &parent, const Scope::type_t &type, const i_ptr_t &body, const hash_ull &hashed_key, const std::vector<type_sll> &extensions, const std::vector<type_sll> &name_trace)
	: parent{ parent }
	, body{ body }
	, hashed_key{ hashed_key }
	, extensions{ extensions }
	, name_trace{ name_trace }
	, type{ type }
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

const sym_t Scope::instantiate(const sym_vec_t &params, const token_t *token, trace_t &stack_trace) const
{
	if (type != STRUCT_O)
		throw RTError(_FAILURE_INSTANTIATE_OBJECT_, *token, stack_trace);

	auto o = std::make_shared<Scope>(parent, INSTANCE_O, body, hashed_key, extensions, name_trace);
	o->body->evaluate(o, stack_trace);
	auto d = sym_t::Object(o);
	o->getVariable(Rossa::HASH_INIT, token, stack_trace).call(params, token, stack_trace);
	return d;
}

const hash_ull Scope::getHashedKey() const
{
	return hashed_key;
}

const sym_t &Scope::getVariable(const hash_ull &key, const token_t *token, trace_t &stack_trace) const
{
	if (values.find(key) != values.end())
		return values.at(key);
	if (parent != NULL)
		return parent->getVariable(key, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_VARIABLE_ERROR_, { ROSSA_DEHASH(key) }), *token, stack_trace);
}

const sym_t &Scope::createVariable(const hash_ull &key, const token_t *token)
{
	values[key] = sym_t();
	return values[key];
}

const sym_t &Scope::createVariable(const hash_ull &key, const sym_t &d, const token_t *token)
{
	if (values.find(key) != values.end() && values[key].getValueType() == Value::type_t::FUNCTION)
		values[key].addFunctions(&d, token);
	else
		values[key] = d;

	return values[key];
}

void Scope::clear()
{
	values.clear();
}

const scope_ptr_t &Scope::getParent() const
{
	return this->parent;
}

const bool Scope::extendsObject(const type_sll &ex) const
{
	return std::find(extensions.begin(), extensions.end(), ex) != extensions.end();
}

const Scope::type_t Scope::getType() const
{
	return type;
}

const i_ptr_t Scope::getBody() const
{
	return body;
}
const sym_t Scope::getThis(const token_t *token, trace_t &stack_trace)
{
	if (type != SCOPE_O)
		return sym_t::Object(shared_from_this());
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