#include "Ruota.h"

using namespace ruota;

Function::Function(const hash_ull &key, Scope *parent, const std::vector<std::pair<LexerTokenType, hash_ull>> &params, const std::shared_ptr<Instruction> &body) : key(key), parent(parent), params(params), body(body)
{}

const Symbol Function::evaluate(const std::vector<Symbol> &paramValues, const Token *token) const
{
	Ruota::stack_trace.push_back(*this);

	Scope newScope(parent, 0);

	for (size_t i = 0; i < params.size(); i++) {
		switch (params[i].first) {
			case TOK_REF:
				newScope.createVariable(params[i].second, paramValues[i], token);
				break;
			default:
			{
				auto temp = newScope.createVariable(params[i].second, token);
				temp.set(&paramValues[i], token, false);
				break;
			}
		}
	}

	auto temp = body->evaluate(&newScope);

	Ruota::stack_trace.pop_back();

	if (temp.getSymbolType() == ID_REFER) {
		temp.setSymbolType(ID_CASUAL);
		return temp;
	}

	auto ret = Symbol();
	ret.set(&temp, token, false);
	return ret;
}

const size_t Function::getArgSize() const
{
	return params.size();
}

const hash_ull Function::getKey() const
{
	return key;
}

Scope *Function::getParent() const
{
	return parent;
}

const std::vector<std::pair<LexerTokenType, hash_ull>> &Function::getParams() const
{
	return params;
}