#ifndef PARAMETER_H
#define PARAMETER_H

#include "../rossa.h"

struct param_t
{
private:
	std::vector<aug_type_t> ancestors;
	aug_type_t base;
	std::vector<param_t> qualifiers;
public:
	param_t(const std::vector<aug_type_t> &, const aug_type_t &);
	param_t(const std::vector<aug_type_t> &, const aug_type_t &, const std::vector<param_t> &);
	void addQualifier(const param_t &);
	const std::vector<param_t> getQualifiers() const;
	const aug_type_t getBase() const;
	const std::string toString() const;
	const std::string toCodeString() const;
	const bool operator<(const param_t &) const;
	const size_t operator&(const param_t &) const;
	const bool operator==(const param_t &) const;
	const unsigned int hash() const;
};

#endif