#include "../../bin/include/Rossa.h"

using namespace rossa;

RTError::RTError(const std::string &error, const Token &token, const trace_t &stack_trace)
	: std::runtime_error(error)
	, token{ token }
	, stack_trace{ stack_trace }
{}

const Token &RTError::getToken() const
{
	return token;
}

const trace_t &RTError::getTrace() const
{
	return stack_trace;
}