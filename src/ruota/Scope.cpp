#include "Ruota.hpp"

Scope::Scope() : parent(NULL) {}

Scope::Scope(Scope &parent) : parent(&parent) {}

SYM Scope::getVariable(const std::string &key)
{
	if (values.find(key) != values.end())
		return values[key];
	if (parent != NULL)
		return parent->getVariable(key);

	throw std::runtime_error("Variable `" + key + "` is not declared within scope");
}

SYM Scope::createVariable(const std::string &key)
{
	values[key] = manager::newValue(false);
	return getVariable(key);
}

SYM Scope::createVariable(const std::string &key, const SYM &d)
{
	if (values.find(key) != values.end() && manager::getType(values[key]) == FUNCTION)
		manager::addFunctions(values[key], d);
	else
		values[key] = d;
	return getVariable(key);
}

Scope::~Scope() {}