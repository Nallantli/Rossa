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

Symbol Scope::getVariable(const std::string &key)
{
	if (values.find(key) != values.end())
		return values[key];
	if (parent != NULL)
		return parent->getVariable(key);

	throw std::runtime_error("Variable `" + key + "` is not declared within scope");
}

Symbol Scope::createVariable(const std::string &key)
{
	values[key] = Symbol();
	return getVariable(key);
}

Symbol Scope::createVariable(const std::string &key, Symbol d)
{
	if (values.find(key) != values.end() && values[key].getType() == FUNCTION)
		values[key].addFunctions(d);
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