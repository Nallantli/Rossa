#include "Ruota.hpp"

Function::Function(Scope &parent, std::vector<std::string> paramNames, std::shared_ptr<Instruction> body) : parent(&parent), paramNames(paramNames), body(body) {}

DatumID Function::evaluate(std::vector<DatumID> paramValues)
{
	Scope newScope(*parent);

	for (int i = 0; i < paramNames.size(); i++)
	{
		auto temp = newScope.createVariable(paramNames[i]);
		Ruota::manager->set(temp, paramValues[i]);
	}

	auto ret = Ruota::manager->newDatum(true);
	Ruota::manager->set(ret, body->evaluate(newScope));
	return ret;
}

const unsigned long Function::getArgSize() const
{
	return paramNames.size();
}