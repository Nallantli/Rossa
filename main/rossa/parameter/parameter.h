#ifndef PARAMETER_H
#define PARAMETER_H

#include "../rossa.h"

struct parameter_t
{
private:
	std::vector<aug_type_t> ancestors;
	aug_type_t base;
	std::vector<parameter_t> qualifiers;
public:
	parameter_t(const std::vector<aug_type_t> &, const aug_type_t &);
	parameter_t(const std::vector<aug_type_t> &, const aug_type_t &, const std::vector<parameter_t> &);
	void addQualifier(const parameter_t &);
	const std::vector<parameter_t> getQualifiers() const;
	const aug_type_t getBase() const;
	const std::string toString() const;
	const std::string toCodeString() const;
	const bool operator<(const parameter_t &) const;
	const size_t operator&(const parameter_t &) const;
	const bool operator==(const parameter_t &) const;
	const unsigned int hash() const;
};

#endif