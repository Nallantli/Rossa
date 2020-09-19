#ifndef RUOTATYPES_H
#define RUOTATYPES_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#define SYM							manager::Symbol
#define RUOTA_EXT_SYM(name, args)	extern "C" BOOST_SYMBOL_EXPORT SYM name(std::vector<SYM> args)

namespace manager
{
	struct Symbol;
	class Value;
}; // namespace manager

class Function;
class Scope;
class Instruction;

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

class Function
{
private:
	std::vector<std::string> paramNames;
	std::shared_ptr<Instruction> body;
	Scope *parent;

public:
	Function(Scope &, std::vector<std::string>, std::shared_ptr<Instruction>);
	SYM evaluate(std::vector<SYM>);
	const unsigned long getArgSize() const;
};

class Scope
{
private:
	Scope *parent;
	std::map<std::string, SYM> values;

public:
	Scope();
	Scope(Scope &);
	SYM getVariable(const std::string &);
	SYM createVariable(const std::string &);
	SYM createVariable(const std::string &, const SYM &);

	~Scope();
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

namespace manager
{
	struct Value
	{
		D_TYPE type;
		double valueNumber;
		bool valueBool;
		std::vector<Symbol> valueVector;
		std::string valueString;
		std::map<unsigned long, std::shared_ptr<Function>> valueFunction;

		Value() : type(NIL) {}
		Value(bool valueBool) : type(BOOLEAN_D), valueBool(valueBool) {}
		Value(std::shared_ptr<Function> function) : type(FUNCTION), valueFunction({{function->getArgSize(), function}}) {}
		Value(double valueNumber) : type(NUMBER), valueNumber(valueNumber) {}
		Value(const std::vector<Symbol> &valueVector) : type(VECTOR), valueVector(valueVector) {}
		Value(const std::string &valueString) : type(STRING), valueString(valueString) {}

		~Value() {}
	};

	struct Symbol
	{
		DID_TYPE type;
		std::shared_ptr<Value> d;

		Symbol() : type(ID_CASUAL), d(std::make_shared<Value>()) {}
		Symbol(std::shared_ptr<Value> d) : d(std::move(d)), type(ID_CASUAL) {}
	};

	static void clearData(std::shared_ptr<Value> dA)
	{
		dA->valueFunction.clear();
		dA->valueVector.clear();
	}

	static Symbol newValue()
	{
		return Symbol(std::make_shared<Value>());
	}

	static Symbol newValue(double d)
	{
		return Symbol(std::make_shared<Value>(d));
	}

	static Symbol newValue(bool b)
	{
		return Symbol(std::make_shared<Value>(b));
	}

	static Symbol newValue(const std::vector<Symbol> &v)
	{
		return Symbol(std::make_shared<Value>(v));
	}

	static Symbol newValue(std::shared_ptr<Function> valueFunction)
	{
		return Symbol(std::make_shared<Value>(valueFunction));
	}

	static Symbol newValue(const std::string &valueString)
	{
		return Symbol(std::make_shared<Value>(valueString));
	}

	static double getNumber(const Symbol &a)
	{
		return a.d->valueNumber;
	}

	static const std::vector<Symbol> &getVector(const Symbol &a)
	{
		return a.d->valueVector;
	}

	static const std::string &getString(const Symbol &a)
	{
		return a.d->valueString;
	}

	static const bool getBool(const Symbol &a)
	{
		return a.d->valueBool;
	}

	static const D_TYPE getType(const Symbol &a)
	{
		return a.d->type;
	}
	static std::map<unsigned long, std::shared_ptr<Function>> getFunctions(const Symbol &a)
	{
		return a.d->valueFunction;
	}
	static std::shared_ptr<Function> getFunction(const Symbol &a, unsigned long argSize)
	{
		if (a.d->type != FUNCTION)
			throw std::runtime_error("Value is not of type Function");
		if (a.d->valueFunction.find(argSize) == a.d->valueFunction.end())
			throw std::runtime_error("Function does not exist");
		return a.d->valueFunction.at(argSize);
	}
	static const Symbol &indexVector(const Symbol &a, unsigned long index)
	{
		return a.d->valueVector.at(index);
	}
	static const unsigned long vectorSize(const Symbol &a)
	{
		return a.d->valueVector.size();
	}

	static const std::string toString(std::shared_ptr<Value> d)
	{
		switch (d->type)
		{
		case NIL:
			return "nil";
		case NUMBER:
			return std::to_string(getNumber(d));
		case STRING:
			return getString(d);
		case FUNCTION:
			return "LAMBDA";
		case BOOLEAN_D:
			return getBool(d) ? "true" : "false";
		case VECTOR:
		{
			std::string ret = "[";
			unsigned int i = 0;
			for (auto &d2 : getVector(d))
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

	static const std::string toString(const Symbol &d)
	{
		return toString(d.d);
	}

	static void clearData(const Symbol &a)
	{
		a.d->valueFunction.clear();
		a.d->valueVector.clear();
	}

	static Symbol call(const Symbol &a, const std::vector<Symbol> &params)
	{
		auto f = getFunction(a.d, params.size());

		return f->evaluate(params);
	}

	static void addFunctions(const Symbol &a, const Symbol &b)
	{
		auto fs = getFunctions(b.d);
		for (auto &f : fs)
			a.d->valueFunction[f.second->getArgSize()] = f.second;
	}

	static Symbol add(const Symbol &a, const Symbol &b)
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
		default:
			throw std::runtime_error("Operator `+` is undefined for value type");
		}
	}

	static void set(std::shared_ptr<Value> dA, std::shared_ptr<Value> dB)
	{
		dA->type = dB->type;
		clearData(dA);
		switch (dA->type)
		{
		case NUMBER:
			dA->valueNumber = getNumber(dB);
			break;
		case BOOLEAN_D:
			dA->valueBool = getBool(dB);
			break;
		case STRING:
			dA->valueString = getString(dB);
			break;
		case FUNCTION:
			dA->valueFunction = getFunctions(dB);
			break;
		case VECTOR:
			for (int i = 0; i < dB->valueVector.size(); i++)
			{
				auto d = newValue(false);
				set(d.d, dB->valueVector[i].d);
				dA->valueVector.push_back(d);
			}
			break;
		default:
			break;
		}
	}

	static void set(Symbol a, const Symbol &b)
	{
		set(a.d, b.d);
	}

	static Symbol sub(const Symbol &a, const Symbol &b)
	{
		auto valA = getNumber(a);
		auto valB = getNumber(b);
		return newValue(valA - valB);
	}

	static Symbol mul(const Symbol &a, const Symbol &b)
	{
		auto valA = getNumber(a);
		auto valB = getNumber(b);
		return newValue(valA * valB);
	}

	static Symbol div(const Symbol &a, const Symbol &b)
	{
		auto valA = getNumber(a);
		auto valB = getNumber(b);
		return newValue(valA / valB);
	}

	static Symbol mod(const Symbol &a, const Symbol &b)
	{
		int valA = getNumber(a);
		int valB = getNumber(b);
		return newValue((double)(valA % valB));
	}

	static Symbol less(const Symbol &a, const Symbol &b)
	{
		int valA = getNumber(a);
		int valB = getNumber(b);
		return newValue(valA < valB);
	}

	static Symbol more(const Symbol &a, const Symbol &b)
	{
		int valA = getNumber(a);
		int valB = getNumber(b);
		return newValue(valA > valB);
	}

	static Symbol eless(const Symbol &a, const Symbol &b)
	{
		int valA = getNumber(a);
		int valB = getNumber(b);
		return newValue(valA <= valB);
	}

	static Symbol emore(const Symbol &a, const Symbol &b)
	{
		int valA = getNumber(a);
		int valB = getNumber(b);
		return newValue(valA >= valB);
	}

	static bool equals(const Symbol &a, const Symbol &b)
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
		case VECTOR:
			if (vectorSize(a) != vectorSize(b))
				return false;
			for (int i = 0; i < vectorSize(a); i++)
			{
				if (!equals(indexVector(a, i), indexVector(b, i)))
					return false;
			}
		default:
			return false;
		}
	}

	static bool nequals(const Symbol &a, const Symbol &b)
	{
		return !equals(a, b);
	}

	static bool dand(const Symbol &a, const Symbol &b)
	{
		return getBool(a) && getBool(b);
	}

	static bool dor(const Symbol &a, const Symbol &b)
	{
		return getBool(a) || getBool(b);
	}
}; // namespace manager

#endif