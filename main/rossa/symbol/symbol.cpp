#include "symbol.h"

#include "../value/value.h"
#include "../rossa_error/rossa_error.h"
#include "../parser/parser.h"
#include "../function/function.h"
#include "../wrapper/wrapper.h"
#include "../signature/signature.h"
#include "../global/global.h"

symbol_t::symbol_t()
	: d{new value_t()}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const type_t &type)
	: d{new value_t()}, type{type}
{
}

symbol_t::symbol_t(const std::shared_ptr<void> &valuePointer)
	: d{new value_t(valuePointer)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const parameter_t &valueType)
	: d{new value_t(valueType)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const number_t &valueNumber)
	: d{new value_t(valueNumber)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const bool &valueBool)
	: d{new value_t(valueBool)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const std::vector<symbol_t> &valueVector)
	: d{new value_t(valueVector)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const object_t &valueObject)
	: d{new value_t(valueObject)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const signature_t &ftype, const ptr_function_t &valueFunction)
	: d{new value_t(ftype, valueFunction)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const ptr_function_t &valueFunction)
	: d{new value_t(valueFunction)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const std::string &valueString)
	: d{new value_t(valueString)}, type{ID_CASUAL}
{
}

symbol_t::symbol_t(const std::map<const std::string, const symbol_t> &valueDictionary)
	: d{new value_t(valueDictionary)}, type{ID_CASUAL}
{
}

const symbol_t symbol_t::Pointer(const std::shared_ptr<void> &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::TypeName(const parameter_t &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::Number(const number_t &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::Boolean(const bool &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::Array(const std::vector<symbol_t> &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::Object(const object_t &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::FunctionSIG(const signature_t &s, const ptr_function_t &v)
{
	return symbol_t(s, v);
}

const symbol_t symbol_t::FunctionVARG(const ptr_function_t &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::String(const std::string &v)
{
	return symbol_t(v);
}

const symbol_t symbol_t::Dictionary(const std::map<const std::string, const symbol_t> &v)
{
	return symbol_t(v);
}

symbol_t::symbol_t(const symbol_t &s)
	: d{s.d}, type{s.type}
{
	this->d->references++;
}

symbol_t::~symbol_t()
{
	d->references--;
	if (d->references == 0)
		delete d;
}

void symbol_t::operator=(const symbol_t &b)
{
	this->d->references--;
	if (this->d->references == 0)
		delete d;

	this->d = b.d;
	this->type = b.type;
	this->d->references++;
}

const unsigned int symbol_t::hash() const
{
	return d->hash();
}

const symbol_t symbol_t::allocate(const size_t &size)
{
	std::vector<symbol_t> v(size);
	return symbol_t(v);
}

const symbol_t symbol_t::allocateAs(const size_t &size, const symbol_t *value, const token_t *token, trace_t &stack_trace)
{
	std::vector<symbol_t> v(size);
	for (auto e : v)
	{
		e.set(value, token, stack_trace);
	}
	return symbol_t(v);
}

const symbol_t::type_t symbol_t::getSymbolType() const
{
	return type;
}

void symbol_t::setSymbolType(const type_t &type)
{
	this->type = type;
}

const number_t &symbol_t::getNumber(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::NUMBER)
	{
		throw rossa_error_t(_NOT_NUMBER_, *token, stack_trace);
	}
	return std::get<number_t>(d->value);
}

const std::shared_ptr<void> symbol_t::getPointer(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::POINTER)
	{
		throw rossa_error_t(_NOT_POINTER_, *token, stack_trace);
	}
	return std::get<std::shared_ptr<void>>(d->value);
}

std::map<const std::string, const symbol_t> &symbol_t::getDictionary(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::DICTIONARY)
	{
		throw rossa_error_t(_NOT_DICTIONARY_, *token, stack_trace);
	}
	return std::get<std::map<const std::string, const symbol_t>>(d->value);
}

const symbol_t &symbol_t::indexVector(const size_t &i, const token_t *token, trace_t &stack_trace) const
{
	auto &v = std::get<std::vector<symbol_t>>(d->value);
	if (i >= v.size())
	{
		throw rossa_error_t(global::format(_INDEX_OUT_OF_BOUNDS_, {std::to_string(v.size()), std::to_string(i)}), *token, stack_trace);
	}
	return v.at(i);
}

const std::vector<symbol_t> &symbol_t::getVector(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::ARRAY)
	{
		throw rossa_error_t(_NOT_VECTOR_, *token, stack_trace);
	}
	return std::get<std::vector<symbol_t>>(d->value);
}

const std::string symbol_t::getString(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::STRING)
	{
		throw rossa_error_t(_NOT_STRING_, *token, stack_trace);
	}
	return std::get<std::string>(d->value);
}

const bool symbol_t::getBool(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::BOOLEAN_D)
	{
		throw rossa_error_t(_NOT_BOOLEAN_, *token, stack_trace);
	}
	return std::get<bool>(d->value);
}

object_t *symbol_t::getObject(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::OBJECT)
	{
		throw rossa_error_t(_NOT_OBJECT_, *token, stack_trace);
	}
	return &std::get<object_t>(d->value);
}

const value_type_enum symbol_t::getValueType() const
{
	return d->type;
}

const parameter_t symbol_t::getAugValueType() const
{
	if (d->type == value_type_enum::OBJECT)
	{
		return std::get<object_t>(d->value).getTypeVec();
	}
	return parameter_t({}, {d->type});
}

const parameter_t symbol_t::getTypeName(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::TYPE_NAME)
	{
		throw rossa_error_t(_NOT_TYPE_, *token, stack_trace);
	}
	return std::get<parameter_t>(d->value);
}

const std::map<const size_t, std::map<const signature_t, ptr_function_t>> &symbol_t::getFunctionOverloads(const token_t *token, trace_t &stack_trace) const
{
	switch (d->type)
	{
	case value_type_enum::FUNCTION:
		return std::get<wrapper_t>(d->value).map;
	case value_type_enum::OBJECT:
		return std::get<object_t>(d->value).getVariable(parser_t::HASH_CALL, token, stack_trace).getFunctionOverloads(token, stack_trace);
	default:
		throw rossa_error_t(_NOT_FUNCTION_, *token, stack_trace);
	}
}

const bool symbol_t::hasVarg(const token_t *token, trace_t &stack_trace) const
{
	switch (d->type)
	{
	case value_type_enum::FUNCTION:
		return std::get<wrapper_t>(d->value).varg != nullptr;
	case value_type_enum::OBJECT:
		return std::get<object_t>(d->value).getVariable(parser_t::HASH_CALL, token, stack_trace).hasVarg(token, stack_trace);
	default:
		throw rossa_error_t(_NOT_FUNCTION_, *token, stack_trace);
	}
}

const ptr_function_t symbol_t::getFunction(const std::vector<symbol_t> &params, const token_t *token, trace_t &stack_trace) const
{
	if (d->type != value_type_enum::FUNCTION)
	{
		throw rossa_error_t(_NOT_FUNCTION_, *token, stack_trace);
	}

	const auto it = std::get<wrapper_t>(d->value).map.find(params.size());
	if (it == std::get<wrapper_t>(d->value).map.end())
	{
		if (std::get<wrapper_t>(d->value).varg != nullptr)
		{
			return std::get<wrapper_t>(d->value).varg;
		}
		throw rossa_error_t(_FUNCTION_ARG_SIZE_FAILURE_, *token, stack_trace);
	}

	ptr_function_t f = nullptr;
	size_t cur_v = 0;
	for (auto &f2 : it->second)
	{
		size_t v = f2.first.validity(params, stack_trace);
		if (v > cur_v)
		{
			cur_v = v;
			f = f2.second;
			if (v == params.size() * 3)
			{
				break;
			}
		}
	}

	if (f == nullptr)
	{
		if (std::get<wrapper_t>(d->value).varg != nullptr)
		{
			return std::get<wrapper_t>(d->value).varg;
		}
		throw rossa_error_t(_FUNCTION_VALUE_NOT_EXIST_, *token, stack_trace);
	}

	return f;
}

const symbol_t &symbol_t::indexDict(const std::string &key) const
{
	return std::get<std::map<const std::string, const symbol_t>>(d->value)[key];
}

const bool symbol_t::hasDictionaryKey(const std::string &key) const
{
	return std::get<std::map<const std::string, const symbol_t>>(d->value).find(key) != std::get<std::map<const std::string, const symbol_t>>(d->value).end();
}

const size_t symbol_t::vectorSize() const
{
	return std::get<std::vector<symbol_t>>(d->value).size();
}

const size_t symbol_t::dictionarySize(const token_t *token, trace_t &stack_trace) const
{
	return getDictionary(token, stack_trace).size();
}

const std::string symbol_t::toString(const token_t *token, trace_t &stack_trace) const
{
	switch (d->type)
	{
	case value_type_enum::NIL:
		return KEYWORD_NIL;
	case value_type_enum::NUMBER:
		return std::get<number_t>(d->value).toString();
	case value_type_enum::STRING:
		return std::get<std::string>(d->value);
	case value_type_enum::FUNCTION:
	{
		std::string ret = "[";
		size_t i = 0;
		for (auto &e : std::get<wrapper_t>(d->value).map)
		{
			for (auto &t : e.second)
			{
				if (i++ > 0)
				{
					ret += ", ";
				}
				ret += "Function<" + t.first.toString() + ">";
			}
		}
		if (std::get<wrapper_t>(d->value).varg != nullptr)
		{
			if (i > 0)
			{
				ret += ", ";
			}
			ret += "Function<...>";
		}
		return ret + "]";
	}
	case value_type_enum::OBJECT:
	{
		if (std::get<object_t>(d->value).hasValue(ROSSA_HASH("->String")))
		{
			return std::get<object_t>(d->value).getVariable(ROSSA_HASH("->String"), token, stack_trace).call({}, token, stack_trace).getString(token, stack_trace);
		}
		std::stringstream ss;
		ss << "Object<" << std::get<object_t>(d->value).getKey() << ">";
		return ss.str();
	}
	case value_type_enum::POINTER:
	{
		std::stringstream ss;
		ss << "Pointer<" << std::addressof(std::get<std::shared_ptr<void>>(d->value)) << ">";
		return ss.str();
	}
	case value_type_enum::BOOLEAN_D:
		return std::get<bool>(d->value) ? KEYWORD_TRUE : KEYWORD_FALSE;
	case value_type_enum::ARRAY:
	{
		std::string ret = "[";
		unsigned int i = 0;
		for (auto &d2 : std::get<std::vector<symbol_t>>(d->value))
		{
			if (i > 0)
			{
				ret += ", ";
			}
			ret += d2.toString(token, stack_trace);
			i++;
		}
		return ret + "]";
	}
	case value_type_enum::DICTIONARY:
	{
		std::string ret = "{";
		unsigned int i = 0;
		for (auto &e : getDictionary(token, stack_trace))
		{
			if (i > 0)
			{
				ret += ", ";
			}
			ret += "\"" + e.first + "\" : " + e.second.toString(token, stack_trace);
			i++;
		}
		return ret + "}";
	}
	case value_type_enum::TYPE_NAME:
		return "Type<" + std::get<parameter_t>(d->value).toString() + ">";
	default:
		return "undefined";
	}
}

const std::string symbol_t::toCodeString() const
{
	if (type != ID_CASUAL)
	{
		return "symbol_t(static_cast<SymbolType>(" + std::to_string(type) + "))";
	}
	trace_t stack_trace;
	switch (d->type)
	{
	case value_type_enum::NIL:
		return "Nil@nil";
	case value_type_enum::NUMBER:
		return "Number@" + std::get<number_t>(d->value).toCodeString();
	case value_type_enum::STRING:
		return "String@\"" + std::get<std::string>(d->value) + "\"";
	case value_type_enum::FUNCTION:
		return "Function@" + getString(NULL, stack_trace);
	case value_type_enum::OBJECT:
		return "Object@" + std::get<object_t>(d->value).getKey();
	case value_type_enum::POINTER:
	{
		std::stringstream ss;
		ss << "Pointer@" << std::addressof(std::get<std::shared_ptr<void>>(d->value));
		return ss.str();
	}
	case value_type_enum::BOOLEAN_D:
		return "Boolean@" + std::string(std::get<bool>(d->value) ? KEYWORD_TRUE : KEYWORD_FALSE);
	case value_type_enum::ARRAY:
	{
		std::string ret = "Array@[";
		unsigned int i = 0;
		for (auto &d2 : std::get<std::vector<symbol_t>>(d->value))
		{
			if (i > 0)
			{
				ret += ", ";
			}
			ret += d2.toCodeString();
			i++;
		}
		return ret + "]";
	}
	case value_type_enum::DICTIONARY:
	{
		std::string ret = "Dictionary@[";
		unsigned int i = 0;
		for (auto &e : getDictionary(NULL, stack_trace))
		{
			if (i > 0)
			{
				ret += ", ";
			}
			ret += "\"" + e.first + "\" : " + e.second.toCodeString();
			i++;
		}
		return ret + "]";
	}
	case value_type_enum::TYPE_NAME:
		return "Type@" + std::get<parameter_t>(d->value).toString();
	default:
		return "<error-type>";
	}
}

const symbol_t symbol_t::call(const std::vector<symbol_t> &params, const token_t *token, trace_t &stack_trace) const
{
	return function_evaluate(getFunction(params, token, stack_trace), params, token, stack_trace);
}

void symbol_t::addFunctions(const symbol_t *b, const token_t *token) const
{
	auto fs = std::get<wrapper_t>(b->d->value).map;
	for (auto &f : fs)
	{
		for (auto &t : f.second)
		{
			std::get<wrapper_t>(d->value).map[f.first][t.first] = t.second;
		}
	}
	if (std::get<wrapper_t>(b->d->value).varg != nullptr)
		std::get<wrapper_t>(d->value).varg = std::get<wrapper_t>(b->d->value).varg;
}

const ptr_function_t &symbol_t::getVARGFunction(const token_t *token, trace_t &stack_trace) const
{
	if (d->type == value_type_enum::FUNCTION)
	{
		return std::get<wrapper_t>(d->value).varg;
	}
	if (d->type == value_type_enum::OBJECT)
	{
		return std::get<object_t>(d->value).getVariable(parser_t::HASH_CALL, token, stack_trace).getVARGFunction(token, stack_trace);
	}
	throw rossa_error_t(_NOT_FUNCTION_, *token, stack_trace);
}

void symbol_t::nullify() const
{
	d->clearData();
	d->type = value_type_enum::NIL;
}

void symbol_t::set(const symbol_t *b, const token_t *token, trace_t &stack_trace) const
{
	if (b->d == d)
	{
		return;
	}
	if (d->type == value_type_enum::OBJECT && std::get<object_t>(d->value).hasValue(parser_t::HASH_SET))
	{
		ptr_function_t f = nullptr;
		try
		{
			f = std::get<object_t>(d->value)
					.getVariable(parser_t::HASH_SET, token, stack_trace)
					.getFunction({*b}, token, stack_trace);
		}
		catch (const rossa_error_t &e)
		{
		}
		if (f)
		{
			function_evaluate(f, {*b}, token, stack_trace);
			return;
		}
	}
	switch (b->d->type)
	{
	case value_type_enum::NIL:
		d->clearData();
		break;
	case value_type_enum::ARRAY:
	{
		auto v = &std::get<std::vector<symbol_t>>(b->d->value);
		std::vector<symbol_t> nv(v->size());
		for (size_t i = 0; i < v->size(); i++)
		{
			nv[i].set(&v->at(i), token, stack_trace);
		}
		d->value = nv;
		break;
	}
	case value_type_enum::DICTIONARY:
	{
		d->value = std::map<const std::string, const symbol_t>();
		auto v = &std::get<std::map<const std::string, const symbol_t>>(b->d->value);
		for (auto &e : *v)
		{
			if (e.second.d->type == value_type_enum::NIL)
			{
				continue;
			}
			auto newd = symbol_t();
			newd.set(&e.second, token, stack_trace);
			std::get<std::map<const std::string, const symbol_t>>(d->value).insert({e.first, newd});
		}
		break;
	}
	default:
		d->value = b->d->value;
		d->references++;
		break;
	}
	d->type = b->d->type;
}

const bool symbol_t::equals(const symbol_t *b, const token_t *token, trace_t &stack_trace) const
{
	if (d->type != b->d->type && d->type != value_type_enum::OBJECT)
	{
		return false;
	}
	switch (d->type)
	{
	case value_type_enum::NIL:
		return true;
	case value_type_enum::NUMBER:
		return std::get<number_t>(d->value) == std::get<number_t>(b->d->value);
	case value_type_enum::BOOLEAN_D:
		return std::get<bool>(d->value) == std::get<bool>(b->d->value);
	case value_type_enum::STRING:
		return std::get<std::string>(d->value) == std::get<std::string>(b->d->value);
	case value_type_enum::OBJECT:
	{
		auto o = std::get<object_t>(d->value);
		if (o.hasValue(parser_t::HASH_EQUALS))
		{
			return o.getVariable(parser_t::HASH_EQUALS, token, stack_trace).call({*b}, token, stack_trace).getBool(token, stack_trace);
		}
		return o == std::get<object_t>(b->d->value);
	}
	case value_type_enum::ARRAY:
	{
		auto bv = std::get<std::vector<symbol_t>>(b->d->value);
		if (std::get<std::vector<symbol_t>>(d->value).size() != bv.size())
		{
			return false;
		}
		for (unsigned long i = 0; i < std::get<std::vector<symbol_t>>(d->value).size(); i++)
		{
			if (!std::get<std::vector<symbol_t>>(d->value)[i].equals(&bv[i], token, stack_trace))
			{
				return false;
			}
		}
		return true;
	}
	case value_type_enum::DICTIONARY:
		for (auto &e : std::get<std::map<const std::string, const symbol_t>>(d->value))
		{
			if (!e.second.equals(&std::get<std::map<const std::string, const symbol_t>>(b->d->value)[e.first], token, stack_trace))
			{
				return false;
			}
		}
		return true;
	case value_type_enum::FUNCTION:
		return std::get<wrapper_t>(d->value).map == std::get<wrapper_t>(b->d->value).map && std::get<wrapper_t>(d->value).varg == std::get<wrapper_t>(b->d->value).varg;
	case value_type_enum::TYPE_NAME:
		return std::get<parameter_t>(d->value) == std::get<parameter_t>(b->d->value);
	default:
		return false;
	}
}

const bool symbol_t::nequals(const symbol_t *b, const token_t *token, trace_t &stack_trace) const
{
	switch (d->type)
	{
	case value_type_enum::OBJECT:
	{
		auto &o = std::get<object_t>(d->value);
		if (o.hasValue(parser_t::HASH_NEQUALS))
		{
			return o.getVariable(parser_t::HASH_NEQUALS, token, stack_trace).call({*b}, token, stack_trace).getBool(token, stack_trace);
		}
	}
	default:
		return !this->equals(b, token, stack_trace);
	}
}

const bool symbol_t::pureEquals(const symbol_t *b, const token_t *token, trace_t &stack_trace) const
{
	if (d->type != b->d->type)
	{
		return false;
	}
	switch (d->type)
	{
	case value_type_enum::OBJECT:
		return std::get<object_t>(d->value) == std::get<object_t>(b->d->value);
	default:
		return this->equals(b, token, stack_trace);
	}
}

const bool symbol_t::pureNEquals(const symbol_t *b, const token_t *token, trace_t &stack_trace) const
{
	return !this->pureEquals(b, token, stack_trace);
}

const bool symbol_t::operator==(const symbol_t &b) const
{
	trace_t stack_trace;
	return this->equals(&b, NULL, stack_trace);
}

const bool symbol_t::operator!=(const symbol_t &b) const
{
	trace_t stack_trace;
	return this->nequals(&b, NULL, stack_trace);
}

const bool symbol_t::operator<(const symbol_t &b) const
{
	return this->toCodeString() < b.toCodeString();
}

void symbol_t::shift() const
{
	if (d->type != value_type_enum::FUNCTION)
	{
		return;
	}
	for (auto &e : std::get<wrapper_t>(d->value).map)
	{
		for (auto &f : e.second)
		{
			f.second->shift();
		}
	}
	if (std::get<wrapper_t>(d->value).varg != nullptr)
	{
		std::get<wrapper_t>(d->value).varg->shift();
	}
}

const symbol_t symbol_t::clone() const
{
	return symbol_t(*this);
}