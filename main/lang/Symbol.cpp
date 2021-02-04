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

sym_t::sym_t(const aug_type_t &valueType)
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

const sym_t sym_t::TypeName(const aug_type_t &v)
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
	sym_t s;
	s.d->type = Value::type_t::ARRAY;
	s.d->valueVector.resize(size);
	return s;
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
	return d->valueNumber;
}

void *sym_t::getPointer(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::POINTER)
		throw rossa_error(_NOT_POINTER_, *token, stack_trace);
	return d->valuePointer.get();
}

const sym_map_t &sym_t::getDictionary(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::DICTIONARY)
		throw rossa_error(_NOT_DICTIONARY_, *token, stack_trace);
	auto iter = d->valueDictionary.begin();
	for (; iter != d->valueDictionary.end();) {
		if (iter->second.d->type == Value::type_t::NIL)
			iter = d->valueDictionary.erase(iter);
		else
			++iter;
	}
	return d->valueDictionary;
}

const sym_t &sym_t::indexVector(const size_t &i, const token_t *token, trace_t &stack_trace) const
{
	if (i >= d->valueVector.size())
		throw rossa_error(format::format(_INDEX_OUT_OF_BOUNDS_, { std::to_string(d->valueVector.size()), std::to_string(i) }), *token, stack_trace);
	return d->valueVector[i];
}

const sym_vec_t &sym_t::getVector(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::ARRAY)
		throw rossa_error(_NOT_VECTOR_, *token, stack_trace);
	return d->valueVector;
}

const std::string sym_t::getString(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::STRING)
		throw rossa_error(_NOT_STRING_, *token, stack_trace);
	return d->valueString;
}

const bool sym_t::getBool(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::BOOLEAN_D)
		throw rossa_error(_NOT_BOOLEAN_, *token, stack_trace);
	return d->valueBool;
}

scope_t *sym_t::getObject(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::OBJECT)
		throw rossa_error(_NOT_OBJECT_, *token, stack_trace);
	return &d->valueObject;
}

const Value::type_t sym_t::getValueType() const
{
	return d->type;
}

const aug_type_t sym_t::getAugValueType() const
{
	if (d->type == Value::type_t::OBJECT)
		return d->valueObject.getTypeVec();
	return { d->type };
}

const aug_type_t sym_t::getTypeName(const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::TYPE_NAME)
		throw rossa_error(_NOT_TYPE_, *token, stack_trace);
	return d->valueType;
}

const f_map_t &sym_t::getFunctionOverloads(const token_t *token, trace_t &stack_trace) const
{
	if (d->type == Value::type_t::FUNCTION)
		return this->d->valueFunction;
	if (d->type == Value::type_t::OBJECT)
		return d->valueObject.getVariable(Rossa::HASH_CALL, token, stack_trace).getFunctionOverloads(token, stack_trace);
	throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);
}

const bool sym_t::hasVarg(const token_t *token, trace_t &stack_trace) const
{
	if (d->type == Value::type_t::FUNCTION)
		return this->d->valueVARGFunction != nullptr;
	if (d->type == Value::type_t::OBJECT)
		return d->valueObject.getVariable(Rossa::HASH_CALL, token, stack_trace).hasVarg(token, stack_trace);
	throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);
}

const func_ptr_t sym_t::getFunction(const sym_vec_t &params, const token_t *token, trace_t &stack_trace) const
{
	if (d->type != Value::type_t::FUNCTION)
		throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);

	if (d->valueFunction.find(params.size()) == d->valueFunction.end()) {
		if (d->valueVARGFunction != nullptr)
			return d->valueVARGFunction;
		throw rossa_error(_FUNCTION_ARG_SIZE_FAILURE_, *token, stack_trace);
	}

	func_ptr_t f = nullptr;
	size_t cur_v = 0;
	for (auto &f2 : d->valueFunction[params.size()]) {
		size_t v = f2.first.validity(params, stack_trace);
		if (v > cur_v) {
			cur_v = v;
			f = f2.second;
			if (v == params.size() * 3)
				break;
		}
	}

	if (f == nullptr) {
		if (d->valueVARGFunction != nullptr)
			return d->valueVARGFunction;
		throw rossa_error(_FUNCTION_VALUE_NOT_EXIST_, *token, stack_trace);
	}

	return f;
}

const sym_t &sym_t::indexDict(const std::string &key) const
{
	return d->valueDictionary[key];
}

const bool sym_t::hasDictionaryKey(const std::string &key) const
{
	return d->valueDictionary.find(key) != d->valueDictionary.end();
}

const size_t sym_t::vectorSize() const
{
	return d->valueVector.size();
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
			return d->valueNumber.toString();
		case Value::type_t::STRING:
			return "\"" + d->valueString + "\"";
		case Value::type_t::FUNCTION:
		{
			std::string ret = "[";
			size_t i = 0;
			for (auto &e : d->valueFunction) {
				for (auto &t : e.second) {
					if (i++ > 0)
						ret += ", ";
					ret += "Function<" + t.first.toString() + ">";
				}
			}
			if (d->valueVARGFunction != nullptr) {
				if (i > 0)
					ret += ", ";
				ret += "Function<...>";
			}
			return ret + "]";
		}
		case Value::type_t::OBJECT:
			if (d->valueObject.hasValue(ROSSA_HASH("->String")))
				auto v = d->valueObject.getVariable(ROSSA_HASH("->String"), token, stack_trace).call({}, token, stack_trace).getString(token, stack_trace);
			return "<Object>";
		case Value::type_t::POINTER:
			return "<Pointer>";
		case Value::type_t::BOOLEAN_D:
			return d->valueBool ? KEYWORD_TRUE : KEYWORD_FALSE;
		case Value::type_t::ARRAY:
		{
			std::string ret = "[";
			unsigned int i = 0;
			for (auto &d2 : d->valueVector) {
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
			return "Type::" + getTypeString(d->valueType);
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
			return "sym_t()";
		case Value::type_t::NUMBER:
			return "sym_t(" + d->valueNumber.toCodeString() + ")";
		case Value::type_t::STRING:
		{
			std::string ret = "{";
			unsigned int i = 0;
			for (auto &c : d->valueString) {
				if (i > 0)
					ret += ", ";
				ret += std::to_string(c);
				i++;
			}
			return "sym_t(std::string(" + ret + "}))";
		}
		case Value::type_t::FUNCTION:
			return "<Function>";
		case Value::type_t::OBJECT:
			return "<Object::" + d->valueObject.getKey() + ">";
		case Value::type_t::POINTER:
			return "<Pointer>";
		case Value::type_t::BOOLEAN_D:
			return "sym_t(" + std::string(d->valueBool ? KEYWORD_TRUE : KEYWORD_FALSE) + ")";
		case Value::type_t::ARRAY:
		{
			std::string ret = "sym_t(sym_vec_t({";
			unsigned int i = 0;
			for (auto &d2 : d->valueVector) {
				if (i > 0)
					ret += ", ";
				ret += d2.toCodeString();
				i++;
			}
			return ret + "}))";
		}
		case Value::type_t::DICTIONARY:
		{
			std::string ret = "sym_t(sym_map_t({";
			unsigned int i = 0;
			for (auto &e : getDictionary(NULL, stack_trace)) {
				if (i > 0)
					ret += ", ";
				ret += "{\"" + e.first + "\", " + e.second.toCodeString() + "}";
				i++;
			}
			return ret + "}))";
		}
		case Value::type_t::TYPE_NAME:
			return "sym_t(static_cast<type_sll>(" + getTypeString(d->valueType) + "))";
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
	auto fs = b->d->valueFunction;
	for (auto &f : fs)
		for (auto &t : f.second)
			d->valueFunction[f.first][t.first] = t.second;
	if (b->d->valueVARGFunction != nullptr)
		d->valueVARGFunction = b->d->valueVARGFunction;
}


const func_ptr_t &sym_t::getVARGFunction(const token_t *token, trace_t &stack_trace) const
{
	if (d->type == Value::type_t::FUNCTION)
		return this->d->valueVARGFunction;
	if (d->type == Value::type_t::OBJECT)
		return d->valueObject.getVariable(Rossa::HASH_CALL, token, stack_trace).getVARGFunction(token, stack_trace);
	throw rossa_error(_NOT_FUNCTION_, *token, stack_trace);
}

void sym_t::nullify(const token_t *token, trace_t &stack_trace) const
{
	d->clearData();
	d->type = Value::type_t::NIL;
}

void sym_t::set(const sym_t *b, const token_t *token, const bool &isConst, trace_t &stack_trace) const
{
	if (b->d == d)
		return;
	if (d->type == Value::type_t::OBJECT && d->valueObject.hasValue(Rossa::HASH_SET)) {
		d->valueObject.getVariable(Rossa::HASH_SET, token, stack_trace).call({ *b }, token, stack_trace);
		return;
	}
	d->clearData();
	d->type = b->d->type;
	switch (d->type) {
		case Value::type_t::NUMBER:
			d->valueNumber = b->d->valueNumber;
			break;
		case Value::type_t::BOOLEAN_D:
			d->valueBool = b->d->valueBool;
			break;
		case Value::type_t::STRING:
			d->valueString = b->d->valueString;
			break;
		case Value::type_t::FUNCTION:
			d->valueFunction = b->d->valueFunction;
			d->valueVARGFunction = b->d->valueVARGFunction;
			break;
		case Value::type_t::OBJECT:
			d->valueObject = b->d->valueObject;
			break;
		case Value::type_t::POINTER:
			d->valuePointer = b->d->valuePointer;
			break;
		case Value::type_t::ARRAY:
		{
			auto &v = b->d->valueVector;
			if (isConst) {
				d->valueVector = v;
				break;
			}
			d->valueVector.resize(v.size());
			for (size_t i = 0; i < v.size(); i++)
				d->valueVector[i].set(&v[i], token, isConst, stack_trace);
			break;
		}
		case Value::type_t::DICTIONARY:
		{
			auto &v = b->d->valueDictionary;
			if (isConst) {
				d->valueDictionary = v;
				break;
			}
			for (auto &e : v) {
				if (e.second.d->type == Value::type_t::NIL)
					continue;
				auto newd = sym_t();
				newd.set(&e.second, token, isConst, stack_trace);
				d->valueDictionary[e.first] = newd;
			}
			break;
		}
		case Value::type_t::TYPE_NAME:
			d->valueType = b->d->valueType;
			break;
		default:
			break;
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
			return d->valueNumber == b->d->valueNumber;
		case Value::type_t::BOOLEAN_D:
			return d->valueBool == b->d->valueBool;
		case Value::type_t::STRING:
			return d->valueString == b->d->valueString;
		case Value::type_t::OBJECT:
		{
			auto o = d->valueObject;
			if (o.hasValue(Rossa::HASH_EQUALS))
				return o.getVariable(Rossa::HASH_EQUALS, token, stack_trace).call({ *b }, token, stack_trace).d->valueBool;
			return o == b->d->valueObject;
		}
		case Value::type_t::ARRAY:
		{
			auto bv = b->d->valueVector;
			if (d->valueVector.size() != bv.size())
				return false;
			for (unsigned long i = 0; i < d->valueVector.size(); i++)
				if (!d->valueVector[i].equals(&bv[i], token, stack_trace))
					return false;
			return true;
		}
		case Value::type_t::DICTIONARY:
			for (auto &e : d->valueDictionary) {
				if (!e.second.equals(&b->d->valueDictionary[e.first], token, stack_trace))
					return false;
			}
			return true;
		case Value::type_t::TYPE_NAME:
			return d->valueType == b->d->valueType;
		default:
			return false;
	}
}

const bool sym_t::nequals(const sym_t *b, const token_t *token, trace_t &stack_trace) const
{
	switch (d->type) {
		case Value::type_t::OBJECT:
		{
			auto &o = d->valueObject;
			if (o.hasValue(Rossa::HASH_NEQUALS))
				return o.getVariable(Rossa::HASH_NEQUALS, token, stack_trace).call({ *b }, token, stack_trace).d->valueBool;
		}
		default:
			return !this->equals(b, token, stack_trace);
	}
}

const bool sym_t::pureEquals(const sym_t *b, const token_t *token, trace_t &stack_trace) const
{
	switch (d->type) {
		case Value::type_t::OBJECT:
			return d->valueObject == b->d->valueObject;
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
	for (auto &e : d->valueFunction)
		for (auto &f : e.second)
			f.second->shift();
	if (d->valueVARGFunction != nullptr)
		d->valueVARGFunction->shift();
}