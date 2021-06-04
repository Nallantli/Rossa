#include "param.h"

#include "../global/global.h"

param_t::param_t(const std::vector<aug_type_t> &ancestors, const aug_type_t &base)
	: ancestors(ancestors)
	, base{ base }
{}

param_t::param_t(const std::vector<aug_type_t> &ancestors, const aug_type_t &base, const std::vector<param_t> &qualifiers)
	: ancestors(ancestors)
	, base{ base }
	, qualifiers{ qualifiers }
{}

void param_t::addQualifier(const param_t &param)
{
	this->qualifiers.push_back(param);
}

const std::string param_t::toString() const
{
	std::string s = global::getTypeString(base);
	if (!qualifiers.empty()) {
		s += "<";
		size_t i = 0;
		for (auto &v : qualifiers) {
			if (i++ > 0)
				s += ", ";
			s += v.toString();
		}
		s += ">";
	}
	return s;
}

const unsigned int param_t::hash() const
{
	int h = 0;
	int i = 0;
	for (auto &e : base) {
		h = (h + (std::abs(e) << i++)) % 0xFFFFFFFF;
	}
	for (auto &e : qualifiers) {
		h = (h + (e.hash() << i++)) % 0xFFFFFFFF;
	}
	return h;
}

const std::string param_t::toCodeString() const
{
	std::string s = "param_t(static_cast<type_sll>(" + global::getTypeString(base) + "), ";
	s += "{";
	size_t i = 0;
	for (auto &v : qualifiers) {
		if (i++ > 0)
			s += ", ";
		s += v.toCodeString();
	}
	s += "}";
	return s + ")";
}

const std::vector<param_t> param_t::getQualifiers() const
{
	return this->qualifiers;
}

const aug_type_t param_t::getBase() const
{
	return this->base;
}

const bool param_t::operator<(const param_t &pt) const
{
	if (base != pt.base)
		return base < pt.base;
	if (qualifiers.size() != pt.qualifiers.size())
		return qualifiers.size() < pt.qualifiers.size();
	for (size_t i = 0; i < qualifiers.size(); i++) {
		if (qualifiers[i] < pt.qualifiers[i])
			return true;
	}
	if (ancestors.size() != pt.ancestors.size())
		return ancestors.size() < pt.ancestors.size();
	for (size_t i = 0; i < ancestors.size(); i++) {
		if (ancestors[i] < pt.ancestors[i])
			return true;
	}
	return false;
}

const size_t param_t::operator&(const param_t &pt) const
{
	if (base[0] == value_type_enum::ANY)
		return 1;
	if (base != pt.base && std::find(pt.ancestors.begin(), pt.ancestors.end(), base) == pt.ancestors.end())
		return 0;
	if (qualifiers.empty() && pt.qualifiers.empty())
		return 3;
	if (qualifiers.empty() && !pt.qualifiers.empty())
		return 2;
	if (!qualifiers.empty() && qualifiers.size() != pt.qualifiers.size())
		return 0;
	size_t v = 3;
	for (size_t i = 0; i < qualifiers.size(); i++) {
		auto val = qualifiers[i] & pt.qualifiers[i];
		if (val == 0)
			return 0;
		if (val < v)
			v = val;
	}
	return v;
}


const bool param_t::operator==(const param_t &p) const
{
	return base == p.base && qualifiers == p.qualifiers;
}