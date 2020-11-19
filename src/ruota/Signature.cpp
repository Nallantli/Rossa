#include "Ruota.h"

using namespace ruota;

const size_t sig::validity(const sig_t &values, const std::vector<Symbol> &check, std::vector<Function> &stack_trace)
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
			if (check[i].getObject(NULL, stack_trace)->extendsObject(values[i]))
				v += 2;
			else
				return 0;
		} else
			return 0;
	}
	return v;
}

const std::string sig::toString(const sig_t &values)
{
	std::string s = "(";
	size_t i = 0;
	for (auto &v : values) {
		if (i++ > 0)
			s += ", ";
		s += getTypeString(v);
	}
	return s + ")";
}

const std::string sig::getTypeString(const type_sll &i)
{
	if (i >= 0)
		return "@" + RUOTA_DEHASH(i);
	else {
		switch (i) {
			case NIL:
				return KEYWORD_NIL_NAME;
			case NUMBER:
				return KEYWORD_NUMBER;
			case STRING:
				return KEYWORD_STRING;
			case BOOLEAN_D:
				return KEYWORD_BOOLEAN;
			case ARRAY:
				return KEYWORD_ARRAY;
			case FUNCTION:
				return KEYWORD_FUNCTION;
			case DICTIONARY:
				return KEYWORD_DICTIONARY;
			case OBJECT:
				return KEYWORD_OBJECT;
			case POINTER:
				return KEYWORD_POINTER;
			case TYPE_NAME:
				return KEYWORD_TYPE;
			default:
				return "<error-type>";
		}
	}
}