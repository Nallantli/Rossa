#include "Ruota.h"

Signature::Signature(const std::vector<ValueType> &values) : values(values) {}

size_t Signature::validity(const std::vector<ValueType> &check) const
{
	if (values.size() == 0)
		return 1;

	size_t v = 0;
	for (size_t i = 0; i < values.size(); i++)
	{
		if (values[i] == check[i])
			v += 2;
		else if (values[i] == NIL && values[i] != check[i])
			v += 1;
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