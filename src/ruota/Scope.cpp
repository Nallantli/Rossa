#include "Ruota.hpp"

Scope::Scope() : parent(NULL) {}

Scope::Scope(Scope &parent) : parent(&parent) {}

DatumID Scope::getVariable(const std::string &key)
{
	if (values.find(key) != values.end())
		return values[key];
	if (parent != NULL)
		return parent->getVariable(key);

	throw std::runtime_error("Variable `" + key + "` is not declared within scope");
}

DatumID Scope::createVariable(const std::string &key)
{
	values[key] = Ruota::manager->newDatum(false);
	return getVariable(key);
}

DatumID Scope::createVariable(const std::string &key, const DatumID &d)
{
	if (values.find(key) != values.end() && Ruota::manager->getType(values[key]) == FUNCTION)
		Ruota::manager->addFunctions(values[key], d);
	else
		values[key] = d;
	return getVariable(key);
}

Scope::~Scope()
{
	for (auto &e : values)
	{
		if (!e.second.constant)
			Ruota::manager->setFree(e.second);
	}
}