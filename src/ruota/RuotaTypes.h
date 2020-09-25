#ifndef RUOTATYPES_H
#define RUOTATYPES_H

#define _USE_CONV_NUM_
#define _USE_RAW_PTR_

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
#include <boost/config.hpp>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/any.hpp>

typedef long double long_double_t;
typedef signed long long long_int_t;
typedef unsigned long long hashcode_t;

struct Hash
{
	std::vector<std::string> variable_hash;

	Hash() {
		variable_hash.push_back("<LAMBDA>");
	}

	inline hashcode_t hashString(const std::string &key)
	{
		if (std::find(variable_hash.begin(), variable_hash.end(), key) != variable_hash.end())
			return std::distance(variable_hash.begin(), std::find(variable_hash.begin(), variable_hash.end(), key));
		variable_hash.push_back(key);
		return variable_hash.size() - 1;
	}

	inline std::string deHash(hashcode_t code)
	{
		return variable_hash[code];
	}
};

extern Hash hash;

#ifdef _USE_CONV_NUM_
#define NUMBER_TYPE SmartNumber
#define NUMBER_STRING(n) n.toString()
#define NUMBER_NEW_DOUBLE(v) SmartNumber(static_cast<long_double_t>(v))
#define NUMBER_NEW_LONG(v) SmartNumber(static_cast<long_int_t>(v))
#define NUMBER_POW(a, b) a ^ b
#define NUMBER_MOD(a, b) a % b
#define NUMBER_GET_LONG(v) v.getLong()
#define NUMBER_GET_DOUBLE(v) v.getDouble()
#else
#define NUMBER_TYPE long_double_t
#define NUMBER_STRING(n) std::to_string(n)
#define NUMBER_NEW_DOUBLE(v) static_cast<long_double_t>(v)
#define NUMBER_NEW_LONG(v) static_cast<long_double_t>(v)
#define NUMBER_POW(a, b) static_cast<long_double_t>(std::pow(a, b))
#define NUMBER_MOD(a, b) static_cast<long_double_t>((long_int_t)a % (long_int_t)b)
#define NUMBER_GET_LONG(v) static_cast<long_double_t>(v)
#define NUMBER_GET_DOUBLE(v) static_cast<long_double_t>(v)
#endif

#ifdef _USE_RAW_PTR_
#define NEW_VALUE_0 new Value()
#define NEW_VALUE_1(a) new Value(a)
#define NEW_VALUE_2(a, b) new Value(a, b)
#else
#define NEW_VALUE_0 std::make_shared<Value>()
#define NEW_VALUE_1(a) std::make_shared<Value>(a)
#define NEW_VALUE_2(a, b) std::make_shared<Value>(a, b)
#endif

#define RUOTA_EXT_SYM(name, args) extern "C" BOOST_SYMBOL_EXPORT Symbol name(std::vector<Symbol> args)
#define RUOTA_LIB_HEADER Hash hash = Hash();

class Token;
class Lexer;
class Symbol;
class Value;

class Function;
class Scope;
class Instruction;
class Object;

enum TextColor
{
	BLACK_TEXT = 30,
	RED_TEXT = 31,
	GREEN_TEXT = 32,
	YELLOW_TEXT = 33,
	BLUE_TEXT = 34,
	MAGENTA_TEXT = 35,
	CYAN_TEXT = 36,
	WHITE_TEXT = 37,
	BRIGHT_BLACK_TEXT = 90,
	BRIGHT_RED_TEXT = 91,
	BRIGHT_GREEN_TEXT = 92,
	BRIGHT_YELLOW_TEXT = 93,
	BRIGHT_BLUE_TEXT = 94,
	BRIGHT_MAGENTA_TEXT = 95,
	BRIGHT_CYAN_TEXT = 96,
	BRIGHT_WHITE_TEXT = 97,

	RESET_TEXT = 0
};

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
	TOK_TYPE_NAME = -32,
	TOK_DICTIONARY = -33,
	TOK_OBJECT = -34,
	TOK_FUNCTION = -35,
	TOK_CAST = -36,

	TOK_LOAD = -37,
	TOK_ALLOC = -38,
	TOK_UNTIL = -39,
	TOK_REF = -40,
	TOK_FINAL = -41,
	TOK_DEF_TYPE = -42,
	TOK_BREAK = -43,
	TOK_REFER = -44,
	TOK_NIL_NAME = -45,
	TOK_POINTER = -46,
	TOK_VIRTUAL = -47
};

enum DID_TYPE
{
	ID_RETURN,
	ID_BREAK,
	ID_CASUAL,
	ID_REFER
};

enum D_TYPE
{
	NIL = -1,
	NUMBER = -2,
	BOOLEAN_D = -3,
	STRING = -4,
	VECTOR = -5,
	FUNCTION = -6,
	DICTIONARY = -7,
	OBJECT = -8,
	TYPE_NAME = -9,
	POINTER = -10
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
	CAST_TO_I,
	POW_I,
	ALLOC_I,
	UNTIL_I,
	SCOPE_I,
	REFER_I,
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
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params;
	std::shared_ptr<Instruction> body;
	Scope *parent;

public:
	Function(Scope &, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>>, std::shared_ptr<Instruction>);
	Symbol evaluate(std::vector<Symbol>);
	Symbol evaluate(std::vector<Symbol>, Symbol *thisSymbol);
	const unsigned long getArgSize() const;
};

class Scope
{
private:
	std::string name;
	Scope *parent;
	std::map<hashcode_t, Symbol> values;

public:
	Scope();
	Scope(Scope &, const std::string &);
	Scope *getParent();
	Symbol getVariable(hashcode_t);
	Symbol createVariable(hashcode_t);
	Symbol createVariable(hashcode_t, Symbol);
	const std::string &getName() const;
	bool hasValue(hashcode_t) const;

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
	std::shared_ptr<Instruction> getBody() const;
	const std::string &getName() const;
	bool hasValue(hashcode_t) const;
};

class Instruction
{
protected:
	I_TYPE type;

public:
	Instruction(I_TYPE);
	virtual Symbol evaluate(Scope &) const = 0;
	virtual const std::string toString(bool) const = 0;
	const I_TYPE getType();
	virtual ~Instruction();
};

enum NumberType
{
	DOUBLE_NUM,
	LONG_NUM
};

#ifdef _USE_CONV_NUM_
struct SmartNumber
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

	SmartNumber() : valueLong(0), valueDouble(0), type(LONG_NUM) {}
	SmartNumber(const SmartNumber &n)
	{
		this->type = n.type;
		this->valueDouble = n.valueDouble;
		this->valueLong = n.valueLong;
	}
	SmartNumber(long_double_t valueDouble) : valueDouble(valueDouble), valueLong(0), type(DOUBLE_NUM) { validate(); }
	SmartNumber(long_int_t valueLong) : valueLong(valueLong), valueDouble(0), type(LONG_NUM) {}

	inline const SmartNumber operator+(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueDouble + n.valueDouble);
			case LONG_NUM:
				return SmartNumber(valueDouble + static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_double_t>(valueLong) + n.valueDouble);
			case LONG_NUM:
				return SmartNumber(valueLong + n.valueLong);
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

	inline const SmartNumber operator-(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueDouble - n.valueDouble);
			case LONG_NUM:
				return SmartNumber(valueDouble - static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_double_t>(valueLong) - n.valueDouble);
			case LONG_NUM:
				return SmartNumber(valueLong - n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const SmartNumber operator*(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueDouble * n.valueDouble);
			case LONG_NUM:
				return SmartNumber(valueDouble * static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_double_t>(valueLong) * n.valueDouble);
			case LONG_NUM:
				return SmartNumber(valueLong * n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const SmartNumber operator/(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueDouble / n.valueDouble);
			case LONG_NUM:
				return SmartNumber(valueDouble / static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_double_t>(valueLong) / n.valueDouble);
			case LONG_NUM:
				return SmartNumber(static_cast<long_double_t>(valueLong) / static_cast<long_double_t>(n.valueLong));
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const SmartNumber operator^(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_double_t>(std::pow(valueDouble, n.valueDouble)));
			case LONG_NUM:
				return SmartNumber(static_cast<long_double_t>(std::pow(valueDouble, static_cast<long_double_t>(n.valueLong))));
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_double_t>(std::pow(static_cast<long_double_t>(valueLong), n.valueDouble)));
			case LONG_NUM:
				return SmartNumber(static_cast<long_double_t>(std::pow(valueLong, n.valueLong)));
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline const SmartNumber operator%(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) % static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) % n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueLong % static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(valueLong % n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		throw std::runtime_error("Invalid conversion - not a user error");
	}

	inline bool operator==(const SmartNumber &n) const
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

	inline bool operator!=(const SmartNumber &n) const
	{
		return !(*this == n);
	}

	inline bool operator<(const SmartNumber &n) const
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

	inline bool operator>(const SmartNumber &n) const
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

	inline const bool operator<=(const SmartNumber &n) const
	{
		return !(*this > n);
	}

	inline const bool operator>=(const SmartNumber &n) const
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
#endif

class Token
{
private:
	std::string valueString;
	std::string line;
	NUMBER_TYPE valueNumber;
	int type;
	unsigned long distance;

public:
	Token() : line(""), distance(0), valueString(""), valueNumber((long_int_t)0), type(0) {}
	Token(const std::string &line, unsigned long distance, const std::string &, NUMBER_TYPE, int);
	const std::string getLine() const { return this->line; };
	const int getType() const { return this->type; }
	const unsigned long getDist() const { return this->distance; }
	const NUMBER_TYPE getValueNumber() const { return this->valueNumber; }
	const std::string getValueString() const { return this->valueString; }
};

class Lexer
{
private:
	std::string ID_STRING;
	unsigned long LINE_INDEX;
	NUMBER_TYPE NUM_VALUE;
	std::string INPUT;
	unsigned long INPUT_INDEX;
	unsigned long TOKEN_DIST;
	const int getToken();
	const char peekChar() const;
	const char nextChar();
	std::map<std::string, signed int> bOperators;
	std::map<std::string, signed int> uOperators;

public:
	Lexer(std::map<std::string, signed int> bOperators, std::map<std::string, signed int> uOperators) : bOperators(bOperators), uOperators(uOperators) {}
	std::vector<Token> lexString(const std::string &);
};

class Value
{
	friend class Symbol;

private:
	D_TYPE type;
	std::shared_ptr<void> valuePointer;
	signed long long valueType;
	NUMBER_TYPE valueNumber;
	bool valueBool;
	std::vector<Symbol> valueVector;
	std::string valueString;
	std::map<D_TYPE, std::map<size_t, std::shared_ptr<Function>>> valueFunction;
	std::map<hashcode_t, Symbol> valueDictionary;
	std::shared_ptr<Object> valueObject;
#ifdef _USE_RAW_PTR_
	unsigned long long references = 1;
#endif

public:
	Value() : type(NIL) {}
	Value(signed long long valueType) : type(TYPE_NAME), valueType(valueType) {}
	Value(bool valueBool) : type(BOOLEAN_D), valueBool(valueBool) {}
	Value(std::shared_ptr<void> valuePointer) : type(POINTER), valuePointer(valuePointer) {}
	Value(std::shared_ptr<Object> &valueObject) : type(OBJECT), valueObject(valueObject) {}
	Value(D_TYPE ftype, std::shared_ptr<Function> &function) : type(FUNCTION), valueFunction({{ftype, {{function->getArgSize(), function}}}}) {}
	Value(NUMBER_TYPE valueNumber) : type(NUMBER), valueNumber(valueNumber) {}
	Value(std::vector<Symbol> valueVector) : type(VECTOR), valueVector(valueVector) {}
	Value(std::map<hashcode_t, Symbol> valueDictionary) : type(DICTIONARY), valueDictionary(valueDictionary) {}
	Value(const std::string &valueString) : type(STRING), valueString(valueString) {}
};

class Symbol
{
private:
	DID_TYPE type;
	bool isMutable;
#ifdef _USE_RAW_PTR_
	Value *d;
#else
	std::shared_ptr<Value> d;
#endif

public:
	Symbol() : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_0) {}

	Symbol(DID_TYPE type) : type(type), isMutable(false), d(NEW_VALUE_0) {}

	Symbol(std::shared_ptr<void> valuePointer) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valuePointer)) {}

	Symbol(signed long long valueType) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valueType)) {}

	Symbol(NUMBER_TYPE valueNumber) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valueNumber)) {}

	Symbol(bool valueBool) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valueBool)) {}

	Symbol(std::vector<Symbol> valueVector) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valueVector)) {}

	Symbol(std::shared_ptr<Object> &valueObject) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valueObject)) {}

	Symbol(D_TYPE ftype, std::shared_ptr<Function> &valueFunction) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_2(ftype, valueFunction)) {}

	Symbol(const std::string &valueString) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valueString)) {}

	Symbol(const std::map<hashcode_t, Symbol> valueDictionary) : type(ID_CASUAL), isMutable(true), d(NEW_VALUE_1(valueDictionary)) {}

#ifdef _USE_RAW_PTR_
	Symbol(const Symbol &s)
	{
		this->type = s.type;
		this->isMutable = s.isMutable;
		this->d = s.d;
		this->d->references++;
	}

	~Symbol()
	{
		d->references--;
		if (d->references == 0)
			delete d;
	}

	inline void operator=(const Symbol &b)
	{
		this->d->references--;
		if (this->d->references == 0)
			delete d;

		this->d = b.d;
		this->type = b.type;
		this->isMutable = b.isMutable;
		this->d->references++;
	}
#endif

	inline Symbol setMutable(bool isMutable)
	{
		this->isMutable = isMutable;
		return *this;
	}

	inline bool canSet() const
	{
		return isMutable;
	}

	inline DID_TYPE getSymbolType() const
	{
		return type;
	}

	inline void setSymbolType(DID_TYPE type)
	{
		this->type = type;
	}

	inline NUMBER_TYPE getNumber() const
	{
		if (d->type != NUMBER)
			throw std::runtime_error("Value is not of type `Number`");

		return d->valueNumber;
	}

	inline std::shared_ptr<void> getPointer() const
	{
		if (d->type != POINTER)
			throw std::runtime_error("Value is not of type `Pointer`");

		return d->valuePointer;
	}

	inline std::map<hashcode_t, Symbol> getDictionary() const
	{
		if (d->type != DICTIONARY)
			throw std::runtime_error("Value is not of type `Dictionary`");

		auto iter = d->valueDictionary.begin();
		for (; iter != d->valueDictionary.end();)
		{
			if (iter->second.getValueType() == NIL)
				iter = d->valueDictionary.erase(iter);
			else
				++iter;
		}
		return d->valueDictionary;
	}

	inline std::vector<Symbol> getVector() const
	{
		if (d->type != VECTOR)
			throw std::runtime_error("Value is not of type `Vector`");

		return d->valueVector;
	}

	inline std::string getString() const
	{
		if (d->type != STRING)
			throw std::runtime_error("Value is not of type `String`");

		return d->valueString;
	}

	inline bool getBool() const
	{
		if (d->type != BOOLEAN_D)
			throw std::runtime_error("Value is not of type `Boolean`");

		return d->valueBool;
	}

	inline std::shared_ptr<Object> getObject() const
	{
		if (d->type != OBJECT)
			throw std::runtime_error("Value is not of type `Object`");

		return d->valueObject;
	}

	inline D_TYPE getValueType() const
	{
		return d->type;
	}

	inline signed long long getTypeName() const
	{
		return d->valueType;
	}

	inline std::shared_ptr<Function> getFunction(D_TYPE ftype, size_t argSize)
	{
		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type `Function`");

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
		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type `Function`");

		return d->valueFunction;
	}

	inline Symbol indexVector(size_t index) const
	{
		if (d->type != VECTOR)
			throw std::runtime_error("Value is not of type `Vector`");

		return d->valueVector[index];
	}

	inline Symbol indexDictionary(hashcode_t key) const
	{
		if (d->type != DICTIONARY)
			throw std::runtime_error("Value is not of type `Dictionary`");

		return d->valueDictionary[key];
	}

	inline bool hasDictionaryKey(hashcode_t key) const
	{
		if (d->type != DICTIONARY)
			throw std::runtime_error("Value is not of type `Dictionary`");

		return d->valueDictionary.find(key) != d->valueDictionary.end();
	}

	inline size_t vectorSize() const
	{
		if (d->type != VECTOR)
			throw std::runtime_error("Value is not of type `Vector`");

		return d->valueVector.size();
	}

	inline size_t dictionarySize() const
	{
		if (d->type != DICTIONARY)
			throw std::runtime_error("Value is not of type `Dictionary`");

		return getDictionary().size();
	}

	inline std::string toString() const
	{
		switch (d->type)
		{
		case NIL:
			return "nil";
		case NUMBER:
			return NUMBER_STRING(d->valueNumber);
		case STRING:
			return d->valueString;
		case FUNCTION:
			return "<Function>";
		case OBJECT:
			return "<Object>";
		case POINTER:
			return "<Pointer>";
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
				ret += "\"" + hash.deHash(e.first) + "\" : " + e.second.toString();
				i++;
			}
			return ret + "}";
		}
		case TYPE_NAME:
		{
			auto a = getTypeName();
			switch (getTypeName())
			{
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
			case VECTOR:
				return "Type::Vector";
			case DICTIONARY:
				return "Type::Dictionary";
			case TYPE_NAME:
				return "Type::Type";
			case POINTER:
				return "Type::Pointer";
			default:
				return "Type::Object::" + hash.deHash(a);
			}
		}
		default:
			return "undefined";
		}
	}

	inline std::string toCodeString() const
	{
		if (type == ID_BREAK)
			return "<BREAK>";
		switch (d->type)
		{
		case NIL:
			return std::string("Symbol()") + (!isMutable ? ".setMutable(false)" : "");
		case NUMBER:
		{
			std::string ret = "Symbol(NUMBER_NEW_";
#ifdef _USE_CONV_NUM_
			if (d->valueNumber.type == DOUBLE_NUM)
				ret += "DOUBLE(" + std::to_string(NUMBER_GET_DOUBLE(d->valueNumber)) + ")" + (!isMutable ? ".setMutable(false)" : "");
			else
				ret += "LONG(" + std::to_string(NUMBER_GET_LONG(d->valueNumber)) + ")" + (!isMutable ? ".setMutable(false)" : "");
#else
			ret += "DOUBLE(" + std::to_string(d->valueNumber) + ")";
#endif
			return ret + ")";
		}
		case STRING:
			return "Symbol(\"" + d->valueString + "\")" + (!isMutable ? ".setMutable(false)" : "");
		case FUNCTION:
			return std::string("<Function>") + (!isMutable ? ".setMutable(false)" : "");
		case OBJECT:
			return std::string("<Object>") + (!isMutable ? ".setMutable(false)" : "");
		case BOOLEAN_D:
		{
			std::string ret = "Symbol(";
			ret += (d->valueBool ? "true" : "false");
			return ret + ")" + (!isMutable ? ".setMutable(false)" : "");
		}
		case VECTOR:
		{
			std::string ret = "Symbol({";
			unsigned int i = 0;
			for (auto &d2 : d->valueVector)
			{
				if (i > 0)
					ret += ", ";
				ret += d2.toCodeString();
				i++;
			}
			return ret + "})" + (!isMutable ? ".setMutable(false)" : "");
		}
		case DICTIONARY:
		{
			std::string ret = "Symbol({";
			unsigned int i = 0;
			for (auto &e : getDictionary())
			{
				if (i > 0)
					ret += ", ";
				ret += "{" + std::to_string(e.first) + ", " + e.second.toCodeString() + "}";
				i++;
			}
			return ret + "})" + (!isMutable ? ".setMutable(false)" : "");
		}
		case TYPE_NAME:
			return std::string("Symbol(static_cast<TYPE_NAME>(") + std::to_string(getTypeName()) + "))" + (!isMutable ? ".setMutable(false)" : "");
		default:
			return "undefined";
		}
	}

	inline void clearData()
	{
		d->valueFunction.clear();
		d->valueVector.clear();
		d->valueDictionary.clear();
		d->valuePointer = nullptr;
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
		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type `Function`");
		auto fs = b.getFunctions();
		for (auto &f : fs)
			for (auto &t : f.second)
				d->valueFunction[f.first][t.first] = t.second;
	}

	inline Symbol operator+(const Symbol &b) const
	{
		if (d->type != b.getValueType())
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
		d->type = b.getValueType();
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
		case POINTER:
			d->valuePointer = b.getPointer();
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

				if (e.second.getValueType() == NIL)
					continue;
				auto newd = Symbol();
				newd.set(e.second);
				d->valueDictionary[e.first] = newd;
			}
			break;
		}
		case TYPE_NAME:
			d->valueType = b.getTypeName();
		default:
			break;
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
		return Symbol(NUMBER_MOD(d->valueNumber, valB));
	}

	inline Symbol operator^(const Symbol &b) const
	{
		auto valB = b.getNumber();
		return Symbol(NUMBER_POW(d->valueNumber, valB));
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

	inline Symbol equals(const Symbol &b) const
	{
		if (d->type != b.getValueType())
			return Symbol(false);
		switch (d->type)
		{
		case NIL:
			return Symbol(true);
		case NUMBER:
			return Symbol(d->valueNumber == b.getNumber());
		case BOOLEAN_D:
			return Symbol(d->valueBool == b.getBool());
		case STRING:
			return Symbol(d->valueString == b.getString());
		case OBJECT:
			return Symbol(d->valueObject == b.getObject());
		case VECTOR:
			if (d->valueVector.size() != b.vectorSize())
				return Symbol(false);
			for (unsigned long i = 0; i < d->valueVector.size(); i++)
			{
				if (!d->valueVector[i].equals(b.indexVector(i)).getBool())
					return Symbol(false);
			}
			return Symbol(true);
		case DICTIONARY:
			for (auto &e : d->valueDictionary)
			{
				if (!e.second.equals(b.indexDictionary(e.first)).getBool())
					return Symbol(false);
			}
			return Symbol(true);
		case TYPE_NAME:
			return Symbol(d->valueType == b.getTypeName());
		default:
			return Symbol(false);
		}
	}

	inline Symbol nequals(const Symbol &b) const
	{
		return Symbol(!(this->equals(b).getBool()));
	}
};

#endif