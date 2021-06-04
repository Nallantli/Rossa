#include "error.h"

#include "../function/function.h"
#include "../symbol/symbol.h"

error_t::error_t(const std::string &error, const token_t &token, const trace_t &stack_trace)
	: std::runtime_error(error)
	, token{ token }
	, stack_trace{ stack_trace }
{}

const token_t &error_t::getToken() const
{
	return token;
}

const trace_t &error_t::getTrace() const
{
	return stack_trace;
}