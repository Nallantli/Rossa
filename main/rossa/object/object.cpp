#include "object.h"

#include <algorithm>

#include "../rossa_error/rossa_error.h"
#include "../scope/scope.h"
#include "../symbol/symbol.h"
#include "../parameter/parameter.h"
#include "../instruction/instruction.h"
#include "../parser/parser.h"

object_t::object_t(scope_t *scope, const object_type_enum &type)
	: scope{ scope }
	, type{ type }
{
	if (type == OBJECT_STRONG)
		scope->references++;
}

object_t::object_t()
	: scope{ NULL }
	, type{ OBJECT_STRONG }
{}

object_t::object_t(const hash_ull &key)
	: scope{ new scope_t(scope_type_enum::SCOPE_BOUNDED, NULL, nullptr, key) }
	, type{ OBJECT_STRONG }
{}

object_t::object_t(const object_t *parent, const hash_ull &key)
	: scope{ new scope_t(scope_type_enum::SCOPE_BOUNDED, parent->scope, nullptr, key) }
	, type{ OBJECT_STRONG }
{}

object_t::object_t(const object_t *parent, const scope_type_enum &type, const ptr_instruction_t &body, const hash_ull &key, const object_t *ex, const std::vector<aug_type_t> &extensions)
	: scope{ new scope_t(type, parent->scope, body, key) }
	, type{ OBJECT_STRONG }
{
	if (ex != NULL) {
		this->scope->extensions = ex->scope->extensions;
		this->scope->extensions.push_back(ex->scope->name_trace);
	} else {
		this->scope->extensions = extensions;
	}
}

object_t::object_t(scope_t *parent, const aug_type_t &name_trace, const std::vector<aug_type_t> &extensions)
	: scope{ new scope_t(parent, name_trace, extensions) }
	, type{ OBJECT_STRONG }
{}

object_t::object_t(const object_t &s)
	: scope{ s.scope }
	, type{ OBJECT_STRONG }
{
#ifdef DEBUG
	std::cout << "&object_t\t" << (scope == NULL ? "NULL" : getKey()) << "\t(";
	std::cout << (s.scope == NULL ? "NULL" : s.getKey()) << ")\n";
#endif
	if (this->scope != NULL)
		this->scope->references++;
}

object_t::~object_t()
{
#ifdef DEBUG
	std::cout << "~object_t\t" << (scope == NULL ? "NULL" : getKey()) << "\n";
#endif
	if (scope != NULL && type == OBJECT_STRONG) {
		scope->references--;
		if (scope->references == 0)
			delete scope;
	}
}

void object_t::operator=(const object_t &b)
{
#ifdef DEBUG
	std::cout << "=object_t\t" << (scope == NULL ? "NULL" : getKey()) << "\t(";
	std::cout << (b.scope == NULL ? "NULL" : b.getKey()) << ")\n";
#endif
	if (this->scope != NULL && type == OBJECT_STRONG) {
		this->scope->references--;
		if (this->scope->references == 0)
			delete scope;
	}

	this->scope = b.scope;
	this->type = b.type;

	if (this->scope != NULL && this->type == OBJECT_STRONG)
		this->scope->references++;
}

const symbol_t object_t::instantiate(const std::vector<symbol_t> &params, const token_t *token, trace_t &stack_trace) const
{
	if (scope->type != scope_type_enum::SCOPE_STRUCT)
		throw rossa_error_t(_FAILURE_INSTANTIATE_OBJECT_, *token, stack_trace);

	object_t o(scope->parent, scope->name_trace, scope->extensions);
	scope->body->evaluate(&o, stack_trace);
	o.scope->getVariable(parser_t::HASH_INIT, token, stack_trace).call(params, token, stack_trace);
	return symbol_t::Object(o);
}

const parameter_t object_t::getTypeVec() const
{
	return parameter_t(scope->extensions, scope->name_trace);
}

const std::string object_t::getKey() const
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

const symbol_t object_t::getThis(const token_t *token, trace_t &stack_trace) const
{
	return scope->getThis(token, stack_trace);
}

const bool object_t::hasValue(const hash_ull &key) const
{
	return scope->values.find(key) != scope->values.end();
}

const bool object_t::operator==(const object_t &b) const
{
	return scope == b.scope;
}

const unsigned int object_t::hash() const
{
	if (this->scope != NULL)
		return this->scope->hash();
	return 0;
}

const symbol_t &object_t::getVariable(const hash_ull &key, const token_t *token, trace_t &stack_trace) const
{
	return scope->getVariable(key, token, stack_trace);
}

const symbol_t &object_t::createVariable(const hash_ull &key, const token_t *token) const
{
	return scope->createVariable(key, token);
}

const symbol_t &object_t::createVariable(const hash_ull &key, const symbol_t &d, const token_t *token) const
{
	return scope->createVariable(key, d, token);
}

const bool object_t::extendsObject(const aug_type_t &ex) const
{
	return std::find(scope->extensions.begin(), scope->extensions.end(), ex) != scope->extensions.end();
}

const scope_type_enum object_t::getType() const
{
	return scope->type;
}

const ptr_instruction_t object_t::getBody() const
{
	return scope->body;
}

scope_t *object_t::getPtr() const
{
	return scope;
}