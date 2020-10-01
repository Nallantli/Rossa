#include "Ruota.h"

Scope::Scope() : parent(NULL), name("") {}

Scope::Scope(Scope *parent, const std::string &name) : parent(parent)
{
	this->name = parent->getName() != "" ? parent->getName() + "." + name : name;
}

const std::string &Scope::getName() const
{
	return this->name;
}

Symbol &Scope::getVariable(hashcode_t key, const Token *token)
{
	if (values.find(key) != values.end())
		return values[key];
	if (parent != NULL)
		return parent->getVariable(key, token);

	throw RuotaError((boost::format(_UNDECLARED_VARIABLE_ERROR_) % hash.deHash(key)).str(), *token);
}

Symbol &Scope::createVariable(hashcode_t key, const Token *token)
{
	values[key] = Symbol();
	return getVariable(key, token);
}

Symbol &Scope::createVariable(hashcode_t key, const Symbol &d, const Token *token)
{
	if (values.find(key) != values.end() && values[key].getValueType() == FUNCTION)
		values[key].addFunctions(&d, token);
	else
		values[key] = d;
	return getVariable(key, token);
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