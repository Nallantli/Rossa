#include "Ruota.h"

Scope::Scope() : parent(NULL), name_trace({}) {}

Scope::Scope(Scope *parent, const hashcode_t &key) : parent(parent)
{
	name_trace = parent->name_trace;
	if (key != 0)
	{
		name_trace.push_back(key);

		size_t i = 0;
		std::string path = "";
		for (auto &p : name_trace)
		{
			if (i++ > 0)
				path += ".";
			path += MAIN_HASH.deHash(p);
		}

		hashed_key = MAIN_HASH.hashString(path);
	}
	else
	{
		hashed_key = parent->hashed_key;
	}
}

const hashcode_t Scope::getHashedKey() const
{
	return hashed_key;
}

const Symbol &Scope::getVariable(const hashcode_t &key, const Token *token)
{
	if (values.find(key) != values.end())
		return values[key];
	if (parent != NULL)
		return parent->getVariable(key, token);

	throw RuotaError((boost::format(_UNDECLARED_VARIABLE_ERROR_) % MAIN_HASH.deHash(key)).str(), *token);
}

const Symbol &Scope::createVariable(const hashcode_t &key, const Token *token)
{
	values[key] = Symbol();
	return values[key];
}

const Symbol &Scope::createVariable(const hashcode_t &key, const Symbol &d, const Token *token)
{
	if (values.find(key) != values.end() && values[key].getValueType() == FUNCTION)
		values[key].addFunctions(&d, token);
	else
		values[key] = d;

	return values[key];
}

void Scope::clear()
{
	values.clear();
}

Scope *Scope::getParent() const
{
	return this->parent;
}

bool Scope::hasValue(const hashcode_t &key) const
{
	return values.find(key) != values.end();
}