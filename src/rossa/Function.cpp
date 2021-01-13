#include "../../bin/include/Rossa.h"

using namespace rossa;

Function::Function(const hash_ull &key, const std::shared_ptr<Scope> &parent, const std::vector<std::pair<LexerTokenType, hash_ull>> &params, const std::shared_ptr<Instruction> &body, const std::map<hash_ull, Symbol> &captures)
	: key{ key }
	, parent{ parent }
	, params{ params }
	, body{ body }
	, captures{ captures }
{}

const Symbol Function::evaluate(const sym_vec_t &paramValues, const Token *token, trace_t &stack_trace) const
{
	stack_trace.push_back(*this);

	auto newScope = std::make_shared<Scope>(parent, 0);

	for (size_t i = 0; i < params.size(); i++) {
		switch (params[i].first) {
			case TOK_REF:
				newScope->createVariable(params[i].second, paramValues[i], token);
				break;
			default:
			{
				auto &temp = newScope->createVariable(params[i].second, token);
				temp.set(&paramValues[i], token, false, stack_trace);
				break;
			}
		}
	}

	for (auto e : captures) {
		newScope->createVariable(e.first, e.second, token);
	}

	auto temp = body->evaluate(newScope, stack_trace);
	newScope->clear();

	if (temp.getSymbolType() == ID_REFER) {
		temp.setSymbolType(ID_CASUAL);
		return temp;
	}

	auto ret = Symbol();
	ret.set(&temp, token, false, stack_trace);

	stack_trace.pop_back();

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

const std::shared_ptr<Scope> &Function::getParent() const
{
	return parent;
}

const std::vector<std::pair<LexerTokenType, hash_ull>> &Function::getParams() const
{
	return params;
}