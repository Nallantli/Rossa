#ifndef SCOPE_H
#define SCOPE_H

#include "../rossa.h"

#include "../object/object.h"
#include "../instruction/instruction.h"
#include "../symbol/symbol.h"

class scope_t
{
	friend class object_t;

public:
	scope_t *getParent() const;
	const unsigned int hash() const;

private:
	const scope_type_enum type;
	scope_t *const parent;
	refc_ull references = 1;
	std::map<const hash_ull, const symbol_t> values;
	const ptr_instruction_t body;
	//hash_ull hashed_key;
	aug_type_t name_trace;
	std::vector<aug_type_t> extensions;

	void traceName(const hash_ull &);

	scope_t(const scope_type_enum &, scope_t *, const ptr_instruction_t &, const hash_ull &);
	scope_t(scope_t *, const aug_type_t &, const std::vector<aug_type_t> &);

	const symbol_t &getVariable(const hash_ull &, const token_t *, trace_t &) const;
	const symbol_t &createVariable(const hash_ull &, const token_t *);
	const symbol_t &createVariable(const hash_ull &, const symbol_t &, const token_t *);
	const symbol_t getThis(const token_t *, trace_t &);

	~scope_t();
};

#endif