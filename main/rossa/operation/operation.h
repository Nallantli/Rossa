#ifndef OPERATION_H
#define OPERATION_H

#include "../rossa.h"

namespace operation
{
	const symbol_t index(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t call(const object_t *, const ptr_instruction_t &, const std::vector<symbol_t> &, const token_t *, trace_t &);
	const symbol_t untilstep(const object_t *, const bool &, const symbol_t &, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t untilnostep(const object_t *, const bool &, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t cct(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t del(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	// Arithmetic
	const symbol_t add(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t sub(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t mul(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t div(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t mod(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t pow(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	// Comparison
	const symbol_t less(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t more(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t eless(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t emore(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	// Bit-Wise
	const symbol_t bor(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t bxor(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t band(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t bshl(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t bshr(const object_t *, const symbol_t &, const symbol_t &, const token_t *, trace_t &);
	const symbol_t bnot(const object_t *, const symbol_t &, const token_t *, trace_t &);
	// unary
	const symbol_t unadd(const object_t *, const symbol_t &, const token_t *, trace_t &);
	const symbol_t neg(const object_t *, const symbol_t &, const token_t *, trace_t &);
	const symbol_t unot(const object_t *, const symbol_t &, const token_t *, trace_t &);
	const symbol_t hash(const object_t *, const symbol_t &, const token_t *, trace_t &);
}

#endif