#include "Ruota.h"

Function::Function(hashcode_t key, Scope &parent, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params, std::shared_ptr<Instruction> body) : key(key), parent(&parent), params(params), body(body) {}

const Symbol Function::evaluate(std::vector<Symbol> paramValues) const
{
	return evaluate(paramValues, NULL);
}

const Symbol Function::evaluate(std::vector<Symbol> paramValues, Symbol *thisSym) const
{
	Ruota::stack_trace.push_back(*this);

	Scope newScope(*parent, "");

	for (size_t i = 0; i < params.size(); i++)
	{
		switch (params[i].first)
		{
		case TOK_FINAL:
		{
			auto temp = newScope.createVariable(params[i].second);
			temp.set(paramValues[i]);
			temp.setMutable(false);
			break;
		}
		case TOK_REF:
		{
			newScope.createVariable(params[i].second, paramValues[i]);
			break;
		}
		default:
		{
			auto temp = newScope.createVariable(params[i].second);
			temp.set(paramValues[i]);
			break;
		}
		}
	}

	if (thisSym != NULL)
		newScope.createVariable(Ruota::HASH_THIS, *thisSym);

	auto temp = body->evaluate(newScope);
	if (!temp.canSet() || temp.getSymbolType() == ID_REFER)
	{
		temp.setSymbolType(ID_CASUAL);
		return temp;
	}

	Ruota::stack_trace.pop_back();

	auto ret = Symbol();
	ret.set(temp);
	return ret;
}

size_t Function::getArgSize() const
{
	return params.size();
}

hashcode_t Function::getKey() const
{
	return key;
}

Scope *Function::getParent() const
{
	return parent;
}

std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> Function::getParams() const
{
	return params;
}