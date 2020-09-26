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

const Symbol &Scope::getVariable(hashcode_t key) const
{
	if (values.find(key) != values.end())
		return values.at(key);
	if (parent != NULL)
		return parent->getVariable(key);

	throw std::runtime_error("Variable `" + hash.deHash(key) + "` is not declared within scope");
}

const Symbol &Scope::createVariable(hashcode_t key)
{
	values[key] = Symbol();
	return getVariable(key);
}

const Symbol &Scope::createVariable(hashcode_t key, const Symbol &d)
{
	if (values.find(key) != values.end() && values[key].getValueType() == FUNCTION)
		values[key].addFunctions(d);
	else
		values[key] = d;
	return getVariable(key);
}
Scope *Scope::getParent() const
{
	return this->parent;
}

bool Scope::hasValue(hashcode_t key) const
{
	return values.find(key) != values.end();
}

Scope::~Scope() {}