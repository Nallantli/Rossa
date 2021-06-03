#include "Rossa.h"

f_wrapper::f_wrapper(const f_map_t &map, const func_ptr_t &varg)
	: map{ map }
	, varg{ varg }
{}

const unsigned int f_wrapper::hash() const
{
	int h = 0;
	int i = 0;
	for (auto &e : map) {
		for (auto &f : e.second){
		h = (h + (f.second->key << i++)) % 0xFFFFFFFF;
		}
	}
	return h;
}

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
	, value{ f_wrapper({ {function->params.size(), {{ftype, function}}} }, nullptr) }
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

const unsigned int Value::hash() const
{
	switch (type) {
		case NIL:
			return 0x00000000;
		case BOOLEAN_D:
			return std::get<bool>(value) ? 0x10000001 : 0x10000000;
		case NUMBER:
			return 0x20000000 | (std::get<number_t>(value).type == number_t::LONG_NUM ? 0x00000000 : 0x0100000) | (std::get<number_t>(value).getLong() % 0x00FFFFFF);
		case ARRAY:
		{
			int h = 0;
			int i = 0;
			for (auto &e : std::get<sym_vec_t>(value)) {
				h = (h + (e.hash() << i++)) % 0x0FFFFFFF;
			}
			return 0x30000000 | h;
		}
		case STRING:
		{
			int h = 0;
			int i = 0;
			for (auto &c : std::get<std::string>(value)) {
				h = (h + ((int)c << i++)) % 0x0FFFFFFF;
			}
			return 0x40000000 | h;
		}
		case OBJECT:
			return 0x50000000 | (std::get<scope_t>(value).hash() % 0x0FFFFFFF);
		case DICTIONARY:
		{
			int h = 0;
			int i = 0;
			for (auto &e : std::get<sym_map_t>(value)) {
				h = (h + (e.second.hash() << i++)) % 0x0FFFFFFF;
			}
			return 0x60000000 | h;
		}
		case FUNCTION:
			return 0x70000000 | (std::get<f_wrapper>(value).hash() % 0x0FFFFFFF);
		case TYPE_NAME:
			return 0x80000000 | (std::get<param_t>(value).hash() % 0x0FFFFFFF);
		default:
			return 0;
	}
}