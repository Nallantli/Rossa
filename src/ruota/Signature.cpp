#include "Ruota.h"

Signature::Signature(const std::vector<object_type_t> &values) : values(values) {}

size_t Signature::validity(const std::vector<Symbol> &check) const
{
	if (values.size() == 0)
		return 1;

	size_t v = 0;
	for (size_t i = 0; i < values.size(); i++)
	{
		object_type_t vt = check[i].getAugValueType();
		if (values[i] == NIL && values[i] != vt)
			v += 1;
		else if (values[i] == vt)
			v += 2;
		else if (check[i].getValueType() == OBJECT)
		{
			if (check[i].getObject(NULL)->extendsObject(values[i]))
				return 2;
			else
				return 0;
		}
		else
			return 0;
	}
	return v;
}

const std::string Signature::toString() const
{
	std::string s = "(";
	size_t i = 0;
	for (auto &v : values)
	{
		if (i++ > 0)
			s += ", ";
		s += getTypeString(v);
	}
	return s + ")";
}

bool Signature::operator<(const Signature &sig) const
{
	return this->toString() < sig.toString();
}