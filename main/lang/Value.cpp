#include "Rossa.h"

f_wrapper::f_wrapper(const f_map_t &map, const func_ptr_t &varg)
	: map{ map }
	, varg{ varg }
{}

Value::Value()
	: type{ NIL }
{}

Value::Value(const param_t &valueType)
	: type{ TYPE_NAME }
	, value{ valueType }
{}

Value::Value(const bool &valueBool)
	: type{ BOOLEAN_D }
	, value{ valueBool }
{}

Value::Value(const std::shared_ptr<void> &valuePointer)
	: type{ POINTER }
	, value{ valuePointer }
{}

Value::Value(const scope_t &valueObject)
	: type{ OBJECT }
	, value{ valueObject }
{}

Value::Value(const fsig_t &ftype, const func_ptr_t &function)
	: type{ FUNCTION }
	, value{ f_wrapper({ {function->getArgSize(), {{ftype, function}}} }, nullptr) }
{}

Value::Value(const func_ptr_t &function)
	: type{ FUNCTION }
	, value{ f_wrapper({}, function) }
{}

Value::Value(const number_t &valueNumber)
	: type{ NUMBER }
	, value{ valueNumber }
{}

Value::Value(const sym_vec_t &valueVector)
	: type{ ARRAY }
	, value{ valueVector }
{}

Value::Value(const sym_map_t &valueDictionary)
	: type{ DICTIONARY }
	, value{ valueDictionary }
{}

Value::Value(const std::string &valueString)
	: type{ STRING }
	, value{ valueString }
{}

void Value::clearData()
{
	value = false;
}