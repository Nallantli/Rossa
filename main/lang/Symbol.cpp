#include "Rossa.h"

sym_t::sym_t()
	: d{ new Value() }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const type_t &type)
	: d{ new Value() }
	, type{ type }
{}

sym_t::sym_t(const std::shared_ptr<void> &valuePointer)
	: d{ new Value(valuePointer) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const param_t &valueType)
	: d{ new Value(valueType) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const number_t &valueNumber)
	: d{ new Value(valueNumber) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const bool &valueBool)
	: d{ new Value(valueBool) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const sym_vec_t &valueVector)
	: d{ new Value(valueVector) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const scope_t &valueObject)
	: d{ new Value(valueObject) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const fsig_t &ftype, const func_ptr_t &valueFunction)
	: d{ new Value(ftype, valueFunction) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const func_ptr_t &valueFunction)
	: d{ new Value(valueFunction) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const std::string &valueString)
	: d{ new Value(valueString) }
	, type{ ID_CASUAL }
{}

sym_t::sym_t(const sym_map_t &valueDictionary)
	: d{ new Value(valueDictionary) }
	, type{ ID_CASUAL }
{}

const sym_t sym_t::Pointer(const std::shared_ptr<void> &v)
{
	return sym_t(v);
}

const sym_t sym_t::TypeName(const param_t &v)
{
	return sym_t(v);
}

const sym_t sym_t::Number(const number_t &v)
{
	return sym_t(v);
}

const sym_t sym_t::Boolean(const bool &v)
{
	return sym_t(v);
}

const sym_t sym_t::Array(const sym_vec_t &v)
{
	return sym_t(v);
}

const sym_t sym_t::Object(const scope_t &v)
{
	return sym_t(v);
}

const sym_t sym_t::FunctionSIG(const fsig_t &s, const func_ptr_t &v)
{
	return sym_t(s, v);
}

const sym_t sym_t::FunctionVARG(const func_ptr_t &v)
{
	return sym_t(v);
}

const sym_t sym_t::String(const std::string &v)
{
	return sym_t(v);
}

const sym_t sym_t::Dictionary(const sym_map_t &v)
{
	return sym_t(v);
}

sym_t::sym_t(const sym_t &s)
{
	this->d = s.d;
	this->type = s.type;
	this->d->references++;
}

sym_t::~sym_t()
{
	d->references--;
	if (d->references == 0)
		delete d;
}

void sym_t::operator=(const sym_t &b)
{
	this->d->references--;
	if (this->d->references == 0)
		delete d;

	this->d = b.d;
	this->type = b.type;
	this->d->references++;
}

const sym_t sym_t::allocate(const size_t &size)
{
	sym_vec_t v;
	v.resize(size);
	return sym_t(v);
}

const sym_t::type_t sym_t::getSymbolType() const
{
	return type;
}

void sym_t::setSymbolType(const type_t &type)
{
	this->type = type;
}

const number_t &sym_t::getNumber(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::NUMBER)
		throw rossa_error(_NOT_NUMBER_, *token, stack_trace);
	return std::get<number_t>(d->value);
}

void *sym_t::getPointer(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::POINTER)
		throw rossa_error(_NOT_POINTER_, *token, stack_trace);
	return std::get<std::shared_ptr<void>>(d->value).get();
}

const sym_map_t &sym_t::getDictionary(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::DICTIONARY)
		throw rossa_error(_NOT_DICTIONARY_, *token, stack_trace);
	auto &m = std::get<sym_map_t>(d->value);
	auto iter = m.begin();
	for (; iter != m.end();) {
		if (iter->second.d->type == Value::type_t::NIL)
			iter = m.erase(iter);
		else
			++iter;
	}
	return m;
}

const sym_t &sym_t::indexVector(const size_t &i, const token_t *token, trace_t &stack_trace) const
{
	auto &v = std::get<sym_vec_t>(d->value);
	if (i >= v.size())
		throw rossa_error(format::format(_INDEX_OUT_OF_BOUNDS_, { std::to_string(v.size()), std::to_string(i) }), *token, stack_trace);
	return v[i];
}

const sym_vec_t &sym_t::getVector(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::ARRAY)
		throw rossa_error(_NOT_VECTOR_, *token, stack_trace);
	return std::get<sym_vec_t>(d->value);
}

const std::string sym_t::getString(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::STRING)
		throw rossa_error(_NOT_STRING_, *token, stack_trace);
	return std::get<std::string>(d->value);
}

const bool sym_t::getBool(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::BOOLEAN_D)
		throw rossa_error(_NOT_BOOLEAN_, *token, stack_trace);
	return std::get<bool>(d->value);
}

scope_t *sym_t::getObject(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::OBJECT)
		throw rossa_error(_NOT_OBJECT_, *token, stack_trace);
	return &std::get<scope_t>(d->value);
}

const Value::type_t sym_t::getValueType() const
{
	return d->type;
}

const param_t sym_t::getAugValueType() const
{
	return std::visit(overloaded{
			[](const scope_t &v) {
				return v.getTypeVec();
			},
			[&](auto arg) {
				return param_t({},{ d->type });
			}
		}, d->value);
}

const param_t sym_t::getTypeName(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::TYPE_NAME)
		throw rossa_error(_NOT_TYPE_, *token, stack_trace);
	return std::get<param_t>(d->value);
}

const f_map_t &sym_t::getFunctionOverloads(const token_t *token, trace_t &stack_trace) const
{
	if (d->type == Value::type_t::FUNCTION)
		return std::get<f_wrapper>(d->value).map;
	if (d->type == Value::type_t::OBJECT)
		return std::get<scope_t>(d->value).getVariable(Rossa::HASH_CALL, token, stack_trace).getFunctionOverloads(token, stack_trace);
	throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);
}

const bool sym_t::hasVarg(const token_t *token, trace_t &stack_trace) const
{
	if (d->type == Value::type_t::FUNCTION)
		return std::get<f_wrapper>(d->value).varg != nullptr;
	if (d->type == Value::type_t::OBJECT)
		return std::get<scope_t>(d->value).getVariable(Rossa::HASH_CALL, token, stack_trace).hasVarg(token, stack_trace);
	throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);
}

const func_ptr_t sym_t::getFunction(const sym_vec_t &params, const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::FUNCTION)
		throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);

	const auto it = std::get<f_wrapper>(d->value).map.find(params.size());
	if (it == std::get<f_wrapper>(d->value).map.end()) {
		if (std::get<f_wrapper>(d->value).varg != nullptr)
			return std::get<f_wrapper>(d->value).varg;
		throw rossa_error(_FUNCTION_ARG_SIZE_FAILURE_, *token, stack_trace);
	}

	func_ptr_t f = nullptr;
	size_t cur_v = 0;
	for (auto &f2 : it->second) {
		size_t v = f2.first.validity(params, stack_trace);
		if (v > cur_v) {
			cur_v = v;
			f = f2.second;
			if (v == params.size() * 3)
				break;
		}
	}

	if (f == nullptr) {
		if (std::get<f_wrapper>(d->value).varg != nullptr)
			return std::get<f_wrapper>(d->value).varg;
		throw rossa_error(_FUNCTION_VALUE_NOT_EXIST_, *token, stack_trace);
	}

	return f;
}

const sym_t &sym_t::indexDict(const std::string &key) const
{
	return std::get<sym_map_t>(d->value)[key];
}

const bool sym_t::hasDictionaryKey(const std::string &key) const
{
	return std::get<sym_map_t>(d->value).find(key) != std::get<sym_map_t>(d->value).end();
}

const size_t sym_t::vectorSize() const
{
	return std::get<sym_vec_t>(d->value).size();
}

const size_t sym_t::dictionarySize(const token_t *token, trace_t &stack_trace) const
{
	return getDictionary(token, stack_trace).size();
}

const std::string sym_t::toString(const token_t *token, trace_t &stack_trace) const
{
	switch (d->type) {
		case Value::type_t::NIL:
			return KEYWORD_NIL;
		case Value::type_t::NUMBER:
			return std::get<number_t>(d->value).toString();
		case Value::type_t::STRING:
			return "\"" + std::get<std::string>(d->value) + "\"";
		case Value::type_t::FUNCTION:
		{
			std::string ret = "[";
			size_t i = 0;
			for (auto &e : std::get<f_wrapper>(d->value).map) {
				for (auto &t : e.second) {
					if (i++ > 0)
						ret += ", ";
					ret += "Function<" + t.first.toString() + ">";
				}
			}
			if (std::get<f_wrapper>(d->value).varg != nullptr) {
				if (i > 0)
					ret += ", ";
				ret += "Function<...>";
			}
			return ret + "]";
		}
		case Value::type_t::OBJECT:
		{
			if (std::get<scope_t>(d->value).hasValue(ROSSA_HASH("->String")))
				auto v = std::get<scope_t>(d->value).getVariable(ROSSA_HASH("->String"), token, stack_trace).call({}, token, stack_trace).getString(token, stack_trace);
			std::stringstream ss;
			ss << "Object<" << std::get<scope_t>(d->value).getKey() << ">";
			return ss.str();
		}
		case Value::type_t::POINTER:
		{
			std::stringstream ss;
			ss << "Pointer<" << std::addressof(std::get<std::shared_ptr<void>>(d->value)) << ">";
			return ss.str();
		}
		case Value::type_t::BOOLEAN_D:
			return std::get<bool>(d->value) ? KEYWORD_TRUE : KEYWORD_FALSE;
		case Value::type_t::ARRAY:
		{
			std::string ret = "[";
			unsigned int i = 0;
			for (auto &d2 : std::get<sym_vec_t>(d->value)) {
				if (i > 0)
					ret += ", ";
				ret += d2.toString(token, stack_trace);
				i++;
			}
			return ret + "]";
		}
		case Value::type_t::DICTIONARY:
		{
			std::string ret = "{";
			unsigned int i = 0;
			for (auto &e : getDictionary(token, stack_trace)) {
				if (i > 0)
					ret += ", ";
				ret += "\"" + e.first + "\" : " + e.second.toString(token, stack_trace);
				i++;
			}
			return ret + "}";
		}
		case Value::type_t::TYPE_NAME:
			return "Type<" + std::get<param_t>(d->value).toString() + ">";
		default:
			return "undefined";
	}
}

const std::string sym_t::toCodeString() const
{
	if (type != ID_CASUAL) {
		return "sym_t(static_cast<SymbolType>(" + std::to_string(type) + "))";
	}
	trace_t stack_trace;
	switch (d->type) {
		case Value::type_t::NIL:
			return "Nil@nil";
		case Value::type_t::NUMBER:
			return "Number@" + std::get<number_t>(d->value).toCodeString();
		case Value::type_t::STRING:
			return "String@\"" + std::get<std::string>(d->value) + "\"";
		case Value::type_t::FUNCTION:
			return "Function@" + getString(NULL, stack_trace);
		case Value::type_t::OBJECT:
			return "Object@" + std::get<scope_t>(d->value).getKey();
		case Value::type_t::POINTER:
		{
			std::stringstream ss;
			ss << "Pointer@" << std::addressof(std::get<std::shared_ptr<void>>(d->value));
			return ss.str();
		}
		case Value::type_t::BOOLEAN_D:
			return "Boolean@" + std::string(std::get<bool>(d->value) ? KEYWORD_TRUE : KEYWORD_FALSE);
		case Value::type_t::ARRAY:
		{
			std::string ret = "Array@[";
			unsigned int i = 0;
			for (auto &d2 : std::get<sym_vec_t>(d->value)) {
				if (i > 0)
					ret += ", ";
				ret += d2.toCodeString();
				i++;
			}
			return ret + "]";
		}
		case Value::type_t::DICTIONARY:
		{
			std::string ret = "Dictionary@[";
			unsigned int i = 0;
			for (auto &e : getDictionary(NULL, stack_trace)) {
				if (i > 0)
					ret += ", ";
				ret += "\"" + e.first + "\" : " + e.second.toCodeString();
				i++;
			}
			return ret + "]";
		}
		case Value::type_t::TYPE_NAME:
			return "Type@" + std::get<param_t>(d->value).toString();
		default:
			return "<error-type>";
	}
}

const sym_t sym_t::call(const sym_vec_t &params, const token_t *token, trace_t &stack_trace) const
{
	return getFunction(params, token, stack_trace)->evaluate(params, token, stack_trace);
}

void sym_t::addFunctions(const sym_t *b, const token_t *token) const
{
	auto fs = std::get<f_wrapper>(b->d->value).map;
	for (auto &f : fs)
		for (auto &t : f.second)
			std::get<f_wrapper>(d->value).map[f.first][t.first] = t.second;
	if (std::get<f_wrapper>(b->d->value).varg != nullptr)
		std::get<f_wrapper>(d->value).varg = std::get<f_wrapper>(b->d->value).varg;
}


const func_ptr_t &sym_t::getVARGFunction(const token_t *token, trace_t &stack_trace) const
{
	if (d->type == Value::type_t::FUNCTION)
		return std::get<f_wrapper>(d->value).varg;
	if (d->type == Value::type_t::OBJECT)
		return std::get<scope_t>(d->value).getVariable(Rossa::HASH_CALL, token, stack_trace).getVARGFunction(token, stack_trace);
	throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);
}

void sym_t::nullify() const
{
	d->clearData();
	d->type = Value::type_t::NIL;
}

void sym_t::set(const sym_t *b, const token_t *token, trace_t &stack_trace) const
{
	if (b->d == d)
		return;
	if (d->type == Value::type_t::OBJECT && std::get<scope_t>(d->value).hasValue(Rossa::HASH_SET)) {
		std::get<scope_t>(d->value).getVariable(Rossa::HASH_SET, token, stack_trace).call({ *b }, token, stack_trace);
		return;
	}
	d->type = b->d->type;
	if (d->type == Value::type_t::NIL) {
		d->value = false;
	} else {
		std::visit(overloaded{
			[&](const number_t &v) {
				d->value = v;
			},
			[&](const bool &v) {
				d->value = v;
			},
			[&](const std::string &v) {
				d->value = v;
			},
			[&](const f_wrapper &v) {
				d->value = v;
			},
			[&](const scope_t &v) {
				d->value = v;
			},
			[&](const std::shared_ptr<void> &v) {
				d->value = v;
			},
			[&](const param_t &v) {
				d->value = v;
			},
			[&](const sym_vec_t &v) {
				sym_vec_t nv;
				nv.resize(v.size());
				for (size_t i = 0; i < v.size(); i++)
					nv[i].set(&v[i], token, stack_trace);
				d->value = nv;
			},
			[&](const sym_map_t &v) {
				d->value = sym_map_t();
				for (auto &e : v) {
					if (e.second.d->type == Value::type_t::NIL)
						continue;
					auto newd = sym_t();
					newd.set(&e.second, token, stack_trace);
					std::get<sym_map_t>(d->value).insert({ e.first, newd });
				}
			},
			[](auto arg) {
				return;
			}
			}, b->d->value);
	}
}

const bool sym_t::equals(const sym_t *b, const token_t *token, trace_t &stack_trace) const
{
	if (d->type != b->d->type && d->type != Value::type_t::OBJECT)
		return false;
	switch (d->type) {
		case Value::type_t::NIL:
			return true;
		case Value::type_t::NUMBER:
			return std::get<number_t>(d->value) == std::get<number_t>(b->d->value);
		case Value::type_t::BOOLEAN_D:
			return std::get<bool>(d->value) == std::get<bool>(b->d->value);
		case Value::type_t::STRING:
			return std::get<std::string>(d->value) == std::get<std::string>(b->d->value);
		case Value::type_t::OBJECT:
		{
			auto o = std::get<scope_t>(d->value);
			if (o.hasValue(Rossa::HASH_EQUALS))
				return std::get<bool>(o.getVariable(Rossa::HASH_EQUALS, token, stack_trace).call({ *b }, token, stack_trace).d->value);
			return o == std::get<scope_t>(b->d->value);
		}
		case Value::type_t::ARRAY:
		{
			auto bv = std::get<sym_vec_t>(b->d->value);
			if (std::get<sym_vec_t>(d->value).size() != bv.size())
				return false;
			for (unsigned long i = 0; i < std::get<sym_vec_t>(d->value).size(); i++)
				if (!std::get<sym_vec_t>(d->value)[i].equals(&bv[i], token, stack_trace))
					return false;
			return true;
		}
		case Value::type_t::DICTIONARY:
			for (auto &e : std::get<sym_map_t>(d->value)) {
				if (!e.second.equals(&std::get<sym_map_t>(b->d->value)[e.first], token, stack_trace))
					return false;
			}
			return true;
		case Value::type_t::FUNCTION:
			return std::get<f_wrapper>(d->value).map == std::get<f_wrapper>(b->d->value).map && std::get<f_wrapper>(d->value).varg == std::get<f_wrapper>(b->d->value).varg;
		case Value::type_t::TYPE_NAME:
			return std::get<param_t>(d->value) == std::get<param_t>(b->d->value);
		default:
			return false;
	}
}

const bool sym_t::nequals(const sym_t *b, const token_t *token, trace_t &stack_trace) const
{
	switch (d->type) {
		case Value::type_t::OBJECT:
		{
			auto &o = std::get<scope_t>(d->value);
			if (o.hasValue(Rossa::HASH_NEQUALS))
				return std::get<bool>(o.getVariable(Rossa::HASH_NEQUALS, token, stack_trace).call({ *b }, token, stack_trace).d->value);
		}
		default:
			return !this->equals(b, token, stack_trace);
	}
}

const bool sym_t::pureEquals(const sym_t *b, const token_t *token, trace_t &stack_trace) const
{
	switch (d->type) {
		case Value::type_t::OBJECT:
			return std::get<scope_t>(d->value) == std::get<scope_t>(b->d->value);
		default:
			return this->equals(b, token, stack_trace);
	}
}

const bool sym_t::pureNEquals(const sym_t *b, const token_t *token, trace_t &stack_trace) const
{
	return !this->pureEquals(b, token, stack_trace);
}

const bool sym_t::operator==(const sym_t &b) const
{
	trace_t stack_trace;
	return this->equals(&b, NULL, stack_trace);
}

const bool sym_t::operator!=(const sym_t &b) const
{
	trace_t stack_trace;
	return this->nequals(&b, NULL, stack_trace);
}

const bool sym_t::operator<(const sym_t &b) const
{
	return this->toCodeString() < b.toCodeString();
}

void sym_t::shift() const
{
	if (d->type != Value::type_t::FUNCTION)
		return;
	for (auto &e : std::get<f_wrapper>(d->value).map)
		for (auto &f : e.second)
			f.second->shift();
	if (std::get<f_wrapper>(d->value).varg != nullptr)
		std::get<f_wrapper>(d->value).varg->shift();
}

const sym_t sym_t::clone() const
{
	return sym_t(*this);
}