#include "function.h"

#include "../symbol/symbol.h"
#include "../instruction/instruction.h"
#include "../scope/scope.h"
#include "../parser/parser.h"

function_t::function_t(const hash_ull &key, scope_t *parent, const std::vector<std::pair<token_type_enum, hash_ull>> &params, const ptr_instruction_t &body, const std::map<const hash_ull, const symbol_t> &captures)
	: key{key}, parent{parent}, params{params}, body{body}, captures{captures}, isVargs{false}
{
}

function_t::function_t(const hash_ull &key, scope_t *parent, const ptr_instruction_t &body, const std::map<const hash_ull, const symbol_t> &captures)
	: key{key}, parent{parent}, body{body}, captures{captures}, isVargs{true}
{
}

const symbol_t function_evaluate(const ptr_function_t &function, const std::vector<symbol_t> &paramValues, const token_t *token, trace_t &stack_trace)
{
	stack_trace.push_back({(token == NULL ? token_t() : *token), *function.get()});
	if (function->isVargs)
		return function_evaluate_vargs(function, paramValues, token, stack_trace);

	object_t p(function->parent, object_type_enum::OBJECT_WEAK);
	const object_t newScope(&p, 0);

	for (size_t i = 0; i < function->params.size(); i++)
	{
		switch (function->params[i].first)
		{
		case TOK_REF:
			newScope.createVariable(function->params[i].second, paramValues[i], token);
			break;
		default:
		{
			const symbol_t &temp = newScope.createVariable(function->params[i].second, token);
			temp.set(&paramValues[i], token, stack_trace);
			break;
		}
		}
	}

	for (const std::pair<const hash_ull, const symbol_t> &e : function->captures)
	{
		newScope.createVariable(e.first, e.second, token);
	}

	symbol_t temp = function->body->evaluate(&newScope, stack_trace);

	if (temp.getSymbolType() == symbol_t::type_t::ID_REFER)
	{
		temp.setSymbolType(symbol_t::type_t::ID_CASUAL);

		stack_trace.pop_back();
		return temp;
	}

	const symbol_t ret = symbol_t();
	ret.set(&temp, token, stack_trace);

	stack_trace.pop_back();
	return ret;
}

const symbol_t function_evaluate_vargs(const ptr_function_t &function, const std::vector<symbol_t> &paramValues, const token_t *token, trace_t &stack_trace)
{
	object_t p(function->parent, object_type_enum::OBJECT_WEAK);
	const object_t newScope(&p, 0);

	newScope.createVariable(parser_t::HASH_VAR_ARGS, symbol_t::Array(paramValues), token);

	for (const std::pair<const hash_ull, const symbol_t> &e : function->captures)
	{
		newScope.createVariable(e.first, e.second, token);
	}

	symbol_t temp = function->body->evaluate(&newScope, stack_trace);

	if (temp.getSymbolType() == symbol_t::type_t::ID_REFER)
	{
		temp.setSymbolType(symbol_t::type_t::ID_CASUAL);

		stack_trace.pop_back();
		return temp;
	}

	const symbol_t ret = symbol_t();
	ret.set(&temp, token, stack_trace);

	stack_trace.pop_back();
	return ret;
}

const object_t function_t::getParent() const
{
	return object_t(parent, object_type_enum::OBJECT_STRONG);
}

void function_t::shift()
{
	if (parent != NULL)
		parent = parent->getParent();
}