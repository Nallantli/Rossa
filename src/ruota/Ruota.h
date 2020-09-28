#ifndef RUOTA_H
#define RUOTA_H

#define _USE_CONV_NUM_
#define _USE_RAW_PTR_

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <stdexcept>
#include <boost/config.hpp>
#include <boost/filesystem.hpp>

typedef long double long_double_t;
typedef signed long long long_int_t;
typedef unsigned long long hashcode_t;

#define _RUOTA_VERSION_ "v1.0.0-beta"

#ifdef _USE_CONV_NUM_
#define NUMBER_TYPE SmartNumber
#define NUMBER_STRING(n) n.toString()
#define NUMBER_NEW_DOUBLE(v) SmartNumber(static_cast<long_double_t>(v))
#define NUMBER_NEW_LONG(v) SmartNumber(static_cast<long_int_t>(v))
#define NUMBER_POW(a, b) a.pow(b)
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

#define RUOTA_EXT_SYM(name, args) extern "C" BOOST_SYMBOL_EXPORT const Symbol name(std::vector<Symbol> args)
#define RUOTA_LIB_HEADER Hash hash = Hash();

struct Hash
{
	std::vector<std::string> variable_hash;

	Hash()
	{
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

class Ruota;
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
	TOK_VIRTUAL = -47,
	TOK_SWITCH = -48,
	TOK_TRY = -49,
	TOK_CATCH = -50,
	TOK_THROW = -51,
	TOK_CHARN = -52,
	TOK_CHARS = -53
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
	SWITCH_I,
	TRY_CATCH_I,
	THROW_I,
	CHARS_I,
	CHARN_I,
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
	PURE_EQUALS,
	PURE_NEQUALS,
	AND,
	OR,
	B_AND,
	B_OR,
	B_XOR,
	B_SH_L,
	B_SH_R
};

enum OBJECT_TYPE
{
	STRUCT_O,
	STATIC_O,
	INSTANCE_O,
	VIRTUAL_O
};

class Function
{
private:
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params;
	std::shared_ptr<Instruction> body;
	Scope *parent;
	hashcode_t key;

public:
	Function(hashcode_t, Scope &, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>>, std::shared_ptr<Instruction>);
	const Symbol evaluate(std::vector<Symbol>) const;
	const Symbol evaluate(std::vector<Symbol>, Symbol *) const;
	size_t getArgSize() const;
	hashcode_t getKey() const;
	Scope *getParent() const;
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> getParams() const;
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
	Scope *getParent() const;
	const Symbol &getVariable(hashcode_t) const;
	const Symbol &createVariable(hashcode_t);
	const Symbol &createVariable(hashcode_t, const Symbol &);
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
	std::shared_ptr<Scope> getScope() const;
	const Symbol instantiate(std::vector<Symbol>) const;
	OBJECT_TYPE getType() const;
	std::shared_ptr<Instruction> getBody() const;
	const std::string &getName() const;
	bool hasValue(hashcode_t) const;

	~Object();
};

class Instruction
{
protected:
	I_TYPE type;

public:
	Instruction(I_TYPE);
	virtual const Symbol evaluate(Scope &) const = 0;
	virtual const std::string toString(bool) const = 0;
	I_TYPE getType() const;
	virtual ~Instruction();
};

class Ruota
{
private:
	static const std::map<std::string, signed int> bOperators;
	static const std::map<std::string, signed int> uOperators;
	Scope main;

public:
	static Lexer lexer;

	static std::vector<Function> stack_trace;

	static hashcode_t HASH_THIS;
	static hashcode_t HASH_INIT;
	static hashcode_t HASH_KEY;
	static hashcode_t HASH_VALUE;
	static hashcode_t HASH_DELETER;

	static hashcode_t HASH_ADD;
	static hashcode_t HASH_SUB;
	static hashcode_t HASH_MUL;
	static hashcode_t HASH_DIV;
	static hashcode_t HASH_MOD;
	static hashcode_t HASH_POW;
	static hashcode_t HASH_B_AND;
	static hashcode_t HASH_B_OR;
	static hashcode_t HASH_B_XOR;
	static hashcode_t HASH_B_SH_L;
	static hashcode_t HASH_B_SH_R;
	static hashcode_t HASH_LESS;
	static hashcode_t HASH_MORE;
	static hashcode_t HASH_ELESS;
	static hashcode_t HASH_EMORE;
	static hashcode_t HASH_INDEX;
	static hashcode_t HASH_EQUALS;
	static hashcode_t HASH_NEQUALS;
	static hashcode_t HASH_SET;
	static hashcode_t HASH_CALL;

	static hashcode_t HASH_TO_STRING;
	static hashcode_t HASH_TO_NUMBER;
	static hashcode_t HASH_TO_BOOLEAN;
	static hashcode_t HASH_TO_VECTOR;
	static hashcode_t HASH_TO_DICTIONARY;

	Ruota();
	Symbol parseCode(const std::string &, boost::filesystem::path, bool);
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
		return SmartNumber();
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
		return SmartNumber();
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
		return SmartNumber();
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
		return SmartNumber();
	}

	inline const SmartNumber pow(const SmartNumber &n) const
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
		return SmartNumber();
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
		return SmartNumber();
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
		return false;
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
		return false;
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
		return false;
	}

	inline bool operator<=(const SmartNumber &n) const
	{
		return !(*this > n);
	}

	inline bool operator>=(const SmartNumber &n) const
	{
		return !(*this < n);
	}

	inline const SmartNumber operator&(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) & static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) & n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueLong & static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(valueLong & n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return SmartNumber();
	}

	inline const SmartNumber operator|(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) | static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) | n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueLong | static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(valueLong | n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return SmartNumber();
	}

	inline const SmartNumber operator^(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) ^ static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) ^ n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueLong ^ static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(valueLong ^ n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return SmartNumber();
	}

	inline const SmartNumber operator<<(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) << static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) << n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueLong << static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(valueLong << n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return SmartNumber();
	}

	inline const SmartNumber operator>>(const SmartNumber &n) const
	{
		switch (type)
		{
		case DOUBLE_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) >> static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(static_cast<long_int_t>(valueDouble) >> n.valueLong);
			default:
				break;
			}
		case LONG_NUM:
			switch (n.type)
			{
			case DOUBLE_NUM:
				return SmartNumber(valueLong >> static_cast<long_int_t>(n.valueDouble));
			case LONG_NUM:
				return SmartNumber(valueLong >> n.valueLong);
			default:
				break;
			}
		default:
			break;
		}
		return SmartNumber();
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
			return "<undefined>";
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
			return 0;
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
			return 0;
		}
	}
};
#endif

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

	inline void setMutable(bool isMutable)
	{
		this->isMutable = isMutable;
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

	inline const std::string &getString() const
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

	inline std::shared_ptr<Function> getFunction(D_TYPE ftype, size_t argSize) const
	{
		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type `Function`");

		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type Function: " + std::to_string(d->type));

		std::map<size_t, std::shared_ptr<Function>> foftype;

		if (ftype != NIL && d->valueFunction.find(ftype) != d->valueFunction.end())
			foftype = d->valueFunction.at(ftype);
		else if (d->valueFunction.find(NIL) != d->valueFunction.end())
			foftype = d->valueFunction.at(NIL);
		else
			throw std::runtime_error("Function does not exist for given value");
		if (foftype.find(argSize) == foftype.end())
			throw std::runtime_error("Function override for parameter size " + std::to_string(argSize) + " does not exist");

		return foftype.at(argSize);
	}

	inline std::map<D_TYPE, std::map<size_t, std::shared_ptr<Function>>> getFunctions() const
	{
		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type `Function`");

		return d->valueFunction;
	}

	inline const Symbol index(const Symbol &a) const
	{
		switch (d->type)
		{
		case VECTOR:
		{
			auto i = NUMBER_GET_LONG(a.getNumber());
			if (i >= d->valueVector.size())
				throw std::runtime_error("Array index out of bounds");
			return d->valueVector[NUMBER_GET_LONG(a.getNumber())];
		}
		case DICTIONARY:
			return d->valueDictionary[hash.hashString(a.getString())];
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_INDEX))
			{
				return o->getScope()->getVariable(Ruota::HASH_INDEX).call(NIL, {a}, this);
			}
			throw std::runtime_error("Operator `[]` is undefined for Object type");
		}
		default:
			throw std::runtime_error("Operator `[]` is undefined for value type");
		}
	}

	inline const Symbol indexDict(hashcode_t i) const
	{
		return d->valueDictionary[i];
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

	inline const std::string toString() const
	{
		switch (d->type)
		{
		case NIL:
			return "nil";
		case NUMBER:
			return NUMBER_STRING(d->valueNumber);
		case STRING:
			return "\"" + d->valueString + "\"";
		case FUNCTION:
			return "<Function>";
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_TO_STRING))
			{
				return o->getScope()->getVariable(Ruota::HASH_TO_STRING).call(NIL, {}, this).getString();
			}
			return "<Object>";
		}
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

	inline const std::string toCodeString() const
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

	inline void clearData() const
	{
		d->valueFunction.clear();
		d->valueVector.clear();
		d->valueDictionary.clear();
		d->valuePointer = nullptr;
		d->valueObject = nullptr;
	}

	inline const Symbol call(D_TYPE ftype, std::vector<Symbol> params) const
	{
		if (d->type == OBJECT)
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL).call(NIL, params, this);
			}
		}

		auto f = getFunction(ftype, params.size());

		return f->evaluate(params);
	}

	inline const Symbol call(D_TYPE ftype, std::vector<Symbol> params, Symbol b) const
	{
		if (d->type == OBJECT)
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL).call(NIL, params, this);
			}
		}

		auto f = getFunction(ftype, params.size());

		return f->evaluate(params, &b);
	}

	inline void addFunctions(const Symbol &b) const
	{
		if (d->type != FUNCTION)
			throw std::runtime_error("Value is not of type `Function`");
		auto fs = b.getFunctions();
		for (auto &f : fs)
			for (auto &t : f.second)
				d->valueFunction[f.first][t.first] = t.second;
	}

	inline const Symbol operator+(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber + b.getNumber());
		case VECTOR:
		{
			auto valA = d->valueVector;
			auto valB = b.getVector();
			valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
			return Symbol(valA);
		}
		case STRING:
			return Symbol(d->valueString + b.getString());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_ADD))
			{
				return o->getScope()->getVariable(Ruota::HASH_ADD).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `+` is undefined for value type");
		}
	}

	inline void set(const Symbol &b) const
	{
		if (!isMutable)
			throw std::runtime_error("Cannot change the value of a variable declared as `final`");
		if (d->type == OBJECT && d->valueObject != nullptr && d->valueObject->hasValue(Ruota::HASH_SET))
		{
			d->valueObject->getScope()->getVariable(Ruota::HASH_SET).call(NIL, {b}, this);
			return;
		}
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

	inline const Symbol operator-(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber - b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_SUB))
			{
				return o->getScope()->getVariable(Ruota::HASH_SUB).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `-` is undefined for value type");
		}
	}

	inline const Symbol operator*(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber * b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_MUL))
			{
				return o->getScope()->getVariable(Ruota::HASH_MUL).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `*` is undefined for value type");
		}
	}

	inline const Symbol operator/(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber / b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_DIV))
			{
				return o->getScope()->getVariable(Ruota::HASH_DIV).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `/` is undefined for value type");
		}
	}

	inline const Symbol operator%(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber % b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_MOD))
			{
				return o->getScope()->getVariable(Ruota::HASH_MOD).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `%` is undefined for value type");
		}
	}

	inline const Symbol pow(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(NUMBER_POW(d->valueNumber, b.getNumber()));
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_POW))
			{
				return o->getScope()->getVariable(Ruota::HASH_POW).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `**` is undefined for value type");
		}
	}

	inline const Symbol operator&(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber & b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_B_AND))
			{
				return o->getScope()->getVariable(Ruota::HASH_B_AND).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `&` is undefined for value type");
		}
	}

	inline const Symbol operator|(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber | b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_B_OR))
			{
				return o->getScope()->getVariable(Ruota::HASH_B_OR).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `|` is undefined for value type");
		}
	}

	inline const Symbol operator^(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber ^ b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_B_XOR))
			{
				return o->getScope()->getVariable(Ruota::HASH_B_XOR).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `^` is undefined for value type");
		}
	}

	inline const Symbol operator<<(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber << b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_B_SH_L))
			{
				return o->getScope()->getVariable(Ruota::HASH_B_SH_L).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `<<` is undefined for value type");
		}
	}

	inline const Symbol operator>>(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return Symbol(d->valueNumber >> b.getNumber());
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_B_SH_L))
			{
				return o->getScope()->getVariable(Ruota::HASH_B_SH_R).call(NIL, {b}, this);
			}
		}
		default:
			throw std::runtime_error("Operator `>>` is undefined for value type");
		}
	}

	inline bool operator<(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return d->valueNumber < b.getNumber();
		case STRING:
			return d->valueString < b.getString();
		case TYPE_NAME:
			return d->valueType < b.getTypeName();
		case BOOLEAN_D:
			return d->valueBool < b.getBool();
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_LESS))
			{
				return o->getScope()->getVariable(Ruota::HASH_LESS).call(NIL, {b}, this).getBool();
			}
			throw std::runtime_error("Operator `<` not defined for Object type");
		}
		default:
			return toString() < b.toString();
		}
	}

	inline bool operator>(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return d->valueNumber > b.getNumber();
		case STRING:
			return d->valueString > b.getString();
		case TYPE_NAME:
			return d->valueType > b.getTypeName();
		case BOOLEAN_D:
			return d->valueBool > b.getBool();
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_MORE))
			{
				return o->getScope()->getVariable(Ruota::HASH_MORE).call(NIL, {b}, this).getBool();
			}
			throw std::runtime_error("Operator `>` not defined for Object type");
		}
		default:
			return toString() > b.toString();
		}
	}

	inline bool operator<=(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return d->valueNumber <= b.getNumber();
		case STRING:
			return d->valueString <= b.getString();
		case TYPE_NAME:
			return d->valueType <= b.getTypeName();
		case BOOLEAN_D:
			return d->valueBool <= b.getBool();
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_ELESS))
			{
				return o->getScope()->getVariable(Ruota::HASH_ELESS).call(NIL, {b}, this).getBool();
			}
			throw std::runtime_error("Operator `<=` not defined for Object type");
		}
		default:
			return toString() <= b.toString();
		}
	}

	inline bool operator>=(const Symbol &b) const
	{
		switch (d->type)
		{
		case NUMBER:
			return d->valueNumber >= b.getNumber();
		case STRING:
			return d->valueString >= b.getString();
		case TYPE_NAME:
			return d->valueType >= b.getTypeName();
		case BOOLEAN_D:
			return d->valueBool >= b.getBool();
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_EMORE))
			{
				return o->getScope()->getVariable(Ruota::HASH_EMORE).call(NIL, {b}, this).getBool();
			}
			throw std::runtime_error("Operator `>=` not defined for Object type");
		}
		default:
			return toString() >= b.toString();
		}
	}

	inline bool operator==(const Symbol &b) const
	{
		if (d->type != b.getValueType() && d->type != OBJECT)
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
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_EQUALS))
			{
				return o->getScope()->getVariable(Ruota::HASH_EQUALS).call(NIL, {b}, this).getBool();
			}
			return o == b.getObject();
		}
		case VECTOR:
		{
			if (d->valueVector.size() != b.vectorSize())
				return false;
			auto bv = b.getVector();
			for (unsigned long i = 0; i < d->valueVector.size(); i++)
			{
				if (!(d->valueVector[i] == bv[i]))
					return false;
			}
			return true;
		}
		case DICTIONARY:
			for (auto &e : d->valueDictionary)
			{
				if (!(e.second == b.indexDict(e.first)))
					return false;
			}
			return true;
		case TYPE_NAME:
			return d->valueType == b.getTypeName();
		default:
			return false;
		}
	}

	inline bool operator!=(const Symbol &b) const
	{
		switch (d->type)
		{
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_NEQUALS))
			{
				return o->getScope()->getVariable(Ruota::HASH_NEQUALS).call(NIL, {b}, this).getBool();
			}
		}
		default:
			return !(*this == b);
		}
	}

	inline bool pureEquals(const Symbol &b) const
	{
		if (d->type != b.getValueType())
			return false;
		switch (d->type)
		{
		case OBJECT:
			return d->valueObject == b.getObject();
		default:
			return *this == b;
		}
	}

	inline bool pureNEquals(const Symbol &b) const
	{
		return !this->pureEquals(b);
	}
};

#endif