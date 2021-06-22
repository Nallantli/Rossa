#include "signature.h"

#include "../value/value.h"
#include "../symbol/symbol.h"
#include "../object/object.h"
#include "../function/function.h"
#include "../parameter/parameter.h"

signature_t::signature_t()
{}

signature_t::signature_t(const std::vector<parameter_t> &values)
	: values{ values }
{}

const size_t signature_t::validity(const std::vector<symbol_t> &check, trace_t &stack_trace) const
{
	if (values.size() == 0)
		return 1;

	size_t v = 0;
	for (size_t i = 0; i < values.size(); i++) {
		auto check_i = check[i];
		auto values_i = values[i];
		auto vt = check_i.getAugValueType();
		if (values_i.getQualifiers().empty()) {
			auto base = values_i.getBase();
			if (base == vt.getBase())
				v += 3;
			else if (base[0] > 0 && vt.getBase()[0] == value_type_enum::NIL)
				v += 2;
			else if (base[0] == value_type_enum::ANY)
				v += 1;
			else if (check[0].getValueType() == value_type_enum::OBJECT) {
				if (base[0] == value_type_enum::OBJECT || check_i.getObject(NULL, stack_trace)->extendsObject(base))
					v += 2;
				else
					return 0;
			} else
				return 0;
		} else {
			auto ql = values_i.getQualifiers();
			auto fo = check_i.getFunctionOverloads(NULL, stack_trace);
			const auto it = fo.find(ql.size());
			if (it == fo.end()) {
				if (check_i.hasVarg(NULL, stack_trace))
					v += 1;
				else
					return 0;
			} else {
				size_t flag = 0;
				for (auto f : it->second) {
					for (size_t i = 0; i < ql.size(); i++) {
						auto val = ql[i] & f.first.values[i];
						if (val > flag) {
							flag = val;
							if (val == 3)
								break;
						}
					}
					if (flag == 3)
						break;
				}
				if (flag > 0)
					v += flag;
				else {
					if (check_i.hasVarg(NULL, stack_trace))
						v += 1;
					else
						return 0;
				}
			}
		}
	}
	return v;
}

const std::string signature_t::toCodeString() const
{
	std::string s = "{";
	size_t i = 0;
	for (auto &v : values) {
		if (i++ > 0)
			s += ", ";
		s += v.toCodeString();
	}
	return s + "}";
}

const std::string signature_t::toString() const
{
	std::string s = "";
	size_t i = 0;
	for (auto &v : values) {
		if (i++ > 0)
			s += ", ";
		s += v.toString();
	}
	return s;
}

const bool signature_t::operator<(const signature_t &s) const
{
	return values < s.values;
}

const bool signature_t::operator==(const signature_t &s) const
{
	return values == s.values;
}