#include "Ruota.h"

Signature::Signature(const std::vector<type_sll> &values) : values(values)
{}

size_t Signature::validity(const std::vector<Symbol> &check) const
{
	if (values.size() == 0)
		return 1;

	size_t v = 0;
	for (size_t i = 0; i < values.size(); i++) {
		type_sll vt = check[i].getAugValueType();
		if (values[i] == NIL && values[i] != vt)
			v += 1;
		else if (values[i] == vt)
			v += 3;
		else if (check[i].getValueType() == OBJECT) {
			if (check[i].getObject(NULL)->extendsObject(values[i]))
				v += 2;
			else
				return 0;
		} else
			return 0;
	}
	return v;
}

const string Signature::toString() const
{
	string s = "(";
	size_t i = 0;
	for (auto &v : values) {
		if (i++ > 0)
			s += ", ";
		s += getTypeString(v);
	}
	return s + ")";
}

const string Signature::getTypeString(const type_sll &i) const
{
	if (i >= 0)
		return "@" + MAIN_HASH.deHash(i);
	else {
		switch (i) {
			case NIL:
				return "Nil";
			case NUMBER:
				return "Number";
			case STRING:
				return "String";
			case BOOLEAN_D:
				return "Boolean";
			case ARRAY:
				return "Array";
			case FUNCTION:
				return "Function";
			case DICTIONARY:
				return "Dictionary";
			case OBJECT:
				return "Object";
			case POINTER:
				return "Pointer";
			case TYPE_NAME:
				return "Type";
			default:
				return "<error-type>";
		}
	}
}

bool Signature::operator<(const Signature &sig) const
{
	return this->toString() < sig.toString();
}