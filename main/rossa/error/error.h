#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

#include "../rossa.h"

class error_t : public std::runtime_error
{
private:
	const token_t token;
	const trace_t stack_trace;

public:
	error_t(const std::string &, const token_t &, const trace_t &);
	const token_t &getToken() const;
	const trace_t &getTrace() const;
};

#endif