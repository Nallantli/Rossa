#include "../../bin/include/Rossa.h"

rossa_error::rossa_error(const std::string &error, const token_t &token, const trace_t &stack_trace)
	: std::runtime_error(error)
	, token{ token }
	, stack_trace{ stack_trace }
{}

const token_t &rossa_error::getToken() const
{
	return token;
}

const trace_t &rossa_error::getTrace() const
{
	return stack_trace;
}