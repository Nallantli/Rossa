#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "../rossa.h"
#include "../param/param.h"

struct signature_t
{
private:
	const std::vector<param_t> values;
public:
	signature_t();
	signature_t(const std::vector<param_t> &values);
	const size_t validity(const std::vector<symbol_t> &, trace_t &stack_trace) const;
	const std::string toString() const;
	const std::string toCodeString() const;
	const bool operator<(const signature_t &) const;
	const bool operator==(const signature_t &) const;
};

#endif