#ifndef FUNCTION_H
#define FUNCTION_H

#include "../rossa.h"

struct function_t : public std::enable_shared_from_this<function_t>
{
	const hash_ull key;
	scope_t *parent;
	const std::vector<std::pair<token_type_enum, hash_ull>> params;
	const ptr_instruction_t body;
	const std::map<const hash_ull, const symbol_t> captures;
	const bool isVargs;

	function_t(const hash_ull &, scope_t *, const std::vector<std::pair<token_type_enum, hash_ull>> &, const ptr_instruction_t &, const std::map<const hash_ull, const symbol_t> &);
	function_t(const hash_ull &, scope_t *, const ptr_instruction_t &, const std::map<const hash_ull, const symbol_t> &);
	const object_t getParent() const;
	void shift();
};

const symbol_t function_evaluate(const ptr_function_t &, const std::vector<symbol_t> &, const token_t *, trace_t &);
const symbol_t function_evaluate_vargs(const ptr_function_t &, const std::vector<symbol_t> &, const token_t *, trace_t &);

#endif