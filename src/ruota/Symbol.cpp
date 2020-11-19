#include "Ruota.h"

using namespace ruota;

Symbol::Symbol() : type(ID_CASUAL), d(new Value())
{}

Symbol::Symbol(const SymbolType &type) : type(type), d(new Value())
{}

Symbol::Symbol(const std::shared_ptr<void> &valuePointer) : type(ID_CASUAL), d(new Value(valuePointer))
{}

Symbol::Symbol(const type_sll &valueType) : type(ID_CASUAL), d(new Value(valueType))
{}

Symbol::Symbol(const RNumber &valueNumber) : type(ID_CASUAL), d(new Value(valueNumber))
{}

Symbol::Symbol(const bool &valueBool) : type(ID_CASUAL), d(new Value(valueBool))
{}

Symbol::Symbol(const std::vector<Symbol> &valueVector) : type(ID_CASUAL), d(new Value(valueVector))
{}

Symbol::Symbol(const std::shared_ptr<Scope> &valueObject) : type(ID_CASUAL), d(new Value(valueObject))
{}

Symbol::Symbol(const sig_t &ftype, const std::shared_ptr<const Function> &valueFunction) : type(ID_CASUAL), d(new Value(ftype, valueFunction))
{}

Symbol::Symbol(const std::string &valueString) : type(ID_CASUAL), d(new Value(valueString))
{}

Symbol::Symbol(const sym_map_t &valueDictionary) : type(ID_CASUAL), d(new Value(valueDictionary))
{}

Symbol::Symbol(const Symbol &s)
{
	this->d = s.d;
	this->type = s.type;
	this->d->references++;
}

Symbol::~Symbol()
{
	d->references--;
	if (d->references == 0)
		delete d;
}

void Symbol::operator=(const Symbol &b)
{
	this->d->references--;
	if (this->d->references == 0)
		delete d;

	this->d = b.d;
	this->type = b.type;
	this->d->references++;
}

const Symbol Symbol::allocate(const size_t &size)
{
	Symbol s;
	s.d->type = ARRAY;
	s.d->valueVector.resize(size);
	return s;
}

const SymbolType Symbol::getSymbolType() const
{
	return type;
}

void Symbol::setSymbolType(const SymbolType &type)
{
	this->type = type;
}

const RNumber &Symbol::getNumber(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != NUMBER)
		throw RTError(_NOT_NUMBER_, *token, stack_trace);
	return d->valueNumber;
}

void *Symbol::getPointer(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != POINTER)
		throw RTError(_NOT_POINTER_, *token, stack_trace);
	return d->valuePointer.get();
}

const sym_map_t &Symbol::getDictionary(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != DICTIONARY)
		throw RTError(_NOT_DICTIONARY_, *token, stack_trace);
	auto iter = d->valueDictionary.begin();
	for (; iter != d->valueDictionary.end();) {
		if (iter->second.d->type == NIL)
			iter = d->valueDictionary.erase(iter);
		else
			++iter;
	}
	return d->valueDictionary;
}

const Symbol &Symbol::indexVector(const size_t &i, const Token *token, std::vector<Function> &stack_trace) const
{
	if (i >= d->valueVector.size())
		throw RTError((boost::format(_INDEX_OUT_OF_BOUNDS_) % d->valueVector.size() % i).str(), *token, stack_trace);
	return d->valueVector[i];
}

const std::vector<Symbol> &Symbol::getVector(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != ARRAY)
		throw RTError(_NOT_VECTOR_, *token, stack_trace);
	return d->valueVector;
}

const std::string &Symbol::getString(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != STRING)
		throw RTError(_NOT_STRING_, *token, stack_trace);
	return d->valueString;
}

const bool Symbol::getBool(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != BOOLEAN_D)
		throw RTError(_NOT_BOOLEAN_, *token, stack_trace);
	return d->valueBool;
}

Scope *Symbol::getObject(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != OBJECT)
		throw RTError(_NOT_OBJECT_, *token, stack_trace);
	return d->valueObject.get();
}

const ValueType Symbol::getValueType() const
{
	return d->type;
}

const type_sll Symbol::getAugValueType() const
{
	if (d->type == OBJECT)
		return d->valueObject->getHashedKey();
	return d->type;
}

const type_sll Symbol::getTypeName(const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != TYPE_NAME)
		throw RTError(_NOT_TYPE_, *token, stack_trace);
	return d->valueType;
}

const std::shared_ptr<const Function> &Symbol::getFunction(const std::vector<Symbol> &params, const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != FUNCTION)
		throw RTError(_NOT_FUNCTION_, *token, stack_trace);

	if (d->valueFunction.find(params.size()) == d->valueFunction.end())
		throw RTError(_FUNCTION_ARG_SIZE_FAILURE_, *token, stack_trace);

	std::vector<type_sll> ftypes;
	for (auto &e : params)
		ftypes.push_back(e.getAugValueType());

	std::map<sig_t, std::shared_ptr<const Function>> foftype = d->valueFunction[params.size()];
	bool flag = false;
	sig_t key;
	size_t cur_v = 0;
	for (auto &f2 : foftype) {
		size_t v = sig::validity(f2.first, params, stack_trace);
		if (v > cur_v) {
			cur_v = v;
			key = f2.first;
			flag = true;
			if (v == ftypes.size() * 2)
				break;
		}
	}

	if (!flag)
		throw RTError(_FUNCTION_VALUE_NOT_EXIST_, *token, stack_trace);

	return foftype[key];
}

const Symbol &Symbol::indexDict(const std::string &key) const
{
	return d->valueDictionary[key];
}

const bool Symbol::hasDictionaryKey(const std::string &key) const
{
	return d->valueDictionary.find(key) != d->valueDictionary.end();
}

const size_t Symbol::vectorSize() const
{
	return d->valueVector.size();
}

const size_t Symbol::dictionarySize(const Token *token, std::vector<Function> &stack_trace) const
{
	return getDictionary(token, stack_trace).size();
}

const std::string Symbol::toString(const Token *token, std::vector<Function> &stack_trace) const
{
	switch (d->type) {
		case NIL:
			return KEYWORD_NIL;
		case NUMBER:
			return d->valueNumber.toString();
		case STRING:
			return "\"" + d->valueString + "\"";
		case FUNCTION:
		{
			std::string ret = "<Function:{";
			size_t i = 0;
			for (auto &e : d->valueFunction) {
				for (auto &t : e.second) {
					if (i++ > 0)
						ret += ", ";
					ret += sig::toString(t.first);
				}
			}
			return ret + "}>";
		}
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_TO_STRING))
				return o->getVariable(Ruota::HASH_TO_STRING, token, stack_trace).call({}, token, stack_trace).getString(token, stack_trace);
			return "<Object>";
		}
		case POINTER:
			return "<Pointer>";
		case BOOLEAN_D:
			return d->valueBool ? KEYWORD_TRUE : KEYWORD_FALSE;
		case ARRAY:
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
		case DICTIONARY:
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
		case TYPE_NAME:
		{
			switch (d->valueType) {
				case NIL:
					return "Type::Nil";
				case NUMBER:
					return "Type::Number";
				case STRING:
					return "Type::String";
				case FUNCTION:
					return "Type::Function";
				case OBJECT:
					return "Type::Object";
				case BOOLEAN_D:
					return "Type::Boolean";
				case ARRAY:
					return "Type::Array";
				case DICTIONARY:
					return "Type::Dictionary";
				case TYPE_NAME:
					return "Type::Type";
				case POINTER:
					return "Type::Pointer";
				default:
					return "Type::@" + RUOTA_DEHASH(d->valueType);
			}
		}
		default:
			return "undefined";
	}
}

const std::string Symbol::toCodeString() const
{
	if (type == ID_BREAK)
		return "<BREAK>";
	std::vector<Function> stack_trace;
	return sig::getTypeString(getAugValueType()) + "::" + toString(NULL, stack_trace);
}

const Symbol Symbol::call(const std::vector<Symbol> &params, const Token *token, std::vector<Function> &stack_trace) const
{
	return getFunction(params, token, stack_trace)->evaluate(params, token, stack_trace);
}

void Symbol::addFunctions(const Symbol *b, const Token *token) const
{
	auto fs = b->d->valueFunction;
	for (auto &f : fs)
		for (auto &t : f.second)
			d->valueFunction[f.first][t.first] = t.second;
}

void Symbol::set(const Symbol *b, const Token *token, const bool &isConst, std::vector<Function> &stack_trace) const
{
	if (b->d == d)
		return;
	if (d->type == OBJECT && d->valueObject != NULL && d->valueObject->hasValue(Ruota::HASH_SET)) {
		d->valueObject->getVariable(Ruota::HASH_SET, token, stack_trace).call({ *b }, token, stack_trace);
		return;
	}
	d->clearData();
	d->type = b->d->type;
	switch (d->type) {
		case NUMBER:
			d->valueNumber = b->d->valueNumber;
			break;
		case BOOLEAN_D:
			d->valueBool = b->d->valueBool;
			break;
		case STRING:
			d->valueString = b->d->valueString;
			break;
		case FUNCTION:
			d->valueFunction = b->d->valueFunction;
			break;
		case OBJECT:
			d->valueObject = b->d->valueObject;
			break;
		case POINTER:
			d->valuePointer = b->d->valuePointer;
			break;
		case ARRAY:
		{
			auto v = b->d->valueVector;
			if (isConst) {
				d->valueVector = v;
				break;
			}
			d->valueVector.resize(v.size());
			for (size_t i = 0; i < v.size(); i++)
				d->valueVector[i].set(&v[i], token, isConst, stack_trace);
			break;
		}
		case DICTIONARY:
		{
			auto v = b->d->valueDictionary;
			if (isConst) {
				d->valueDictionary = v;
				break;
			}
			for (auto &e : v) {
				if (e.second.d->type == NIL)
					continue;
				auto newd = Symbol();
				newd.set(&e.second, token, isConst, stack_trace);
				d->valueDictionary[e.first] = newd;
			}
			break;
		}
		case TYPE_NAME:
			d->valueType = b->d->valueType;
			break;
		default:
			break;
	}
}

const bool Symbol::equals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
{
	if (d->type != b->d->type && d->type != OBJECT)
		return false;
	switch (d->type) {
		case NIL:
			return true;
		case NUMBER:
			return d->valueNumber == b->d->valueNumber;
		case BOOLEAN_D:
			return d->valueBool == b->d->valueBool;
		case STRING:
			return d->valueString == b->d->valueString;
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_EQUALS))
				return o->getVariable(Ruota::HASH_EQUALS, token, stack_trace).call({ *b }, token, stack_trace).d->valueBool;
			return o == b->d->valueObject;
		}
		case ARRAY:
		{
			auto bv = b->d->valueVector;
			if (d->valueVector.size() != bv.size())
				return false;
			for (unsigned long i = 0; i < d->valueVector.size(); i++)
				if (!d->valueVector[i].equals(&bv[i], token, stack_trace))
					return false;
			return true;
		}
		case DICTIONARY:
			for (auto &e : d->valueDictionary) {
				if (!e.second.equals(&b->d->valueDictionary[e.first], token, stack_trace))
					return false;
			}
			return true;
		case TYPE_NAME:
			return d->valueType == b->d->valueType;
		default:
			return false;
	}
}

const bool Symbol::nequals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
{
	switch (d->type) {
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_NEQUALS))
				return o->getVariable(Ruota::HASH_NEQUALS, token, stack_trace).call({ *b }, token, stack_trace).d->valueBool;
		}
		default:
			return !this->equals(b, token, stack_trace);
	}
}

const bool Symbol::pureEquals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
{
	switch (d->type) {
		case OBJECT:
			return d->valueObject == b->d->valueObject;
		default:
			return this->equals(b, token, stack_trace);
	}
}

const bool Symbol::pureNEquals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
{
	return !this->pureEquals(b, token, stack_trace);
}

const bool Symbol::operator==(const Symbol &b) const
{
	std::vector<Function> stack_trace;
	return this->equals(&b, NULL, stack_trace);
}

const bool Symbol::operator!=(const Symbol &b) const
{
	std::vector<Function> stack_trace;
	return this->nequals(&b, NULL, stack_trace);
}

const bool Symbol::operator<(const Symbol &b) const
{
	return this->toCodeString() < b.toCodeString();
}