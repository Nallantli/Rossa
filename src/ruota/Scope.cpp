#include "Ruota.h"

Scope::Scope() : parent(NULL), name("") {}

Scope::Scope(Scope &parent, const std::string &name) : parent(&parent)
{
	this->name = parent.getName() != "" ? parent.getName() + "." + name : name;
}

const std::string &Scope::getName() const
{
	return this->name;
}

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
	values[key] = manager::newValue();
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
Scope *Scope::getParent()
{
	return this->parent;
}

bool Scope::hasValue(const std::string &key) const
{
	return values.find(key) != values.end();
}

Scope::~Scope() {}