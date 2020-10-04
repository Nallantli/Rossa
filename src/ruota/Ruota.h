#ifndef RUOTA_H
#define RUOTA_H

#include "Declarations.h"
#include "CNumber.h"

#include <memory>
#include <map>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

struct Signature
{
	std::vector<ValueType> values;
	Signature(std::vector<ValueType>);
	size_t validity(std::vector<ValueType>) const;
	bool operator<(const Signature &) const;
	const std::string toString() const;
};

class Token
{
private:
	std::string filename;
	std::string valueString;
	std::string line;
	CNumber valueNumber;
	int type;
	size_t distance;
	size_t lineNumber;

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
	Token token;

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
	InstructionType type;
	const Token token;

public:
	Instruction(InstructionType, const Token);
	virtual const Symbol evaluate(Scope *) const = 0;
	InstructionType getType() const;
	virtual ~Instruction();
};

class Function
{
private:
	std::vector<std::pair<LexerTokenType, hashcode_t>> params;
	std::shared_ptr<Instruction> body;
	Scope *parent;
	hashcode_t key;

public:
	Function(hashcode_t, Scope *, std::vector<std::pair<LexerTokenType, hashcode_t>>, std::shared_ptr<Instruction>);
	const Symbol evaluate(std::vector<Symbol> &, const Symbol *, const Token *) const;
	size_t getArgSize() const;
	hashcode_t getKey() const;
	Scope *getParent() const;
	std::vector<std::pair<LexerTokenType, hashcode_t>> getParams() const;
};

class Scope
{
private:
	std::string name;
	Scope *parent;
	std::map<hashcode_t, Symbol> values;

public:
	Scope();
	Scope(Scope *, const std::string &);
	Scope *getParent() const;
	Symbol &getVariable(hashcode_t, const Token *);
	Symbol &createVariable(hashcode_t, const Token *);
	Symbol &createVariable(hashcode_t, const Symbol &, const Token *);
	const std::string &getName() const;
	bool hasValue(hashcode_t) const;

	~Scope();
};

class Object
{
private:
	std::string key;
	ObjectType type;
	std::shared_ptr<Scope> internal;
	std::shared_ptr<Instruction> body;

public:
	Object(Scope *, ObjectType, std::shared_ptr<Instruction>, const std::string &);
	Scope *getScope() const;
	const Symbol instantiate(std::vector<Symbol> &, const Token *) const;
	ObjectType getType() const;
	std::shared_ptr<Instruction> getBody() const;
	const std::string &getName() const;
	bool hasValue(hashcode_t) const;

	~Object();
};

class Node
{
protected:
	NodeType type;
	const Token token;

public:
	Node(NodeType, const Token);
	NodeType getType() const;
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

	int getToken();
	char peekChar(size_t) const;
	char nextChar();
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
		signed long long valueType;
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
	Value(signed long long valueType) : type(TYPE_NAME), valueType(valueType) {}
	Value(bool valueBool) : type(BOOLEAN_D), valueBool(valueBool) {}
	Value(std::shared_ptr<void> valuePointer) : type(POINTER), valuePointer(valuePointer) {}
	Value(std::shared_ptr<Object> &valueObject) : type(OBJECT), valueObject(valueObject) {}
	Value(Signature ftype, std::shared_ptr<Function> &function) : type(FUNCTION), valueFunction({{function->getArgSize(), {{ftype, function}}}}) {}
	Value(CNumber valueNumber) : type(NUMBER), valueNumber(valueNumber) {}
	Value(std::vector<Symbol> valueVector) : type(VECTOR), valueVector(valueVector) {}
	Value(std::map<hashcode_t, Symbol> valueDictionary) : type(DICTIONARY), valueDictionary(valueDictionary) {}
	Value(const std::string &valueString) : type(STRING), valueString(valueString) {}
};

class Symbol
{
private:
	SymbolType type;
	bool isMutable;
	Value *d;

public:
	Symbol() : type(ID_CASUAL), isMutable(true), d(new Value()) {}

	Symbol(SymbolType type) : type(type), isMutable(false), d(new Value()) {}

	Symbol(std::shared_ptr<void> valuePointer) : type(ID_CASUAL), isMutable(true), d(new Value(valuePointer)) {}

	Symbol(signed long long valueType) : type(ID_CASUAL), isMutable(true), d(new Value(valueType)) {}

	Symbol(CNumber valueNumber) : type(ID_CASUAL), isMutable(true), d(new Value(valueNumber)) {}

	Symbol(bool valueBool) : type(ID_CASUAL), isMutable(true), d(new Value(valueBool)) {}

	Symbol(std::vector<Symbol> valueVector) : type(ID_CASUAL), isMutable(true), d(new Value(valueVector)) {}

	Symbol(std::shared_ptr<Object> &valueObject) : type(ID_CASUAL), isMutable(true), d(new Value(valueObject)) {}

	Symbol(Signature ftype, std::shared_ptr<Function> &valueFunction) : type(ID_CASUAL), isMutable(true), d(new Value(ftype, valueFunction)) {}

	Symbol(const std::string &valueString) : type(ID_CASUAL), isMutable(true), d(new Value(valueString)) {}

	Symbol(const std::map<hashcode_t, Symbol> valueDictionary) : type(ID_CASUAL), isMutable(true), d(new Value(valueDictionary)) {}

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

	inline void setMutable(bool isMutable)
	{
		this->isMutable = isMutable;
	}

	inline bool canSet() const
	{
		return isMutable;
	}

	inline SymbolType getSymbolType() const
	{
		return type;
	}

	inline void setSymbolType(SymbolType type)
	{
		this->type = type;
	}

	inline CNumber getNumber(const Token *token) const
	{
		if (d->type != NUMBER)
			throw RuotaError(_NOT_NUMBER_, *token);
		return d->valueNumber;
	}

	inline std::shared_ptr<void> getPointer(const Token *token) const
	{
		if (d->type != POINTER)
			throw RuotaError(_NOT_POINTER_, *token);
		return d->valuePointer;
	}

	inline std::map<hashcode_t, Symbol> getDictionary(const Token *token) const
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

	inline Symbol &indexVector(size_t i, const Token *token) const
	{
		if (i >= d->valueVector.size())
			throw RuotaError((boost::format(_INDEX_OUT_OF_BOUNDS_) % d->valueVector.size() % i).str(), *token);
		return d->valueVector[i];
	}

	inline std::vector<Symbol> getVector(const Token *token) const
	{
		if (d->type != VECTOR)
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

	inline std::shared_ptr<Object> getObject(const Token *token) const
	{
		if (d->type != OBJECT)
			throw RuotaError(_NOT_OBJECT_, *token);
		return d->valueObject;
	}

	inline ValueType getValueType() const
	{
		return d->type;
	}

	inline signed long long getTypeName(const Token *token) const
	{
		if (d->type != TYPE_NAME)
			throw RuotaError(_NOT_TYPE_, *token);
		return d->valueType;
	}

	inline std::shared_ptr<Function> getFunction(std::vector<ValueType> ftypes, const Token *token) const
	{
		if (d->type != FUNCTION)
			throw RuotaError(_NOT_FUNCTION_, *token);

		if (d->valueFunction.find(ftypes.size()) == d->valueFunction.end())
			throw RuotaError(_FUNCTION_ARG_SIZE_FAILURE_, *token);

		std::map<Signature, std::shared_ptr<Function>> foftype = d->valueFunction[ftypes.size()];
		std::shared_ptr<Function> f = nullptr;
		size_t cur_v = 0;
		for (auto &f2 : foftype)
		{
			size_t v = f2.first.validity(ftypes);
			if (v > cur_v)
			{
				cur_v = v;
				f = f2.second;
				if (v == ftypes.size() * 2)
					break;
			}
		}

		if (!f)
			throw RuotaError(_FUNCTION_VALUE_NOT_EXIST_, *token);

		return f;
	}

	inline Symbol &indexDict(hashcode_t i) const
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
		case VECTOR:
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
				ret += "\"" + hash.deHash(e.first) + "\" : " + e.second.toString(token);
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
			case VECTOR:
				return "Type::Vector";
			case DICTIONARY:
				return "Type::Dictionary";
			case TYPE_NAME:
				return "Type::Type";
			case POINTER:
				return "Type::Pointer";
			default:
				return "Type::@" + hash.deHash(d->valueType);
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
			if (d->valueNumber.getType() == CNumber::DOUBLE_NUM)
				ret += "DOUBLE(" + std::to_string(d->valueNumber.getDouble()) + ")" + (!isMutable ? ".setMutable(false)" : "");
			else
				ret += "LONG(" + std::to_string(d->valueNumber.getLong()) + ")" + (!isMutable ? ".setMutable(false)" : "");
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
			for (auto &e : d->valueDictionary)
			{
				if (i > 0)
					ret += ", ";
				ret += "{" + std::to_string(e.first) + ", " + e.second.toCodeString() + "}";
				i++;
			}
			return ret + "})" + (!isMutable ? ".setMutable(false)" : "");
		}
		case TYPE_NAME:
			return std::string("Symbol(static_cast<TYPE_NAME>(") + std::to_string(d->valueType) + "))" + (!isMutable ? ".setMutable(false)" : "");
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

	inline Symbol call(std::vector<Symbol> params, const Symbol *b, const Token *token) const
	{
		std::vector<ValueType> ftypes;
		for (auto &e : params)
			ftypes.push_back(e.getValueType());

		auto f = getFunction(ftypes, token);

		return f->evaluate(params, b, token);
	}

	inline void addFunctions(const Symbol *b, const Token *token) const
	{
		auto fs = b->d->valueFunction;
		for (auto &f : fs)
			for (auto &t : f.second)
				d->valueFunction[f.first][t.first] = t.second;
	}

	inline void set(Symbol *b, const Token *token) const
	{
		if (!isMutable)
			throw RuotaError("Cannot change the value of a variable declared as `final`", *token);
		if (d->type == OBJECT && d->valueObject != nullptr && d->valueObject->hasValue(Ruota::HASH_SET))
		{
			d->valueObject->getScope()->getVariable(Ruota::HASH_SET, token).call({*b}, this, token);
			return;
		}
		d->type = b->d->type;
		clearData();
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
		case VECTOR:
		{
			auto v = b->d->valueVector;
			for (unsigned long i = 0; i < v.size(); i++)
			{
				auto newd = Symbol();
				newd.set(&v[i], token);
				d->valueVector.push_back(newd);
			}
			break;
		}
		case DICTIONARY:
		{
			auto v = b->d->valueDictionary;
			for (auto &e : v)
			{

				if (e.second.d->type == NIL)
					continue;
				auto newd = Symbol();
				newd.set(&e.second, token);
				d->valueDictionary[e.first] = newd;
			}
			break;
		}
		case TYPE_NAME:
			d->valueType = b->d->valueType;
		default:
			break;
		}
	}

	inline bool equals(Symbol *b, const Token *token) const
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
		case VECTOR:
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

	inline bool nequals(Symbol *b, const Token *token) const
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

	inline bool pureEquals(Symbol *b, const Token *token) const
	{
		switch (d->type)
		{
		case OBJECT:
			return d->valueObject == b->d->valueObject;
		default:
			return this->equals(b, token);
		}
	}

	inline bool pureNEquals(Symbol *b, const Token *token) const
	{
		return !this->pureEquals(b, token);
	}

	inline bool operator<(const Symbol &b) const
	{
		return this->toCodeString() < b.toCodeString();
	}
};

#endif