#include "../../bin/include/Rossa.h"

Scope::Scope(const type_t &type, Scope *parent, const i_ptr_t &body, const hash_ull &key)
	: type{ type }
	, parent{ parent }
	, body{ body }
{
	traceName(key);
}

Scope::Scope(Scope *parent, const aug_type_t &name_trace, const std::vector<aug_type_t> &extensions)
	: type{ INSTANCE_O }
	, parent{ parent }
//	, hashed_key{ key }
	, name_trace{ name_trace }
	, extensions{ extensions }
{}

scope_t::scope_t(Scope *scope, const type_t &type)
	: scope{ scope }
	, type{ type }
{
	if (type == STRONG)
		scope->references++;
}

scope_t::scope_t()
	: scope{ NULL }
	, type{ STRONG }
{}

scope_t::scope_t(const hash_ull &key)
	: scope{ new Scope(Scope::SCOPE_O, NULL, nullptr, key) }
	, type{ STRONG }
{}

scope_t::scope_t(const scope_t *parent, const hash_ull &key)
	: scope{ new Scope(Scope::SCOPE_O, parent->scope, nullptr, key) }
	, type{ STRONG }
{}

scope_t::scope_t(const scope_t *parent, const Scope::type_t &type, const i_ptr_t &body, const hash_ull &key, const scope_t *ex, const std::vector<aug_type_t> &extensions)
	: scope{ new Scope(type, parent->scope, body, key) }
	, type{ STRONG }
{
	if (ex != NULL) {
		this->scope->extensions = ex->scope->extensions;
		this->scope->extensions.push_back(ex->scope->name_trace);
	} else {
		this->scope->extensions = extensions;
	}
}

scope_t::scope_t(Scope *parent, const aug_type_t &name_trace, const std::vector<aug_type_t> &extensions)
	: scope{ new Scope(parent, name_trace, extensions) }
	, type{ STRONG }
{}

scope_t::scope_t(const scope_t &s)
	: scope{ s.scope }
	, type{ STRONG }
{
#ifdef DEBUG
	std::cout << "&scope_t\t" << (scope == NULL ? "NULL" : ROSSA_DEHASH(scope->hashed_key)) << "\n";
#endif
	if (this->scope != NULL)
		this->scope->references++;
}

scope_t::~scope_t()
{
#ifdef DEBUG
	std::cout << "~scope_t\t" << (scope == NULL ? "NULL" : ROSSA_DEHASH(scope->hashed_key)) << "\n";
#endif
	if (scope != NULL && type == STRONG) {
		scope->references--;
		if (scope->references == 0)
			delete scope;
	}
}

void scope_t::operator=(const scope_t &b)
{
#ifdef DEBUG
	std::cout << "=scope_t\t" << (scope == NULL ? "NULL" : ROSSA_DEHASH(scope->hashed_key)) << "\n";
#endif
	if (this->scope != NULL && type == STRONG) {
		this->scope->references--;
		if (this->scope->references == 0)
			delete scope;
	}

	this->scope = b.scope;
	this->type = b.type;

	if (this->scope != NULL && this->type == STRONG)
		this->scope->references++;
}

void Scope::traceName(const hash_ull &key)
{
	if (parent != NULL)
		name_trace = parent->name_trace;
	if (key != 0)
		name_trace.push_back(key);
}

const sym_t scope_t::instantiate(const sym_vec_t &params, const token_t *token, trace_t &stack_trace) const
{
	if (scope->type != Scope::STRUCT_O)
		throw rossa_error(_FAILURE_INSTANTIATE_OBJECT_, *token, stack_trace);

	scope_t o(scope->parent, scope->name_trace, scope->extensions);
	scope->body->evaluate(&o, stack_trace);
	o.scope->getVariable(Rossa::HASH_INIT, token, stack_trace).call(params, token, stack_trace);
	return sym_t::Object(o);
}

const aug_type_t scope_t::getTypeVec() const {
	return scope->name_trace;
}

const std::string scope_t::getKey() const
{
	size_t i = 0;
	std::string path = "";
	for (auto &p : scope->name_trace) {
		if (i++ > 0)
			path += ".";
		path += ROSSA_DEHASH(p);
	}
	return path;
}

const sym_t &Scope::getVariable(const hash_ull &key, const token_t *token, trace_t &stack_trace) const
{
	if (values.find(key) != values.end())
		return values.at(key);
	if (parent != NULL)
		return parent->getVariable(key, token, stack_trace);

	throw rossa_error(format::format(_UNDECLARED_VARIABLE_ERROR_, { ROSSA_DEHASH(key) }), *token, stack_trace);
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

const sym_t &scope_t::getVariable(const hash_ull &key, const token_t *token, trace_t &stack_trace) const
{
	return scope->getVariable(key, token, stack_trace);
}

const sym_t &scope_t::createVariable(const hash_ull &key, const token_t *token) const
{
	return scope->createVariable(key, token);
}

const sym_t &scope_t::createVariable(const hash_ull &key, const sym_t &d, const token_t *token) const
{
	return scope->createVariable(key, d, token);
}

const bool scope_t::extendsObject(const aug_type_t &ex) const
{
	return std::find(scope->extensions.begin(), scope->extensions.end(), ex) != scope->extensions.end();
}

const Scope::type_t scope_t::getType() const
{
	return scope->type;
}

const i_ptr_t scope_t::getBody() const
{
	return scope->body;
}

const sym_t Scope::getThis(const token_t *token, trace_t &stack_trace)
{
	if (type != SCOPE_O)
		return sym_t::Object(scope_t(this, scope_t::type_t::STRONG));
	if (parent != NULL)
		return parent->getThis(token, stack_trace);

	throw rossa_error(format::format(_UNDECLARED_VARIABLE_ERROR_, { KEYWORD_THIS }), *token, stack_trace);
}

const sym_t scope_t::getThis(const token_t *token, trace_t &stack_trace) const
{
	return scope->getThis(token, stack_trace);
}

const bool scope_t::hasValue(const hash_ull &key) const
{
	return scope->values.find(key) != scope->values.end();
}

const bool scope_t::operator==(const scope_t &b) const
{
	return scope == b.scope;
}

Scope::~Scope()
{
#ifdef DEBUG
	std::cout << "~Scope\t" << std::to_string(type) << "\t" << ROSSA_DEHASH(hashed_key) << "\n";
#endif
	if (type == INSTANCE_O && values.find(Rossa::HASH_DELETER) != values.end()) {
		trace_t stack_trace;
		values[Rossa::HASH_DELETER].call({}, NULL, stack_trace);
	}
	for (auto &e : values) {
		e.second.shift();
	}
}

Scope *scope_t::getPtr() const
{
	return scope;
}

Scope *Scope::getParent() const
{
	return parent;
}