#include "value.h"

#include "../function/function.h"
#include "../signature/signature.h"

value_t::value_t()
	: type{NIL}
{
}

value_t::value_t(const parameter_t &valueType)
	: type{TYPE_NAME}, value{valueType}
{
}

value_t::value_t(const bool &valueBool)
	: type{BOOLEAN_D}, value{valueBool}
{
}

value_t::value_t(const std::shared_ptr<void> &valuePointer)
	: type{POINTER}, value{valuePointer}
{
}

value_t::value_t(const object_t &valueObject)
	: type{OBJECT}, value{valueObject}
{
}

value_t::value_t(const signature_t &ftype, const ptr_function_t &function)
	: type{FUNCTION}, value{wrapper_t({{function->params.size(), {{ftype, function}}}}, nullptr)}
{
}

value_t::value_t(const ptr_function_t &function)
	: type{FUNCTION}, value{wrapper_t({}, function)}
{
}

value_t::value_t(const number_t &valueNumber)
	: type{NUMBER}, value{valueNumber}
{
}

value_t::value_t(const std::vector<symbol_t> &valueVector)
	: type{ARRAY}, value{valueVector}
{
}

value_t::value_t(const std::map<const std::string, const symbol_t> &valueDictionary)
	: type{DICTIONARY}, value{valueDictionary}
{
}

value_t::value_t(const std::string &valueString)
	: type{STRING}, value{valueString}
{
}

void value_t::clearData()
{
	value = std::monostate();
}

const unsigned int value_t::hash() const
{
	switch (type)
	{
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
		for (auto &e : std::get<std::vector<symbol_t>>(value))
		{
			h = (h + (e.hash() << i++)) % 0x0FFFFFFF;
		}
		return 0x30000000 | h;
	}
	case STRING:
	{
		int h = 0;
		int i = 0;
		for (auto &c : std::get<std::string>(value))
		{
			h = (h + ((int)c << i++)) % 0x0FFFFFFF;
		}
		return 0x40000000 | h;
	}
	case OBJECT:
		return 0x50000000 | (std::get<object_t>(value).hash() % 0x0FFFFFFF);
	case DICTIONARY:
	{
		int h = 0;
		int i = 0;
		for (auto &e : std::get<std::map<const std::string, const symbol_t>>(value))
		{
			h = (h + (e.second.hash() << i++)) % 0x0FFFFFFF;
		}
		return 0x60000000 | h;
	}
	case FUNCTION:
		return 0x70000000 | (std::get<wrapper_t>(value).hash() % 0x0FFFFFFF);
	case TYPE_NAME:
		return 0x80000000 | (std::get<parameter_t>(value).hash() % 0x0FFFFFFF);
	default:
		return 0;
	}
}