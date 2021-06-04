#include "scope.h"

#include "../parser/parser.h"
#include "../global/global.h"

scope_t::scope_t(const scope_type_enum &type, scope_t *parent, const ptr_instruction_t &body, const hash_ull &key)
	: type{ type }
	, parent{ parent }
	, body{ body }
{
	traceName(key);
}

scope_t::scope_t(scope_t *parent, const aug_type_t &name_trace, const std::vector<aug_type_t> &extensions)
	: type{ scope_type_enum::SCOPE_INSTANCE }
	, parent{ parent }
	//	, hashed_key{ key }
	, name_trace{ name_trace }
	, extensions{ extensions }
{}

void scope_t::traceName(const hash_ull &key)
{
	if (parent != NULL)
		name_trace = parent->name_trace;
	if (key != 0)
		name_trace.push_back(key);
}

const symbol_t &scope_t::getVariable(const hash_ull &key, const token_t *token, trace_t &stack_trace) const
{
	const auto it = values.find(key);
	if (it != values.end())
		return it->second;
	if (parent != NULL)
		return parent->getVariable(key, token, stack_trace);

	throw rossa_error_t(global::format(_UNDECLARED_VARIABLE_ERROR_, { ROSSA_DEHASH(key) }), *token, stack_trace);
}

const symbol_t &scope_t::createVariable(const hash_ull &key, const token_t *token)
{
	values[key].nullify();
	return values[key];
}

const symbol_t &scope_t::createVariable(const hash_ull &key, const symbol_t &d, const token_t *token)
{
	const auto it = values.find(key);
	if (it != values.end() && it->second.getValueType() == value_type_enum::FUNCTION) {
		it->second.addFunctions(&d, token);
		return it->second;
	}
	return values.insert({ key, d }).first->second;
}

const unsigned int scope_t::hash() const
{
	int h = 0;
	int i = 0;
	for (auto &e : values) {
		h = (h + ((e.first + e.second.hash()) << i++)) % 0xFFFFFFFF;
	}
	return h;
}

const symbol_t scope_t::getThis(const token_t *token, trace_t &stack_trace)
{
	if (type != scope_type_enum::SCOPE_BOUNDED)
		return symbol_t::Object(object_t(this, object_type_enum::OBJECT_STRONG));
	if (parent != NULL)
		return parent->getThis(token, stack_trace);

	throw rossa_error_t(global::format(_UNDECLARED_VARIABLE_ERROR_, { KEYWORD_THIS }), *token, stack_trace);
}

scope_t::~scope_t()
{
#ifdef DEBUG
	std::cout << "~scope_t\t" << std::to_string(type) << "\n";
#endif
	if (type == scope_type_enum::SCOPE_INSTANCE) {
		const auto it = values.find(parser_t::HASH_DELETER);
		if (it != values.end()) {
#ifdef DEBUG
			std::cout << "Deleter Found\n";
#endif
			trace_t stack_trace;
			it->second.call({}, NULL, stack_trace);
#ifdef DEBUG
			std::cout << "Deleter Executed\n";
#endif
		}
	}
	for (auto &e : values) {
		e.second.shift();
	}
}

scope_t *scope_t::getParent() const
{
	return parent;
}