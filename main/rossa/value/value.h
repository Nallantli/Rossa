#ifndef VALUE_H
#define VALUE_H

#include <variant>

#include "../rossa.h"

#include "../object/object.h"
#include "../wrapper/wrapper.h"
#include "../parameter/parameter.h"
#include "../symbol/symbol.h"

class value_t
{
	friend class symbol_t;

public:
	const unsigned int hash() const;
	value_type_enum type;

private:
	std::variant<
		std::monostate,
		bool,
		number_t,
		parameter_t,
		std::string,
		std::shared_ptr<void>,
		std::vector<symbol_t>,
		wrapper_t,
		std::map<const std::string, const symbol_t>,
		object_t>
		value;

	refc_ull references = 1;

	~value_t();
	value_t();
	value_t(const parameter_t &);
	value_t(const bool &);
	value_t(const std::shared_ptr<void> &);
	value_t(const object_t &);
	value_t(const signature_t &, const ptr_function_t &);
	value_t(const ptr_function_t &);
	value_t(const number_t &);
	value_t(const std::vector<symbol_t> &);
	value_t(const std::map<const std::string, const symbol_t> &);
	value_t(const std::string &);
	void clearData();
};

#endif