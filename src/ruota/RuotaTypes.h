#ifndef RUOTATYPES_H
#define RUOTATYPES_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <deque>
#include <stdexcept>
#include <fstream>
#include <memory>
#include <map>
#include <sstream>
#include <typeinfo>
#include <cmath>
#include <limits>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

typedef long double long_double_t;
typedef signed long long long_int_t;

#define RUOTA_EXT_SYM(name, args) extern "C" BOOST_SYMBOL_EXPORT Symbol name(std::vector<Symbol> args)

class Symbol;
class Value;

class Function;
class Scope;
class Instruction;
class Object;

enum LEX_TOKEN_TYPE
{
	TOK_EOF = -1,

	TOK_IDF = -2,
	TOK_NUM = -3,

	TOK_OPR = -4,
	TOK_IF = -5,
	TOK_ELSE = -6,
	TOK_ELSEIF = -7,
	TOK_IN = -8,
	TOK_VAR = -9,
	TOK_DO = -10,
	TOK_WHILE = -11,
	TOK_THEN = -12,
	TOK_FOR = -13,
	TOK_DEF = -14,
	TOK_FALSE = -15,
	TOK_TRUE = -16,
	TOK_RETURN = -17,
	TOK_STR_LIT = -18,
	TOK_NIL = -19,
	TOK_EXTERN = -20,

	TOK_NUMBER = -21,
	TOK_STRING = -22,
	TOK_VECTOR = -23,
	TOK_BOOLEAN = -24,
	TOK_EXTERN_CALL = -25,

	TOK_LENGTH = -26,
	TOK_SIZE = -27,
	TOK_CLASS = -28,
	TOK_STRUCT = -29,
	TOK_STATIC = -30,
	TOK_NEW = -31,
	TOK_TYPE = -32,
	TOK_DICTIONARY = -33,
	TOK_OBJECT = -34,
	TOK_FUNCTION = -35,
	TOK_CAST = -36,

	TOK_LOAD = -37,
	TOK_ALLOC = -38,
	TOK_UNTIL = -39,
	TOK_REF = -40,
	TOK_FINAL = -41,
	TOK_DEF_TYPE = -42
};

enum DID_TYPE
{
	ID_RETURN,
	ID_BREAK,
	ID_CASUAL
};

enum D_TYPE
{
	NIL,
	NUMBER,
	BOOLEAN_D,
	STRING,
	VECTOR,
	FUNCTION,
	DICTIONARY,
	OBJECT
};

enum I_TYPE
{
	CONTAINER,
	VARIABLE,
	SEQUENCE,
	DECLARE,
	INDEX,
	INNER,
	IFELSE,
	WHILE,
	DEFINE,
	RETURN,
	EXTERN,
	LENGTH,
	SIZE_I,
	CLASS_I,
	NEW_I,
	TYPE_I,
	CAST_TO_I,
	POW_I,
	ALLOC_I,
	UNTIL_I,
	SCOPE_I,
	MAP_I,
	FOR,
	SET,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	LESS,
	MORE,
	ELESS,
	EMORE,
	EQUALS,
	NEQUALS,
	AND,
	OR
};

enum OBJECT_TYPE
{
	STRUCT_O,
	STATIC_O,
	VIRTUAL_O
};

class Function
{
private:
	std::vector<std::pair<LEX_TOKEN_TYPE, std::string>> params;
	std::shared_ptr<Instruction> body;
	Scope *parent;

public:
	Function(Scope &, std::vector<std::pair<LEX_TOKEN_TYPE, std::string>>, std::shared_ptr<Instruction>);
	Symbol evaluate(std::vector<Symbol>);
	Symbol evaluate(std::vector<Symbol>, Symbol *thisSymbol);
	const unsigned long getArgSize() const;
};

class Scope
{
private:
	std::string name;
	Scope *parent;
	std::map<std::string, Symbol> values;

public:
	Scope();
	Scope(Scope &, const std::string &);
	Scope *getParent();
	Symbol getVariable(const std::string &);
	Symbol createVariable(const std::string &);
	Symbol createVariable(const std::string &, Symbol);
	const std::string &getName() const;
	bool hasValue(const std::string &) const;

	~Scope();
};

class Object
{
private:
	std::string key;
	OBJECT_TYPE type;
	std::shared_ptr<Scope> internal;
	std::shared_ptr<Instruction> body;

public:
	Object(Scope &, OBJECT_TYPE, std::shared_ptr<Instruction>, const std::string &);
	std::shared_ptr<Scope> getScope();
	Symbol instantiate(std::vector<Symbol> params) const;
	OBJECT_TYPE getType() const;
	const std::string &getName() const;
	bool hasValue(const std::string &) const;
};

class Instruction
{
protected:
	I_TYPE type;

public:
	Instruction(I_TYPE);
	virtual Symbol evaluate(Scope &) const = 0;
	virtual const std::string toString() const = 0;
	const I_TYPE getType();
	virtual ~Instruction();
};

enum NumberType
{
	DOUBLE_NUM,
	LONG_NUM
};

struct Number
{
	NumberType type;
	long_double_t valueDouble;
	long_int_t valueLong;

	inline void validate()
	{
		if (valueDouble == static_cast<long_int_t>(valueDouble))
		{
			valueLong = static_cast<long_int_t>(valueDouble);
			type = LONG_NUM;
		}
	}

	Number() : valueLong(0), valueDouble(0), type(LONG_NUM) {}
	Number(const Number &n)
	{
		this->type = n.type;
		this->valueDouble = n.valueDouble;
		this->valueLong = n.valueLong;
	}
	Number(long_double_t valueDouble) : valueDouble(valueDouble), valueLong(0), type(DOUBLE_NUM) { validate(); }
	Number(long_int_t valueLong) : valueLong(valueLong), valueDouble(0), type(LONG_NUM) {}

	inline const Number operator+(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(valueDouble + n.valueDouble);
			case LONG_NUM:
				return Number(valueDouble + static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(static_cast<long_double_t>(valueLong) + n.valueDouble);
			case LONG_NUM:
				return Number(valueLong + n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline void operator+=(long_int_t v)
	{
		switch (type)
		{
		case DOUBLE_NUM:
			valueDouble += static_cast<long_double_t>(v);
		case LONG_NUM:
			valueLong += v;
		default:
			break;
		}
	}

	inline const Number operator-(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(valueDouble - n.valueDouble);
			case LONG_NUM:
				return Number(valueDouble - static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(static_cast<long_double_t>(valueLong) - n.valueDouble);
			case LONG_NUM:
				return Number(valueLong - n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const Number operator*(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(valueDouble * n.valueDouble);
			case LONG_NUM:
				return Number(valueDouble * static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(static_cast<long_double_t>(valueLong) * n.valueDouble);
			case LONG_NUM:
				return Number(valueLong * n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const Number operator/(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(valueDouble / n.valueDouble);
			case LONG_NUM:
				return Number(valueDouble / static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(static_cast<long_double_t>(valueLong) / n.valueDouble);
			case LONG_NUM:
				return Number(valueLong / n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const Number operator^(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(static_cast<long_double_t>(std::pow(valueDouble, n.valueDouble)));
			case LONG_NUM:
				return Number(static_cast<long_double_t>(std::pow(valueDouble, static_cast<long_double_t>(n.valueLong))));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(static_cast<long_double_t>(std::pow(static_cast<long_double_t>(valueLong), n.valueDouble)));
			case LONG_NUM:
				return Number(static_cast<long_double_t>(std::pow(valueLong, n.valueLong)));
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const Number operator%(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(static_cast<long_int_t>(valueDouble) % static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return Number(static_cast<long_int_t>(valueDouble) % n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return Number(valueLong % static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return Number(valueLong % n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline bool operator==(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueDouble == n.valueDouble;
			case LONG_NUM:
				return valueDouble == n.valueLong;
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueLong == n.valueDouble;
			case LONG_NUM:
				return valueLong == n.valueLong;
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline bool operator!=(const Number &n) const
	{
		return !(*this == n);
	}

	inline bool operator<(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueDouble < n.valueDouble;
			case LONG_NUM:
				return valueDouble < n.valueLong;
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueLong < n.valueDouble;
			case LONG_NUM:
				return valueLong < n.valueLong;
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline bool operator>(const Number &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueDouble > n.valueDouble;
			case LONG_NUM:
				return valueDouble > n.valueLong;
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return valueLong > n.valueDouble;
			case LONG_NUM:
				return valueLong > n.valueLong;
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const bool operator<=(const Number &n) const
	{
		return !(*this > n);
	}

	inline const bool operator>=(const Number &n) const
	{
		return !(*this < n);
	}

	inline const std::string toString() const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			return std::to_string(valueDouble);
		case LONG_NUM:
			return std::to_string(valueLong);
		default:
			throw std::runtime_error("Invalid conversion - not a user error");
		}
	}

	inline long_double_t getDouble() const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			return valueDouble;
		case LONG_NUM:
			return static_cast<long_double_t>(valueLong);
		default:
			throw std::runtime_error("Invalid conversion - not a user error");
		}
	}

	inline long_int_t getLong() const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			return static_cast<long_int_t>(valueDouble);
		case LONG_NUM:
			return valueLong;
		default:
			throw std::runtime_error("Invalid conversion - not a user error");
		}
	}
};

class Value
{
	friend class Symbol;

private:
	D_TYPE type;
	Number valueNumber;
	bool valueBool;
	std::vector<Symbol> valueVector;
	std::string valueString;
	std::map<D_TYPE, std::map<size_t, std::shared_ptr<Function>>> valueFunction;
	std::map<std::string, Symbol> valueDictionary;
	std::shared_ptr<Object> valueObject;

public:
	Value() : type(NIL) {}
	Value(bool valueBool) : type(BOOLEAN_D), valueBool(valueBool) {}
	Value(std::shared_ptr<Object> &valueObject) : type(OBJECT), valueObject(valueObject) {}
	Value(D_TYPE ftype, std::shared_ptr<Function> &function) : type(FUNCTION), valueFunction({{ftype, {{function->getArgSize(), function}}}}) {}
	Value(Number valueNumber) : type(NUMBER), valueNumber(valueNumber) {}
	Value(std::vector<Symbol> valueVector) : type(VECTOR), valueVector(valueVector) {}
	Value(std::map<std::string, Symbol> valueDictionary) : type(DICTIONARY), valueDictionary(valueDictionary) {}
	Value(const std::string &valueString) : type(STRING), valueString(valueString) {}
};

class Symbol
{
private:
	DID_TYPE type;
	bool isMutable;
	std::shared_ptr<Value> d;

public:
	Symbol() : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>()) {}

	Symbol(Number valueNumber) : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>(valueNumber)) {}

	Symbol(bool valueBool) : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>(valueBool)) {}

	Symbol(std::vector<Symbol> valueVector) : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>(valueVector)) {}

	Symbol(std::shared_ptr<Object> &valueObject) : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>(valueObject)) {}

	Symbol(D_TYPE ftype, std::shared_ptr<Function> &valueFunction) : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>(ftype, valueFunction)) {}

	Symbol(const std::string &valueString) : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>(valueString)) {}

	Symbol(const std::map<std::string, Symbol> valueDictionary) : type(ID_CASUAL), isMutable(true), d(std::make_shared<Value>(valueDictionary)) {}

	inline void setMutable(bool isMutable)
	{
		this->isMutable = isMutable;
	}

	inline DID_TYPE getSymbolType() const
	{
		return type;
	}

	inline void setSymbolType(DID_TYPE type)
	{
		this->type = type;
	}

	inline Number getNumber() const
	{
		return d->valueNumber;
	}

	inline std::map<std::string, Symbol> getDictionary() const
	{
		auto iter = d->valueDictionary.begin();
		for (; iter != d->valueDictionary.end();)
		{
			if (iter->second.getType() == NIL)
				iter = d->valueDictionary.erase(iter);
			else
				++iter;
		}
		return d->valueDictionary;
	}

	inline std::vector<Symbol> getVector() const
	{
		return d->valueVector;
	}

	inline std::string getString() const
	{
		return d->valueString;
	}

	inline bool getBool() const
	{
		return d->valueBool;
	}

	inline std::shared_ptr<Object> getObject() const
	{
		return d->valueObject;
	}

	inline D_TYPE getType() const
	{
		return d->type;
	}

	inline std::shared_ptr<Function> getFunction(D_TYPE ftype, size_t argSize)
	{
		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type Function: " + std::to_string(d->type));

		std::map<size_t, std::shared_ptr<Function>> foftype;

		if (ftype != NIL && d->valueFunction.find(ftype) != d->valueFunction.end())
			foftype = d->valueFunction[ftype];
		else if (d->valueFunction.find(NIL) != d->valueFunction.end())
			foftype = d->valueFunction[NIL];
		else
			throw std::runtime_error("Function does not exist");
		if (foftype.find(argSize) == foftype.end())
			throw std::runtime_error("Function does not exist");

		return foftype[argSize];
	}

	inline std::map<D_TYPE, std::map<size_t, std::shared_ptr<Function>>> getFunctions() const
	{
		return d->valueFunction;
	}

	inline Symbol indexVector(size_t index) const
	{
		return d->valueVector[index];
	}

	inline Symbol indexDictionary(const std::string &key) const
	{
		return d->valueDictionary[key];
	}

	inline bool hasDictionaryKey(const std::string &key) const
	{
		return d->valueDictionary.find(key) != d->valueDictionary.end();
	}

	inline size_t vectorSize() const
	{
		return d->valueVector.size();
	}

	inline size_t dictionarySize() const
	{
		return getDictionary().size();
	}

	inline std::string toString() const
	{
		switch (d->type)
		{
		case NIL:
			return "nil";
		case NUMBER:
			return d->valueNumber.toString();
		case STRING:
			return d->valueString;
		case FUNCTION:
			return "<Function>";
		case OBJECT:
			return "<Object>";
		case BOOLEAN_D:
			return d->valueBool ? "true" : "false";
		case VECTOR:
		{
			std::string ret = "[";
			unsigned int i = 0;
			for (auto &d2 : d->valueVector)
			{
				if (i > 0)
					ret += ", ";
				ret += d2.toString();
				i++;
			}
			return ret + "]";
		}
		case DICTIONARY:
		{
			std::string ret = "{";
			unsigned int i = 0;
			for (auto &e : getDictionary())
			{
				if (i > 0)
					ret += ", ";
				ret += "\"" + e.first + "\" : " + e.second.toString();
				i++;
			}
			return ret + "}";
		}
		default:
			return "undefined";
		}
	}

	inline void clearData()
	{
		d->valueFunction.clear();
		d->valueVector.clear();
		d->valueDictionary.clear();
		d->valueObject = nullptr;
	}

	inline Symbol call(D_TYPE ftype, std::vector<Symbol> params)
	{
		auto f = getFunction(ftype, params.size());

		return f->evaluate(params);
	}

	inline Symbol call(D_TYPE ftype, std::vector<Symbol> params, Symbol b)
	{
		auto f = getFunction(ftype, params.size());

		return f->evaluate(params, &b);
	}

	inline void addFunctions(const Symbol &b)
	{
		auto fs = b.getFunctions();
		for (auto &f : fs)
			for (auto &t : f.second)
				d->valueFunction[f.first][t.first] = t.second;
	}

	inline Symbol operator+(const Symbol &b) const
	{
		if (d->type != b.getType())
			throw std::runtime_error("Cannot add values of different type");
		switch (d->type)
		{
		case NUMBER:
		{
			auto valB = b.getNumber();
			return Symbol(d->valueNumber + valB);
		}
		case VECTOR:
		{
			auto valA = d->valueVector;
			auto valB = b.getVector();
			valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
			return Symbol(valA);
		}
		case STRING:
		{
			auto valB = b.getString();
			return Symbol(d->valueString + valB);
		}
		default:
			throw std::runtime_error("Operator `+` is undefined for value type");
		}
	}

	inline void set(const Symbol &b)
	{
		if (!isMutable)
			throw std::runtime_error("Cannot change the value of a variable declared as `final`");
		d->type = b.getType();
		clearData();
		switch (d->type)
		{
		case NUMBER:
			d->valueNumber = b.getNumber();
			break;
		case BOOLEAN_D:
			d->valueBool = b.getBool();
			break;
		case STRING:
			d->valueString = b.getString();
			break;
		case FUNCTION:
			d->valueFunction = b.getFunctions();
			break;
		case OBJECT:
			d->valueObject = b.getObject();
			break;
		case VECTOR:
		{
			auto v = b.getVector();
			for (unsigned long i = 0; i < v.size(); i++)
			{
				auto newd = Symbol();
				newd.set(v[i]);
				d->valueVector.push_back(newd);
			}
			break;
		}
		case DICTIONARY:
		{
			auto v = b.getDictionary();
			for (auto &e : v)
			{

				if (e.second.getType() == NIL)
					continue;
				auto newd = Symbol();
				newd.set(e.second);
				d->valueDictionary[e.first] = newd;
			}
			break;
		}
		default:
			break;
		}
	}

	inline std::string getTypeString() const
	{
		switch (d->type)
		{
		case NIL:
			return "nil";
		case NUMBER:
			return "Number";
		case STRING:
			return "String";
		case BOOLEAN_D:
			return "Boolean";
		case VECTOR:
			return "Vector";
		case DICTIONARY:
			return "Dictionary";
		case FUNCTION:
			return "Function";
		case OBJECT:
			return d->valueObject->getName();
		default:
			return "<unknown>";
		}
	}

	inline Symbol operator-(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber - valB);
	}

	inline Symbol operator*(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber * valB);
	}

	inline Symbol operator/(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber / valB);
	}

	inline Symbol operator%(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber % valB);
	}

	inline Symbol pow(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber ^ valB);
	}

	inline Symbol operator<(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber < valB);
	}

	inline Symbol operator>(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber > valB);
	}

	inline Symbol operator<=(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber <= valB);
	}

	inline Symbol operator>=(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(d->valueNumber >= valB);
	}

	inline bool operator==(const Symbol &b) const
	{
		if (d->type != b.getType())
			return false;
		switch (d->type)
		{
		case NIL:
			return true;
		case NUMBER:
			return d->valueNumber == b.getNumber();
		case BOOLEAN_D:
			return d->valueBool == b.getBool();
		case STRING:
			return d->valueString == b.getString();
		case OBJECT:
			return d->valueObject == b.getObject();
		case VECTOR:
			if (d->valueVector.size() != b.vectorSize())
				return false;
			for (unsigned long i = 0; i < d->valueVector.size(); i++)
			{
				if (d->valueVector[i] != b.indexVector(i))
					return false;
			}
			return true;
		case DICTIONARY:
			for (auto &e : d->valueDictionary)
			{
				if (e.second != b.indexDictionary(e.first))
					return false;
			}
			return true;
		default:
			return false;
		}
	}

	inline bool operator!=(const Symbol &b) const
	{
		return !(*this == b);
	}

	inline bool operator&&(const Symbol &b) const
	{
		return d->valueBool && b.getBool();
	}

	inline bool operator||(const Symbol &b) const
	{
		return d->valueBool || b.getBool();
	}
};

#endif