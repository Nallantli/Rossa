#include "Ruota.hpp"

Function::Function(Scope &parent, std::vector<std::string> paramNames, std::shared_ptr<Instruction> body) : parent(&parent), paramNames(paramNames), body(body) {}

SYM Function::evaluate(std::vector<SYM> paramValues)
{
	Scope newScope(*parent);

	for (int i = 0; i < paramNames.size(); i++)
	{
		auto temp = newScope.createVariable(paramNames[i]);
		manager::set(temp, paramValues[i]);
	}

	auto ret = manager::newValue(true);
	manager::set(ret, body->evaluate(newScope));
	return ret;
}

const unsigned long Function::getArgSize() const
{
	return paramNames.size();
}