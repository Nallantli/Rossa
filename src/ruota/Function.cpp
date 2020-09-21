#include "Ruota.h"

Function::Function(Scope &parent, std::vector<std::pair<LEX_TOKEN_TYPE, std::string>> params, std::shared_ptr<Instruction> body) : parent(&parent), params(params), body(body) {}

Symbol Function::evaluate(std::vector<Symbol> paramValues)
{
	return evaluate(paramValues, NULL);
}

Symbol Function::evaluate(std::vector<Symbol> paramValues, Symbol *thisSym)
{
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
		newScope.createVariable("this", *thisSym);

	auto ret = Symbol(true);
	ret.set(body->evaluate(newScope));
	return ret;
}

const unsigned long Function::getArgSize() const
{
	return params.size();
}