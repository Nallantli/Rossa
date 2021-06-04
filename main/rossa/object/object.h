#ifndef OBJECT_H
#define OBJECT_H

#include "../rossa.h"

struct object_t
{
private:
	scope_t *scope;

public:
	object_type_enum type;

	const unsigned int hash() const;

	object_t();
	object_t(scope_t *, const object_type_enum &);
	object_t(const hash_ull &key);
	object_t(const object_t *, const hash_ull &);
	object_t(const object_t *, const scope_type_enum &, const ptr_instruction_t &, const hash_ull &, const object_t *, const std::vector<aug_type_t> &);
	object_t(scope_t *, const aug_type_t &, const std::vector<aug_type_t> &);

	object_t(const object_t &);
	~object_t();
	void operator=(const object_t &);
	const bool operator==(const object_t &) const;

	const symbol_t instantiate(const std::vector<symbol_t> &, const token_t *, trace_t &) const;
	const bool extendsObject(const aug_type_t &) const;
	const scope_type_enum getType() const;
	const ptr_instruction_t getBody() const;
	const parameter_t getTypeVec() const;
	const std::string getKey() const;
	const bool hasValue(const hash_ull &) const;
	const symbol_t getThis(const token_t *, trace_t &) const;

	const symbol_t &getVariable(const hash_ull &, const token_t *, trace_t &) const;
	const symbol_t &createVariable(const hash_ull &, const token_t *) const;
	const symbol_t &createVariable(const hash_ull &, const symbol_t &, const token_t *) const;

	scope_t *getPtr() const;
};

#endif