#include "rossa_error.h"

#include "../function/function.h"
#include "../symbol/symbol.h"

rossa_error_t::rossa_error_t(const std::string &error, const token_t &token, const trace_t &stack_trace)
	: std::runtime_error(error)
	, token{ token }
	, stack_trace{ stack_trace }
{}

const token_t &rossa_error_t::getToken() const
{
	return token;
}

const trace_t &rossa_error_t::getTrace() const
{
	return stack_trace;
}