#ifndef RUOTA_H
#define RUOTA_H

#include "Locale.h"
#include "Declarations.h"
#include "CNumber.h"

#include <memory>
#include <map>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

struct Signature
{
	const std::vector<object_type_t> values;
	Signature(const std::vector<object_type_t> &);
	size_t validity(const std::vector<Symbol> &) const;
	bool operator<(const Signature &) const;
	const std::string toString() const;
};

class Token
{
private:
	std::string filename;
	std::string line;
	size_t lineNumber;
	size_t distance;
	std::string valueString;
	CNumber valueNumber;
	int type;

public:
	Token();
	Token(const std::string &, const std::string &, size_t, size_t, const std::string &, CNumber, int);
	const std::string &getLine() const;
	int getType() const;
	size_t getDist() const;
	size_t getLineNumber() const;
	const CNumber getValueNumber() const;
	const std::string &getValueString() const;
	const std::string &getFilename() const;
};

class RuotaError : public std::runtime_error
{
private:
	const Token token;

public:
	RuotaError(const std::string &error, const Token &token) : std::runtime_error(error), token(token) {}

	const Token &getToken() const
	{
		return token;
	}
};

class Instruction
{
protected:
	const InstructionType type;
	const Token token;

public:
	Instruction(const InstructionType &, const Token &);
	virtual const Symbol evaluate(Scope *) const = 0;
	InstructionType getType() const;
	virtual ~Instruction();
};

class Function
{
private:
	const hashcode_t key;
	Scope *parent;
	const std::vector<std::pair<LexerTokenType, hashcode_t>> params;
	const std::shared_ptr<Instruction> body;

public:
	Function(const hashcode_t &, Scope *, const std::vector<std::pair<LexerTokenType, hashcode_t>> &, const std::shared_ptr<Instruction> &);
	const Symbol evaluate(const std::vector<Symbol> &, const Symbol *, const Token *) const;
	size_t getArgSize() const;
	hashcode_t getKey() const;
	Scope *getParent() const;
	const std::vector<std::pair<LexerTokenType, hashcode_t>> &getParams() const;
};

class Scope
{
private:
	Scope *parent;
	std::vector<hashcode_t> name_trace;
	std::map<hashcode_t, Symbol> values;
	hashcode_t hashed_key;

public:
	Scope();
	Scope(Scope *, const hashcode_t &);
	Scope *getParent() const;
	void clear();
	const Symbol &getVariable(const hashcode_t &, const Token *);
	const Symbol &createVariable(const hashcode_t &, const Token *);
	const Symbol &createVariable(const hashcode_t &, const Symbol &, const Token *);
	const hashcode_t getHashedKey() const;
	bool hasValue(const hashcode_t &) const;
};

class Object
{
private:
	const ObjectType type;
	const std::shared_ptr<Instruction> body;
	const hashcode_t key;
	std::vector<object_type_t> extensions;
	const std::shared_ptr<Scope> internal;

public:
	Object(Scope *, const ObjectType &, const std::shared_ptr<Instruction> &, const hashcode_t &, const std::vector<object_type_t> &);
	Object(Scope *, const ObjectType &, const std::shared_ptr<Instruction> &, const hashcode_t &, const Object *);
	Scope *getScope() const;
	const Symbol instantiate(const std::vector<Symbol> &, const Token *) const;
	const ObjectType getType() const;
	const std::shared_ptr<Instruction> getBody() const;
	const hashcode_t getHashedKey() const;
	bool hasValue(const hashcode_t &) const;
	bool extendsObject(const hashcode_t &) const;

	~Object();
};

class Node
{
protected:
	const NodeType type;
	const Token token;

public:
	Node(NodeType, const Token);
	const NodeType getType() const;
	const Token getToken() const;

	virtual std::shared_ptr<Instruction> genParser() const = 0;
	virtual bool isConst() const = 0;
	virtual void printTree(std::string, bool) const = 0;
	virtual std::unique_ptr<Node> fold() const = 0;
};

class Lexer
{
private:
	std::string ID_STRING;
	size_t LINE_INDEX;
	CNumber NUM_VALUE;
	std::string INPUT;
	size_t INPUT_INDEX;
	size_t TOKEN_DIST;

	const int getToken();
	const char peekChar(const size_t &) const;
	const char nextChar();
	std::map<std::string, signed int> bOperators;
	std::map<std::string, signed int> uOperators;

public:
	Lexer(std::map<std::string, signed int>, std::map<std::string, signed int>);
	std::vector<Token> lexString(const std::string &, const std::string &);
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
	static hashcode_t HASH_RANGE;

	static hashcode_t HASH_TO_STRING;
	static hashcode_t HASH_TO_NUMBER;
	static hashcode_t HASH_TO_BOOLEAN;
	static hashcode_t HASH_TO_VECTOR;
	static hashcode_t HASH_TO_DICTIONARY;

	Ruota(std::vector<std::string>);
	std::unique_ptr<Node> compileCode(const std::string &, boost::filesystem::path) const;
	const Symbol runCode(std::unique_ptr<Node>, bool);
};

class Value
{
	friend class Symbol;

private:
	ValueType type;

	union
	{
		object_type_t valueType;
		bool valueBool;
		CNumber valueNumber;
	};

	std::string valueString;
	std::shared_ptr<void> valuePointer;
	std::vector<Symbol> valueVector;
	std::map<size_t, std::map<Signature, std::shared_ptr<Function>>> valueFunction;
	std::map<hashcode_t, Symbol> valueDictionary;
	std::shared_ptr<Object> valueObject;
	unsigned long long references = 1;

public:
	Value() : type(NIL) {}
	Value(const object_type_t &valueType) : type(TYPE_NAME), valueType(valueType) {}
	Value(const bool &valueBool) : type(BOOLEAN_D), valueBool(valueBool) {}
	Value(const std::shared_ptr<void> &valuePointer) : type(POINTER), valuePointer(valuePointer) {}
	Value(const std::shared_ptr<Object> &valueObject) : type(OBJECT), valueObject(valueObject) {}
	Value(const Signature &ftype, const std::shared_ptr<Function> &function) : type(FUNCTION), valueFunction({{function->getArgSize(), {{ftype, function}}}}) {}
	Value(const CNumber &valueNumber) : type(NUMBER), valueNumber(valueNumber) {}
	Value(const std::vector<Symbol> &valueVector) : type(ARRAY), valueVector(valueVector) {}
	Value(const std::map<hashcode_t, Symbol> &valueDictionary) : type(DICTIONARY), valueDictionary(valueDictionary) {}
	Value(const std::string &valueString) : type(STRING), valueString(valueString) {}

	inline void clearData()
	{
		switch (type)
		{
		case FUNCTION:
			valueFunction.clear();
			break;
		case ARRAY:
			valueVector.clear();
			break;
		case DICTIONARY:
			valueDictionary.clear();
			break;
		case POINTER:
			valuePointer = nullptr;
			break;
		case OBJECT:
			valueObject = nullptr;
			break;
		default:
			return;
		}
	}
};

class Symbol
{
private:
	SymbolType type;
	Value *d;

public:
	Symbol() : type(ID_CASUAL), d(new Value()) {}

	Symbol(const SymbolType &type) : type(type), d(new Value()) {}

	Symbol(const std::shared_ptr<void> &valuePointer) : type(ID_CASUAL), d(new Value(valuePointer)) {}

	Symbol(const object_type_t &valueType) : type(ID_CASUAL), d(new Value(valueType)) {}

	Symbol(const CNumber &valueNumber) : type(ID_CASUAL), d(new Value(valueNumber)) {}

	Symbol(const bool &valueBool) : type(ID_CASUAL), d(new Value(valueBool)) {}

	Symbol(const std::vector<Symbol> &valueVector) : type(ID_CASUAL), d(new Value(valueVector)) {}

	Symbol(const std::shared_ptr<Object> &valueObject) : type(ID_CASUAL), d(new Value(valueObject)) {}

	Symbol(const Signature &ftype, const std::shared_ptr<Function> &valueFunction) : type(ID_CASUAL), d(new Value(ftype, valueFunction)) {}

	Symbol(const std::string &valueString) : type(ID_CASUAL), d(new Value(valueString)) {}

	Symbol(const std::map<hashcode_t, Symbol> &valueDictionary) : type(ID_CASUAL), d(new Value(valueDictionary)) {}

	Symbol(const Symbol &s)
	{
		this->d = s.d;
		this->type = s.type;
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
		this->d->references++;
	}

	static inline const Symbol allocate(size_t size)
	{
		Symbol s;
		s.d->type = ARRAY;
		s.d->valueVector.resize(size);
		return s;
	}

	inline SymbolType getSymbolType() const
	{
		return type;
	}

	inline void setSymbolType(SymbolType type)
	{
		this->type = type;
	}

	inline const CNumber &getNumber(const Token *token) const
	{
		if (d->type != NUMBER)
			throw RuotaError(_NOT_NUMBER_, *token);
		return d->valueNumber;
	}

	inline void *getPointer(const Token *token) const
	{
		if (d->type != POINTER)
			throw RuotaError(_NOT_POINTER_, *token);
		return d->valuePointer.get();
	}

	inline const std::map<hashcode_t, Symbol> &getDictionary(const Token *token) const
	{
		if (d->type != DICTIONARY)
			throw RuotaError(_NOT_DICTIONARY_, *token);
		auto iter = d->valueDictionary.begin();
		for (; iter != d->valueDictionary.end();)
		{
			if (iter->second.d->type == NIL)
				iter = d->valueDictionary.erase(iter);
			else
				++iter;
		}
		return d->valueDictionary;
	}

	inline const Symbol &indexVector(size_t i, const Token *token) const
	{
		if (i >= d->valueVector.size())
			throw RuotaError((boost::format(_INDEX_OUT_OF_BOUNDS_) % d->valueVector.size() % i).str(), *token);
		return d->valueVector[i];
	}

	inline const std::vector<Symbol> &getVector(const Token *token) const
	{
		if (d->type != ARRAY)
			throw RuotaError(_NOT_VECTOR_, *token);
		return d->valueVector;
	}

	inline const std::string &getString(const Token *token) const
	{
		if (d->type != STRING)
			throw RuotaError(_NOT_STRING_, *token);
		return d->valueString;
	}

	inline bool getBool(const Token *token) const
	{
		if (d->type != BOOLEAN_D)
			throw RuotaError(_NOT_BOOLEAN_, *token);
		return d->valueBool;
	}

	inline const std::shared_ptr<Object> &getObject(const Token *token) const
	{
		if (d->type != OBJECT)
			throw RuotaError(_NOT_OBJECT_, *token);
		return d->valueObject;
	}

	inline ValueType getValueType() const
	{
		return d->type;
	}

	inline object_type_t getAugValueType() const
	{
		if (d->type == OBJECT)
			return d->valueObject->getHashedKey();
		return d->type;
	}

	inline object_type_t getTypeName(const Token *token) const
	{
		if (d->type != TYPE_NAME)
			throw RuotaError(_NOT_TYPE_, *token);
		return d->valueType;
	}

	inline const std::shared_ptr<Function> &getFunction(const std::vector<Symbol> &params, const Token *token) const
	{
		if (d->type != FUNCTION)
			throw RuotaError(_NOT_FUNCTION_, *token);

		if (d->valueFunction.find(params.size()) == d->valueFunction.end())
			throw RuotaError(_FUNCTION_ARG_SIZE_FAILURE_, *token);

		std::vector<object_type_t> ftypes;
		for (auto &e : params)
			ftypes.push_back(e.getAugValueType());

		std::map<Signature, std::shared_ptr<Function>> foftype = d->valueFunction[params.size()];
		const Signature *key = NULL;
		size_t cur_v = 0;
		for (auto &f2 : foftype)
		{
			size_t v = f2.first.validity(params);
			if (v > cur_v)
			{
				cur_v = v;
				key = &f2.first;
				if (v == ftypes.size() * 2)
					break;
			}
		}

		if (key == NULL)
			throw RuotaError(_FUNCTION_VALUE_NOT_EXIST_, *token);

		return foftype[*key];
	}

	inline const Symbol &indexDict(hashcode_t i) const
	{
		return d->valueDictionary[i];
	}

	inline bool hasDictionaryKey(hashcode_t key) const
	{
		return d->valueDictionary.find(key) != d->valueDictionary.end();
	}

	inline size_t vectorSize() const
	{
		return d->valueVector.size();
	}

	inline size_t dictionarySize(const Token *token) const
	{
		return getDictionary(token).size();
	}

	inline const std::string toString(const Token *token) const
	{
		switch (d->type)
		{
		case NIL:
			return "nil";
		case NUMBER:
			return d->valueNumber.toString();
		case STRING:
			return "\"" + d->valueString + "\"";
		case FUNCTION:
		{
			std::string ret = "<Function:{";
			size_t i = 0;
			for (auto &e : d->valueFunction)
			{
				for (auto &t : e.second)
				{
					if (i++ > 0)
						ret += ", ";
					ret += t.first.toString();
				}
			}
			return ret + "}>";
		}
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_TO_STRING))
				return o->getScope()->getVariable(Ruota::HASH_TO_STRING, token).call({}, this, token).getString(token);
			return "<Object>";
		}
		case POINTER:
			return "<Pointer>";
		case BOOLEAN_D:
			return d->valueBool ? "true" : "false";
		case ARRAY:
		{
			std::string ret = "[";
			unsigned int i = 0;
			for (auto &d2 : d->valueVector)
			{
				if (i > 0)
					ret += ", ";
				ret += d2.toString(token);
				i++;
			}
			return ret + "]";
		}
		case DICTIONARY:
		{
			std::string ret = "{";
			unsigned int i = 0;
			for (auto &e : getDictionary(token))
			{
				if (i > 0)
					ret += ", ";
				ret += "\"" + MAIN_HASH.deHash(e.first) + "\" : " + e.second.toString(token);
				i++;
			}
			return ret + "}";
		}
		case TYPE_NAME:
		{
			switch (d->valueType)
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
			case ARRAY:
				return "Type::Array";
			case DICTIONARY:
				return "Type::Dictionary";
			case TYPE_NAME:
				return "Type::Type";
			case POINTER:
				return "Type::Pointer";
			default:
				return "Type::@" + MAIN_HASH.deHash(d->valueType);
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
			return std::string("Symbol()");
		case NUMBER:
		{
			std::string ret = "Symbol(NUMBER_NEW_";
			if (d->valueNumber.type == CNumber::DOUBLE_NUM)
				ret += "DOUBLE(" + std::to_string(d->valueNumber.getDouble()) + ")";
			else
				ret += "LONG(" + std::to_string(d->valueNumber.getLong()) + ")";
			return ret + ")";
		}
		case STRING:
			return "Symbol(\"" + d->valueString + "\")";
		case FUNCTION:
			return std::string("<Function>");
		case OBJECT:
			return std::string("<Object>");
		case BOOLEAN_D:
		{
			std::string ret = "Symbol(";
			ret += (d->valueBool ? "true" : "false");
			return ret + ")";
		}
		case ARRAY:
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
			return ret + "})";
		}
		case DICTIONARY:
		{
			std::string ret = "Symbol({";
			unsigned int i = 0;
			for (auto &e : d->valueDictionary)
			{
				if (i > 0)
					ret += ", ";
				ret += "{" + std::to_string(e.first) + ", " + e.second.toCodeString() + "}";
				i++;
			}
			return ret + "})";
		}
		case TYPE_NAME:
			return std::string("Symbol(static_cast<TYPE_NAME>(") + std::to_string(d->valueType) + "))";
		default:
			return "undefined";
		}
	}

	inline const Symbol call(const std::vector<Symbol> &params, const Symbol *b, const Token *token) const
	{
		return getFunction(params, token)->evaluate(params, b, token);
	}

	inline void addFunctions(const Symbol *b, const Token *token) const
	{
		auto fs = b->d->valueFunction;
		for (auto &f : fs)
			for (auto &t : f.second)
				d->valueFunction[f.first][t.first] = t.second;
	}

	inline void set(const Symbol *b, const Token *token, const bool &isConst) const
	{
		if (b->d == d)
			return;
		if (d->type == OBJECT && d->valueObject != nullptr && d->valueObject->hasValue(Ruota::HASH_SET))
		{
			d->valueObject->getScope()->getVariable(Ruota::HASH_SET, token).call({*b}, this, token);
			return;
		}
		d->clearData();
		d->type = b->d->type;
		switch (d->type)
		{
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
			if (isConst)
			{
				d->valueVector = v;
				break;
			}
			d->valueVector.resize(v.size());
			for (size_t i = 0; i < v.size(); i++)
				d->valueVector[i].set(&v[i], token, isConst);
			break;
		}
		case DICTIONARY:
		{
			auto v = b->d->valueDictionary;
			if (isConst)
			{
				d->valueDictionary = v;
				break;
			}
			for (auto &e : v)
			{
				if (e.second.d->type == NIL)
					continue;
				auto newd = Symbol();
				newd.set(&e.second, token, isConst);
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

	inline bool equals(const Symbol *b, const Token *token) const
	{
		if (d->type != b->d->type && d->type != OBJECT)
			return false;
		switch (d->type)
		{
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
				return o->getScope()->getVariable(Ruota::HASH_EQUALS, token).call({*b}, this, token).d->valueBool;
			return o == b->d->valueObject;
		}
		case ARRAY:
		{
			auto bv = b->d->valueVector;
			if (d->valueVector.size() != bv.size())
				return false;
			for (unsigned long i = 0; i < d->valueVector.size(); i++)
				if (!d->valueVector[i].equals(&bv[i], token))
					return false;
			return true;
		}
		case DICTIONARY:
			for (auto &e : d->valueDictionary)
			{
				if (!e.second.equals(&b->d->valueDictionary[e.first], token))
					return false;
			}
			return true;
		case TYPE_NAME:
			return d->valueType == b->d->valueType;
		default:
			return false;
		}
	}

	inline bool nequals(const Symbol *b, const Token *token) const
	{
		switch (d->type)
		{
		case OBJECT:
		{
			auto o = d->valueObject;
			if (o->hasValue(Ruota::HASH_NEQUALS))
				return o->getScope()->getVariable(Ruota::HASH_NEQUALS, token).call({*b}, this, token).d->valueBool;
		}
		default:
			return !this->equals(b, token);
		}
	}

	inline bool pureEquals(const Symbol *b, const Token *token) const
	{
		switch (d->type)
		{
		case OBJECT:
			return d->valueObject == b->d->valueObject;
		default:
			return this->equals(b, token);
		}
	}

	inline bool pureNEquals(const Symbol *b, const Token *token) const
	{
		return !this->pureEquals(b, token);
	}

	inline bool operator==(const Symbol &b) const
	{
		return this->equals(&b, NULL);
	}

	inline bool operator!=(const Symbol &b) const
	{
		return this->nequals(&b, NULL);
	}

	inline bool operator<(const Symbol &b) const
	{
		return this->toCodeString() < b.toCodeString();
	}
};

#endif