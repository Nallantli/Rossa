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
#include <cmath>
#include <limits>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

typedef long double long_double_t;

#define SYM manager::Symbol
#define RUOTA_EXT_SYM(name, args) extern "C" BOOST_SYMBOL_EXPORT SYM name(std::vector<SYM> args)

	struct Symbol;
	class Value;

class Function;
class Scope;
class Instruction;
class Object;

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
	std::vector<std::string> paramNames;
	std::shared_ptr<Instruction> body;
	Scope *parent;

public:
	Function(Scope &, std::vector<std::string>, std::shared_ptr<Instruction>);
	SYM evaluate(std::vector<SYM>);
	SYM evaluate(std::vector<SYM>, SYM thisSymbol);
	const unsigned long getArgSize() const;
};

class Scope
{
private:
	std::string name;
	Scope *parent;
	std::map<std::string, SYM> values;

public:
	Scope();
	Scope(Scope &, const std::string &);
	Scope *getParent();
	SYM getVariable(const std::string &);
	SYM createVariable(const std::string &);
	SYM createVariable(const std::string &, const SYM &);
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
	SYM instantiate(std::vector<SYM> params) const;
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
	virtual SYM evaluate(Scope &) const = 0;
	virtual const std::string toString() const = 0;
	const I_TYPE getType();
	virtual ~Instruction();
};

	struct Value
	{
		D_TYPE type;
		long_double_t valueNumber;
		bool valueBool;
		std::vector<Symbol> valueVector;
		std::string valueString;
		std::map<unsigned long, std::shared_ptr<Function>> valueFunction;
		std::shared_ptr<Object> valueObject;

		Value() : type(NIL) {}
		Value(bool valueBool) : type(BOOLEAN_D), valueBool(valueBool) {}
		Value(std::shared_ptr<Object> &valueObject) : type(OBJECT), valueObject(valueObject) {}
		Value(std::shared_ptr<Function> &function) : type(FUNCTION), valueFunction({{function->getArgSize(), function}}) {}
		Value(long_double_t valueNumber) : type(NUMBER), valueNumber(valueNumber) {}
		Value(std::vector<Symbol> &valueVector) : type(VECTOR), valueVector(valueVector) {}
		Value(const std::string &valueString) : type(STRING), valueString(valueString) {}

		~Value() {}
	};

	struct Symbol
	{
		DID_TYPE type;
		std::shared_ptr<Value> d;

		Symbol() : type(ID_CASUAL), d(std::make_shared<Value>()) {}
		Symbol(std::shared_ptr<Value> d) : d(d), type(ID_CASUAL) {}

		/*inline void clearData(std::shared_ptr<Value> &dA)
		{
			dA->valueFunction.clear();
			dA->valueVector.clear();
		}*/

		static Symbol newValue()
		{
			return Symbol(std::make_shared<Value>());
		}

		static Symbol newValue(long_double_t d)
		{
			return Symbol(std::make_shared<Value>(d));
		}

		static Symbol newValue(bool b)
		{
			return Symbol(std::make_shared<Value>(b));
		}

		static Symbol newValue(std::vector<Symbol> &v)
		{
			return Symbol(std::make_shared<Value>(v));
		}

		static Symbol newValue(std::shared_ptr<Object> &valueObject)
		{
			return Symbol(std::make_shared<Value>(valueObject));
		}

		static Symbol newValue(std::shared_ptr<Function> &valueFunction)
		{
			return Symbol(std::make_shared<Value>(valueFunction));
		}

		static Symbol newValue(const std::string &valueString)
		{
			return Symbol(std::make_shared<Value>(valueString));
		}

		inline long_double_t getNumber()
		{
			return d->valueNumber;
		}

		inline std::vector<Symbol> getVector()
		{
			return d->valueVector;
		}

		inline std::string getString()
		{
			return d->valueString;
		}

		inline bool getBool()
		{
			return d->valueBool;
		}

		inline std::shared_ptr<Object> getObject()
		{
			return d->valueObject;
		}

		inline D_TYPE getType()
		{
			return d->type;
		}

		inline std::shared_ptr<Function> getFunction(Symbol &a, size_t argSize)
		{
			if (a.d->type != FUNCTION)
				throw std::runtime_error("Value is not of type Function: " + std::to_string(getType(a)));
			if (a.d->valueFunction.find(argSize) == a.d->valueFunction.end())
				throw std::runtime_error("Function does not exist");
			return a.d->valueFunction.at(argSize);
		}

		inline std::map<unsigned long, std::shared_ptr<Function>> getFunctions(Symbol &a)
		{
			return a.d->valueFunction;
		}
		inline Symbol &indexVector(Symbol &a, unsigned long index)
		{
			return a.d->valueVector.at(index);
		}
		inline size_t vectorSize(Symbol &a)
		{
			return a.d->valueVector.size();
		}

		inline std::string toString(const std::shared_ptr<Value> &d)
		{
			switch (d->type)
			{
			case NIL:
				return "nil";
			case NUMBER:
			{
				std::stringstream ss;
				ss.precision(std::numeric_limits<long_double_t>::max_digits10);
				ss << std::fixed << d->valueNumber;
				std::string ret = ss.str();
				if (ret.find('.') != std::string::npos)
				{
					while (ret.back() == '0')
						ret = ret.substr(0, ret.size() - 1);
					if (ret.back() == '.')
						ret = ret.substr(0, ret.size() - 1);
				}
				return ret;
			}
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
					ret += toString(d2.d);
					i++;
				}
				return ret + "]";
			}
			default:
				return "undefined";
			}
		}

		inline std::string toString(const Symbol &d)
		{
			return toString(d.d);
		}

		inline void clearData(Symbol &a)
		{
			a.d->valueFunction.clear();
			a.d->valueVector.clear();
			a.d->valueObject = nullptr;
		}

		inline Symbol call(Symbol &a, std::vector<Symbol> &params)
		{
			auto f = getFunction(a, params.size());

			return f->evaluate(params);
		}

		inline Symbol call(Symbol &a, std::vector<Symbol> &params, Symbol &b)
		{
			auto f = getFunction(a, params.size());

			return f->evaluate(params, b);
		}

		inline void addFunctions(Symbol &a, Symbol &b)
		{
			auto fs = getFunctions(b);
			for (auto &f : fs)
				a.d->valueFunction[f.second->getArgSize()] = f.second;
		}

		inline Symbol add(Symbol &a, Symbol &b)
		{
			if (getType(a) != getType(b))
				throw std::runtime_error("Cannot add values of different type");
			switch (getType(a))
			{
			case NUMBER:
			{
				auto valA = getNumber(a);
				auto valB = getNumber(b);
				return newValue(valA + valB);
			}
			case VECTOR:
			{
				auto valA = getVector(a);
				auto valB = getVector(b);
				valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
				return newValue(valA);
			}
			case STRING:
			{
				auto valA = getString(a);
				auto valB = getString(b);
				return newValue(valA + valB);
			}
			default:
				throw std::runtime_error("Operator `+` is undefined for value type");
			}
		}

		inline void setP(std::shared_ptr<Value> &dA, Symbol &b)
		{
			dA->type = getType(b);
			clearData(dA);
			switch (dA->type)
			{
			case NUMBER:
				dA->valueNumber = getNumber(b);
				break;
			case BOOLEAN_D:
				dA->valueBool = getBool(b);
				break;
			case STRING:
				dA->valueString = getString(b);
				break;
			case FUNCTION:
				dA->valueFunction = getFunctions(b);
				break;
			case OBJECT:
				dA->valueObject = getObject(b);
				break;
			case VECTOR:
			{
				auto v = getVector(b);
				for (unsigned long i = 0; i < v.size(); i++)
				{
					auto d = newValue(false);
					setP(d.d, v[i]);
					dA->valueVector.push_back(d);
				}
				break;
			}
			default:
				break;
			}
		}

		inline void set(Symbol &a, Symbol &b)
		{
			setP(a.d, b);
		}

		inline std::string getTypeString(Symbol &a)
		{
			switch (getType(a))
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
				return getObject(a)->getName();
			default:
				return "<unknown>";
			}
		}

		inline Symbol sub(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(valA - valB);
		}

		inline Symbol mul(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(valA * valB);
		}

		inline Symbol div(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(valA / valB);
		}

		inline Symbol mod(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue((long_double_t)((long long)valA % (long long)valB));
		}

		inline Symbol pow(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(std::pow(valA, valB));
		}

		inline Symbol less(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(valA < valB);
		}

		inline Symbol more(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(valA > valB);
		}

		inline Symbol eless(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(valA <= valB);
		}

		inline Symbol emore(Symbol &a, Symbol &b)
		{
			auto valA = getNumber(a);
			auto valB = getNumber(b);
			return newValue(valA >= valB);
		}

		inline bool equals(Symbol &a, Symbol &b)
		{
			if (getType(a) != getType(b))
				return false;
			switch (getType(a))
			{
			case NIL:
				return true;
			case NUMBER:
				return getNumber(a) == getNumber(b);
			case BOOLEAN_D:
				return getBool(a) == getBool(b);
			case STRING:
				return getString(a) == getString(b);
			case OBJECT:
				return getObject(a) == getObject(b);
			case VECTOR:
				if (vectorSize(a) != vectorSize(b))
					return false;
				for (unsigned long i = 0; i < vectorSize(a); i++)
				{
					if (!equals(indexVector(a, i), indexVector(b, i)))
						return false;
				}
			default:
				return false;
			}
		}

		inline bool nequals(Symbol &a, Symbol &b)
		{
			return !equals(a, b);
		}

		inline bool dand(Symbol &a, Symbol &b)
		{
			return getBool(a) && getBool(b);
		}

		inline bool dor(Symbol &a, Symbol &b)
		{
			return getBool(a) || getBool(b);
		}
	};

#endif