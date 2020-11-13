#pragma once

#include "Locale.h"
#include "Declarations.h"
#include "CNumber.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace ruota
{
	typedef std::vector<type_sll> sig_t;

	inline const std::string getTypeString(const type_sll &i)
	{
		if (i >= 0)
			return "@" + RUOTA_DEHASH(i);
		else {
			switch (i) {
				case NIL:
					return "Nil";
				case NUMBER:
					return "Number";
				case STRING:
					return "String";
				case BOOLEAN_D:
					return "Boolean";
				case ARRAY:
					return "Array";
				case FUNCTION:
					return "Function";
				case DICTIONARY:
					return "Dictionary";
				case OBJECT:
					return "Object";
				case POINTER:
					return "Pointer";
				case TYPE_NAME:
					return "Type";
				default:
					return "<error-type>";
			}
		}
	}

	namespace sig
	{
		const size_t validity(const sig_t &, const std::vector<Symbol> &, std::vector<Function> &stack_trace);
		const std::string toString(const sig_t &);
	}

	struct Token
	{
		boost::filesystem::path filename;
		std::string line;
		size_t lineNumber;
		size_t distance;
		std::string valueString;
		CNumber valueNumber;
		int type = NULL_TOK;
	};

	class RTError : public std::runtime_error
	{
	private:
		const Token token;
		const std::vector<Function> stack_trace;

	public:
		RTError(const std::string &error, const Token &token, const std::vector<Function> &stack_trace) : std::runtime_error(error), token(token), stack_trace(stack_trace)
		{}

		const Token &getToken() const
		{
			return token;
		}

		const std::vector<Function> &getTrace() const
		{
			return stack_trace;
		}
	};

	class Instruction
	{
	protected:
		const InstructionType type;
		const Token token;

	public:
		Instruction(const InstructionType &, const Token &);
		virtual const Symbol evaluate(Scope *, std::vector<Function> &) const = 0;
		InstructionType getType() const;
		virtual ~Instruction();
	};

	class Function : public std::enable_shared_from_this<Function>
	{
	private:
		const hash_ull key;
		Scope *parent;
		const std::vector<std::pair<LexerTokenType, hash_ull>> params;
		const std::shared_ptr<Instruction> body;

	public:
		Function(const hash_ull &, Scope *, const std::vector<std::pair<LexerTokenType, hash_ull>> &, const std::shared_ptr<Instruction> &);
		const Symbol evaluate(const std::vector<Symbol> &, const Token *, std::vector<Function> &) const;
		const size_t getArgSize() const;
		const hash_ull getKey() const;
		Scope *getParent() const;
		const std::vector<std::pair<LexerTokenType, hash_ull>> &getParams() const;
	};

	class Scope : public std::enable_shared_from_this<Scope>
	{
	private:
		Scope *parent;
		const ObjectType type;
		const std::shared_ptr<Instruction> body;
		hash_ull hashed_key;
		std::vector<type_sll> name_trace;
		std::vector<type_sll> extensions;
		std::map<hash_ull, Symbol> values;
		void traceName(const hash_ull &);

	public:
		Scope();
		Scope(Scope *, const hash_ull &);
		Scope(Scope *, const ObjectType &, const std::shared_ptr<Instruction> &, const hash_ull &, const Scope *, const std::vector<type_sll> &);
		Scope(Scope *, const ObjectType &, const std::shared_ptr<Instruction> &, const hash_ull &, const std::vector<type_sll> &);
		Scope *getParent() const;
		const Symbol instantiate(const std::vector<Symbol> &, const Token *, std::vector<Function> &) const;
		void clear();
		const Symbol getThis(const Token *, std::vector<Function> &);
		const bool extendsObject(const type_sll &) const;
		const ObjectType getType() const;
		const std::shared_ptr<Instruction> getBody() const;
		const Symbol &getVariable(const hash_ull &, const Token *, std::vector<Function> &stack_trace) const;
		const Symbol &createVariable(const hash_ull &, const Token *);
		const Symbol &createVariable(const hash_ull &, const Symbol &, const Token *);
		const hash_ull getHashedKey() const;
		const bool hasValue(const hash_ull &) const;

		~Scope();
	};

	class Node
	{
	protected:
		const NodeType type;
		const Token token;

	public:
		Node(const NodeType &, const Token &);
		const NodeType getType() const;
		const Token getToken() const;

		virtual std::shared_ptr<Instruction> genParser() const = 0;
		virtual bool isConst() const = 0;
		virtual std::stringstream printTree(std::string, bool) const = 0;
		virtual std::shared_ptr<Node> fold() const = 0;
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

	public:
		Lexer();
		std::vector<Token> lexString(const std::string &, const boost::filesystem::path &);
	};

	class Ruota
	{
	private:
		Scope main;

	public:
		static const std::map<std::string, signed int> bOperators;
		static const std::map<std::string, signed int> uOperators;

		static Lexer lexer;

		static const hash_ull HASH_THIS;
		static const hash_ull HASH_INIT;
		static const hash_ull HASH_DELETER;

		static const hash_ull HASH_ADD;
		static const hash_ull HASH_SUB;
		static const hash_ull HASH_MUL;
		static const hash_ull HASH_DIV;
		static const hash_ull HASH_MOD;
		static const hash_ull HASH_POW;
		static const hash_ull HASH_B_AND;
		static const hash_ull HASH_B_OR;
		static const hash_ull HASH_B_XOR;
		static const hash_ull HASH_B_SH_L;
		static const hash_ull HASH_B_SH_R;
		static const hash_ull HASH_LESS;
		static const hash_ull HASH_MORE;
		static const hash_ull HASH_ELESS;
		static const hash_ull HASH_EMORE;
		static const hash_ull HASH_INDEX;
		static const hash_ull HASH_EQUALS;
		static const hash_ull HASH_NEQUALS;
		static const hash_ull HASH_SET;
		static const hash_ull HASH_CALL;
		static const hash_ull HASH_RANGE;

		static const hash_ull HASH_TO_STRING;
		static const hash_ull HASH_TO_NUMBER;
		static const hash_ull HASH_TO_BOOLEAN;
		static const hash_ull HASH_TO_VECTOR;
		static const hash_ull HASH_TO_DICTIONARY;

		Ruota(std::vector<std::string>);
		std::shared_ptr<Node> compileCode(const std::string &, boost::filesystem::path) const;
		const Symbol runCode(std::shared_ptr<Node>, bool);
		static void printError(const RTError &);
	};

	class Value
	{
		friend class Symbol;

	private:
		ValueType type;

		union
		{
			type_sll valueType;
			bool valueBool;
			CNumber valueNumber;
		};

		std::string valueString;
		std::shared_ptr<void> valuePointer;
		std::vector<Symbol> valueVector;
		std::unordered_map<size_t, std::map<sig_t, std::shared_ptr<const Function>>> valueFunction;
		sym_map_t valueDictionary;
		std::shared_ptr<Scope> valueObject;
		refc_ull references = 1;

		Value() : type(NIL)
		{}
		Value(const type_sll &valueType) : type(TYPE_NAME), valueType(valueType)
		{}
		Value(const bool &valueBool) : type(BOOLEAN_D), valueBool(valueBool)
		{}
		Value(const std::shared_ptr<void> &valuePointer) : type(POINTER), valuePointer(valuePointer)
		{}
		Value(const std::shared_ptr<Scope> &valueObject) : type(OBJECT), valueObject(valueObject)
		{}
		Value(const sig_t &ftype, const std::shared_ptr<const Function> &function) : type(FUNCTION), valueFunction({ {function->getArgSize(), {{ftype, function}}} })
		{}
		Value(const CNumber &valueNumber) : type(NUMBER), valueNumber(valueNumber)
		{}
		Value(const std::vector<Symbol> &valueVector) : type(ARRAY), valueVector(valueVector)
		{}
		Value(const sym_map_t &valueDictionary) : type(DICTIONARY), valueDictionary(valueDictionary)
		{}
		Value(const std::string &valueString) : type(STRING), valueString(valueString)
		{}

		inline void clearData()
		{
			switch (type) {
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
		Symbol() : type(ID_CASUAL), d(new Value())
		{}

		Symbol(const SymbolType &type) : type(type), d(new Value())
		{}

		Symbol(const std::shared_ptr<void> &valuePointer) : type(ID_CASUAL), d(new Value(valuePointer))
		{}

		Symbol(const type_sll &valueType) : type(ID_CASUAL), d(new Value(valueType))
		{}

		Symbol(const CNumber &valueNumber) : type(ID_CASUAL), d(new Value(valueNumber))
		{}

		Symbol(const bool &valueBool) : type(ID_CASUAL), d(new Value(valueBool))
		{}

		Symbol(const std::vector<Symbol> &valueVector) : type(ID_CASUAL), d(new Value(valueVector))
		{}

		Symbol(const std::shared_ptr<Scope> &valueObject) : type(ID_CASUAL), d(new Value(valueObject))
		{}

		Symbol(const sig_t &ftype, const std::shared_ptr<const Function> &valueFunction) : type(ID_CASUAL), d(new Value(ftype, valueFunction))
		{}

		Symbol(const std::string &valueString) : type(ID_CASUAL), d(new Value(valueString))
		{}

		Symbol(const sym_map_t &valueDictionary) : type(ID_CASUAL), d(new Value(valueDictionary))
		{}

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

		static inline const Symbol allocate(const size_t &size)
		{
			Symbol s;
			s.d->type = ARRAY;
			s.d->valueVector.resize(size);
			return s;
		}

		inline const SymbolType getSymbolType() const
		{
			return type;
		}

		inline void setSymbolType(const SymbolType &type)
		{
			this->type = type;
		}

		inline const CNumber &getNumber(const Token *token, std::vector<Function> &stack_trace) const
		{
			if (d->type != NUMBER)
				throw RTError(_NOT_NUMBER_, *token, stack_trace);
			return d->valueNumber;
		}

		inline void *getPointer(const Token *token, std::vector<Function> &stack_trace) const
		{
			if (d->type != POINTER)
				throw RTError(_NOT_POINTER_, *token, stack_trace);
			return d->valuePointer.get();
		}

		inline const sym_map_t &getDictionary(const Token *token, std::vector<Function> &stack_trace) const
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

		inline const Symbol &indexVector(const size_t &i, const Token *token, std::vector<Function> &stack_trace) const
		{
			if (i >= d->valueVector.size())
				throw RTError((boost::format(_INDEX_OUT_OF_BOUNDS_) % d->valueVector.size() % i).str(), *token, stack_trace);
			return d->valueVector[i];
		}

		inline const std::vector<Symbol> &getVector(const Token *token, std::vector<Function> &stack_trace) const
		{
			if (d->type != ARRAY)
				throw RTError(_NOT_VECTOR_, *token, stack_trace);
			return d->valueVector;
		}

		inline const std::string &getString(const Token *token, std::vector<Function> &stack_trace) const
		{
			if (d->type != STRING)
				throw RTError(_NOT_STRING_, *token, stack_trace);
			return d->valueString;
		}

		inline const bool getBool(const Token *token, std::vector<Function> &stack_trace) const
		{
			if (d->type != BOOLEAN_D)
				throw RTError(_NOT_BOOLEAN_, *token, stack_trace);
			return d->valueBool;
		}

		inline Scope *getObject(const Token *token, std::vector<Function> &stack_trace) const
		{
			if (d->type != OBJECT)
				throw RTError(_NOT_OBJECT_, *token, stack_trace);
			return d->valueObject.get();
		}

		inline const ValueType getValueType() const
		{
			return d->type;
		}

		inline const type_sll getAugValueType() const
		{
			if (d->type == OBJECT)
				return d->valueObject->getHashedKey();
			return d->type;
		}

		inline const type_sll getTypeName(const Token *token, std::vector<Function> &stack_trace) const
		{
			if (d->type != TYPE_NAME)
				throw RTError(_NOT_TYPE_, *token, stack_trace);
			return d->valueType;
		}

		inline const std::shared_ptr<const Function> &getFunction(const std::vector<Symbol> &params, const Token *token, std::vector<Function> &stack_trace) const
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

		inline const Symbol &indexDict(const std::string &key) const
		{
			return d->valueDictionary[key];
		}

		inline const bool hasDictionaryKey(const std::string &key) const
		{
			return d->valueDictionary.find(key) != d->valueDictionary.end();
		}

		inline const size_t vectorSize() const
		{
			return d->valueVector.size();
		}

		inline const size_t dictionarySize(const Token *token, std::vector<Function> &stack_trace) const
		{
			return getDictionary(token, stack_trace).size();
		}

		inline const std::string toString(const Token *token, std::vector<Function> &stack_trace) const
		{
			switch (d->type) {
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
					return d->valueBool ? "true" : "false";
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

		inline const std::string toCodeString() const
		{
			if (type == ID_BREAK)
				return "<BREAK>";
			std::vector<Function> stack_trace;
			return getTypeString(getAugValueType()) + "::" + toString(NULL, stack_trace);
		}

		inline const Symbol call(const std::vector<Symbol> &params, const Token *token, std::vector<Function> &stack_trace) const
		{
			return getFunction(params, token, stack_trace)->evaluate(params, token, stack_trace);
		}

		inline void addFunctions(const Symbol *b, const Token *token) const
		{
			auto fs = b->d->valueFunction;
			for (auto &f : fs)
				for (auto &t : f.second)
					d->valueFunction[f.first][t.first] = t.second;
		}

		inline void set(const Symbol *b, const Token *token, const bool &isConst, std::vector<Function> &stack_trace) const
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

		inline const bool equals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
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

		inline const bool nequals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
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

		inline const bool pureEquals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
		{
			switch (d->type) {
				case OBJECT:
					return d->valueObject == b->d->valueObject;
				default:
					return this->equals(b, token, stack_trace);
			}
		}

		inline const bool pureNEquals(const Symbol *b, const Token *token, std::vector<Function> &stack_trace) const
		{
			return !this->pureEquals(b, token, stack_trace);
		}

		inline const bool operator==(const Symbol &b) const
		{
			std::vector<Function> stack_trace;
			return this->equals(&b, NULL, stack_trace);
		}

		inline const bool operator!=(const Symbol &b) const
		{
			std::vector<Function> stack_trace;
			return this->nequals(&b, NULL, stack_trace);
		}

		inline const bool operator<(const Symbol &b) const
		{
			return this->toCodeString() < b.toCodeString();
		}
	};
}