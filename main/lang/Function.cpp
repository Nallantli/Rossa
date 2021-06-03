#include "Rossa.h"

Function::Function(const hash_ull &key, Scope *parent, const std::vector<std::pair<LexerTokenType, hash_ull>> &params, const i_ptr_t &body, const hash_sym_map_t &captures)
	: key{ key }
	, parent{ parent }
	, params{ params }
	, body{ body }
	, captures{ captures }
	, isVargs{ false }
{}

Function::Function(const hash_ull &key, Scope *parent, const i_ptr_t &body, const hash_sym_map_t &captures)
	: key{ key }
	, parent{ parent }
	, body{ body }
	, captures{ captures }
	, isVargs{ true }
{}

const sym_t function_evaluate(const func_ptr_t &function, const sym_vec_t &paramValues, const token_t *token, trace_t &stack_trace)
{
	stack_trace.push_back({ (token == NULL ? token_t() : *token), *function.get() });
	if (function->isVargs)
		return function_evaluate_vargs(function, paramValues, token, stack_trace);

	scope_t p(function->parent, scope_t::type_t::WEAK);
	const scope_t newScope(&p, 0);

	for (size_t i = 0; i < function->params.size(); i++) {
		switch (function->params[i].first) {
			case TOK_REF:
				newScope.createVariable(function->params[i].second, paramValues[i], token);
				break;
			default:
			{
				const sym_t &temp = newScope.createVariable(function->params[i].second, token);
				temp.set(&paramValues[i], token, stack_trace);
				break;
			}
		}
	}

	for (const std::pair<const hash_ull, const sym_t> &e : function->captures) {
		newScope.createVariable(e.first, e.second, token);
	}

	sym_t temp = function->body->evaluate(&newScope, stack_trace);

	if (temp.getSymbolType() == sym_t::type_t::ID_REFER) {
		temp.setSymbolType(sym_t::type_t::ID_CASUAL);

		stack_trace.pop_back();
		return temp;
	}

	const sym_t ret = sym_t();
	ret.set(&temp, token, stack_trace);

	stack_trace.pop_back();
	return ret;
}

const sym_t function_evaluate_vargs(const func_ptr_t &function, const sym_vec_t &paramValues, const token_t *token, trace_t &stack_trace)
{
	scope_t p(function->parent, scope_t::type_t::WEAK);
	const scope_t newScope(&p, 0);

	newScope.createVariable(Rossa::HASH_VAR_ARGS, sym_t::Array(paramValues), token);

	for (const std::pair<const hash_ull, const sym_t> &e : function->captures) {
		newScope.createVariable(e.first, e.second, token);
	}

	sym_t temp = function->body->evaluate(&newScope, stack_trace);

	if (temp.getSymbolType() == sym_t::type_t::ID_REFER) {
		temp.setSymbolType(sym_t::type_t::ID_CASUAL);

		stack_trace.pop_back();
		return temp;
	}

	const sym_t ret = sym_t();
	ret.set(&temp, token, stack_trace);

	stack_trace.pop_back();
	return ret;
}

const scope_t Function::getParent() const
{
	return scope_t(parent, scope_t::type_t::STRONG);
}

void Function::shift()
{
	if (parent != NULL)
		parent = parent->getParent();
}