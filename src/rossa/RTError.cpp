#include "../../bin/include/Rossa.h"

RTError::RTError(const std::string &error, const token_t &token, const trace_t &stack_trace)
	: std::runtime_error(error)
	, token{ token }
	, stack_trace{ stack_trace }
{}

const token_t &RTError::getToken() const
{
	return token;
}

const trace_t &RTError::getTrace() const
{
	return stack_trace;
}