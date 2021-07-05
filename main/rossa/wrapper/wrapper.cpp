#include "wrapper.h"

#include "../function/function.h"
#include "../signature/signature.h"

wrapper_t::wrapper_t(const std::map<const size_t, std::map<const signature_t, ptr_function_t>> &map, const ptr_function_t &varg)
	: map{map}, varg{varg}
{
}

const unsigned int wrapper_t::hash() const
{
	int h = 0;
	int i = 0;
	for (auto &e : map)
	{
		for (auto &f : e.second)
		{
			h = (h + (f.second->key << i++)) % 0xFFFFFFFF;
		}
	}
	return h;
}