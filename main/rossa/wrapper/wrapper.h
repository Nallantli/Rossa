#ifndef WRAPPER_H
#define WRAPPER_H

#include "../rossa.h"

struct wrapper_t
{
	std::map<const size_t, std::map<const signature_t, ptr_function_t>> map;
	ptr_function_t varg = nullptr;

	wrapper_t(const std::map<const size_t, std::map<const signature_t, ptr_function_t>> &, const ptr_function_t &);
	const unsigned int hash() const;
};

#endif