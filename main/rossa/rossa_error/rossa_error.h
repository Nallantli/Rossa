#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

#include "../rossa.h"
#include "../tokenizer/tokenizer.h"

typedef std::vector<std::pair<token_t, function_t>> trace_t;

class rossa_error_t : public std::runtime_error
{
private:
	const token_t token;
	const trace_t stack_trace;

public:
	rossa_error_t(const std::string &, const token_t &, const trace_t &);
	const token_t &getToken() const;
	const trace_t &getTrace() const;
};

#endif