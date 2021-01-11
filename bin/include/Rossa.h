#pragma once

#define _ROSSA_VERSION_ "v1.10.0-alpha"
#define COERCE_PTR(v, t) reinterpret_cast<t *>(v)

#define ROSSA_DEHASH(x) Rossa::MAIN_HASH.deHash(x)
#define ROSSA_HASH(x) Rossa::MAIN_HASH.hashValue(x)

#define colorASCII(c) "\033[" + std::to_string(c) + "m"
#define PRINTC(s, c) std::cout << colorASCII(c) << s << colorASCII(0)

#include "Locale.h"
#include "RNumber.h"
#include "Keywords.h"
#include "Compilation.h"
#include "Format.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>

#define ROSSA_EXT_SIG(name, args, token, hash, stack_trace) inline const Symbol name(const std::vector<Symbol> &args, const Token *token, Hash &hash, std::vector<Function> &stack_trace)
#define ADD_EXT(name) fmap[#name] = name

#ifndef _WIN32
#include <limits.h>
#include <unistd.h>
#include <dlfcn.h>
#define EXPORT_FUNCTIONS(name) extern "C" void name##_rossaExportFunctions(std::map<std::string, rossa::extf_t> &fmap)
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define EXPORT_FUNCTIONS(name) extern "C" __declspec(dllexport) void name##_rossaExportFunctions(std::map<std::string, rossa::extf_t> &fmap)
#endif

namespace rossa
{
	struct Token;
	struct Hash;

	class Instruction;
	class Function;
	class Scope;
	class Node;
	class NodeParser;
	class Rossa;
	class Value;
	class Symbol;
	class ParamType;

	typedef unsigned long long hash_ull;
	typedef unsigned long long refc_ull;
	typedef signed long long type_sll;
	typedef std::vector<ParamType> sig_t;
	typedef std::map<std::string, Symbol> sym_map_t;
	typedef const Symbol(*extf_t)(const std::vector<Symbol> &, const Token *, Hash &, std::vector<Function> &);
	typedef void (*export_fns_t)(std::map<std::string, extf_t> &);

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

	enum LexerTokenType
	{
		NULL_TOK = 0,

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
		TOK_ARRAY = -23,
		TOK_BOOLEAN = -24,
		TOK_EXTERN_CALL = -25,

		TOK_LENGTH = -26,
		TOK_ANY = -27,
		TOK_CLASS = -28,
		TOK_STRUCT = -29,
		TOK_STATIC = -30,
		TOK_NEW = -31,
		TOK_TYPE_NAME = -32,
		TOK_DICTIONARY = -33,
		TOK_OBJECT = -34,
		TOK_FUNCTION = -35,
		TOK_OF = -36,

		TOK_LOAD = -37,
		TOK_ALLOC = -38,
		TOK_INNER = -39,
		TOK_REF = -40,
		TOK_CASE = -41,
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
		TOK_CHARS = -53,
		TOK_LAMBDA = -54,
		TOK_UNTILT = -55,
		TOK_UNTILF = -56,
		TOK_PARSE = -57,
		TOK_CONTINUE = -58,
		TOK_CALL_OP = -59,
		TOK_NO_PARAM_LAMBDA = -60
	};

	enum SymbolType
	{
		ID_RETURN,
		ID_BREAK,
		ID_CONTINUE,
		ID_CASUAL,
		ID_REFER
	};

	enum ValueType
	{
		NIL = -1,
		NUMBER = -2,
		BOOLEAN_D = -3,
		STRING = -4,
		ARRAY = -5,
		FUNCTION = -6,
		DICTIONARY = -7,
		OBJECT = -8,
		TYPE_NAME = -9,
		POINTER = -10
	};

	enum InstructionType
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
		B_SH_R,
		DECLARE_VARS_I,
	TYPE_I,
	CALL_OP_I
	};

	enum ObjectType
	{
		SCOPE_O,
		STRUCT_O,
		STATIC_O,
		INSTANCE_O,
		VIRTUAL_O
	};

	enum NodeType
	{
		ENTRY_NODE,
		CALL_NODE,
		CALL_BUILT_NODE,
		UN_OP_NODE,
		BIN_OP_NODE,
		INS_NODE,
		IF_ELSE_NODE,
		WHILE_NODE,
		FOR_NODE,
		VECTOR_NODE,
		DEFINE_NODE,
		RETURN_NODE,
		ID_NODE,
		VAR_NODE,
		CLASS_NODE,
		EXTERN_CALL_NODE,
		NEW_NODE,
		CAST_TO_NODE,
		UNTIL_NODE,
		MAP_NODE,
		CONTAINER_NODE,
		BREAK_NODE,
		CONTINUE_NODE,
		REFER_NODE,
		SWITCH_NODE,
		BID_NODE,
		TRY_CATCH_NODE,
		THROW_NODE,
		PAREN_NODE,
		CALL_OP_NODE
	};

	struct Hash
	{
		std::vector<std::string> variable_hash;

		Hash()
		{
			variable_hash.push_back("<LAMBDA>");
		}

		inline const hash_ull hashValue(const std::string &key)
		{
			if (std::find(variable_hash.begin(), variable_hash.end(), key) != variable_hash.end())
				return std::distance(variable_hash.begin(), std::find(variable_hash.begin(), variable_hash.end(), key));
			variable_hash.push_back(key);
			return variable_hash.size() - 1;
		}

		inline const std::string deHash(const hash_ull &code) const
		{
			return variable_hash[code];
		}
	};

	namespace sig
	{
		const size_t validity(const sig_t &, const std::vector<Symbol> &, std::vector<Function> &stack_trace);
		const std::string toString(const sig_t &);
		const std::string getTypeString(const type_sll &);
		const std::string toCodeString(const sig_t &);
	}

	class ParamType
	{
	private:
		type_sll base;
		std::vector<ParamType> qualifiers;
	public:
		ParamType(const type_sll &);
		ParamType(const type_sll &, const std::vector<ParamType> &);
		void addQualifier(const ParamType &);
		const std::vector<ParamType> getQualifiers() const;
		const type_sll getBase() const;
		const std::string toString() const;
		const std::string toCodeString() const;
		const bool operator<(const ParamType &) const;
		const size_t operator&(const ParamType &) const;
	};

	struct Token
	{
		std::filesystem::path filename;
		std::string line;
		size_t lineNumber;
		size_t distance;
		std::string valueString;
		RNumber valueNumber;
		int type = NULL_TOK;
	};

	class RTError : public std::runtime_error
	{
	private:
		const Token token;
		const std::vector<Function> stack_trace;

	public:
		RTError(const std::string &, const Token &, const std::vector<Function> &);
		const Token &getToken() const;
		const std::vector<Function> &getTrace() const;
	};

	class Instruction
	{
	protected:
		const InstructionType type;
		const Token token;

	public:
		Instruction(const InstructionType &, const Token &);
		virtual const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const = 0;
		InstructionType getType() const;
		virtual const std::string compile() const = 0;
		virtual ~Instruction();
	};

	class Function : public std::enable_shared_from_this<Function>
	{
	private:
		const hash_ull key;
		const std::shared_ptr<Scope> parent;
		const std::vector<std::pair<LexerTokenType, hash_ull>> params;
		const std::shared_ptr<Instruction> body;
		const std::map<hash_ull, Symbol> captures;

	public:
		Function(const hash_ull &, const std::shared_ptr<Scope> &, const std::vector<std::pair<LexerTokenType, hash_ull>> &, const std::shared_ptr<Instruction> &, const std::map<hash_ull, Symbol> &);
		const Symbol evaluate(const std::vector<Symbol> &, const Token *, std::vector<Function> &) const;
		const size_t getArgSize() const;
		const hash_ull getKey() const;
		const std::shared_ptr<Scope> &getParent() const;
		const std::vector<std::pair<LexerTokenType, hash_ull>> &getParams() const;
	};

	class Scope : public std::enable_shared_from_this<Scope>
	{
	private:
		const std::shared_ptr<Scope> parent;
		const ObjectType type;
		const std::shared_ptr<Instruction> body;
		hash_ull hashed_key;
		std::vector<type_sll> name_trace;
		std::vector<type_sll> extensions;
		std::map<hash_ull, Symbol> values;
		void traceName(const hash_ull &);

	public:
		Scope();
		Scope(const std::shared_ptr<Scope> &, const hash_ull &);
		Scope(const std::shared_ptr<Scope> &, const ObjectType &, const std::shared_ptr<Instruction> &, const hash_ull &, const std::shared_ptr<Scope> &, const std::vector<type_sll> &);
		Scope(const std::shared_ptr<Scope> &, const ObjectType &, const std::shared_ptr<Instruction> &, const hash_ull &, const std::vector<type_sll> &);
		const std::shared_ptr<Scope> &getParent() const;
		const Symbol instantiate(const std::vector<Symbol> &, const Token *, std::vector<Function> &) const;
		void clear();
		const Symbol getThis(const Token *, std::vector<Function> &);
		const bool extendsObject(const type_sll &) const;
		const ObjectType getType() const;
		const std::shared_ptr<Instruction> getBody() const;
		const Symbol &getVariable(const hash_ull &, const Token *, std::vector<Function> &) const;
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

	class NodeParser
	{
	private:
		const std::vector<Token> tokens;
		const std::filesystem::path currentFile;

		unsigned int index = 0;
		Token currentToken;
		void nextToken();
		std::shared_ptr<Node> parseNumNode();
		std::shared_ptr<Node> parseBoolNode();
		std::shared_ptr<Node> parseIDNode();
		std::shared_ptr<Node> parseBIDNode();
		std::shared_ptr<Node> parseEntryNode();
		std::shared_ptr<Node> parseExprNode();
		std::shared_ptr<Node> parseEquNode();
		std::shared_ptr<Node> parseVectorNode();
		std::shared_ptr<Node> parseUnitNode();
		std::shared_ptr<Node> parseBaseNode();
		std::shared_ptr<Node> parseUnOpNode();
		std::shared_ptr<Node> parseMapNode();
		std::shared_ptr<Node> parseIfElseNode();
		std::shared_ptr<Node> parseWhileNode();
		std::shared_ptr<Node> parseForNode();
		std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> parseSigNode(const ValueType &start);
		std::shared_ptr<Node> parseDefineNode();
		std::shared_ptr<Node> parseLambdaNode();
		std::shared_ptr<Node> parseNPLambdaNode();
		std::shared_ptr<Node> parseExternNode();
		std::shared_ptr<Node> parseExternCallNode();
		std::shared_ptr<Node> parseCallOpNode();
		std::shared_ptr<Node> parseCallBuiltNode();
		std::shared_ptr<Node> parseClassNode();
		std::shared_ptr<Node> parseNewNode();
		std::shared_ptr<Node> parseLoadNode();
		std::shared_ptr<Node> parseSwitchNode();
		std::shared_ptr<Node> parseTryCatchNode();
		std::shared_ptr<Node> parseTypeNode();
		ParamType parseParamTypeNode(const type_sll &);
		std::shared_ptr<Node> parseTrailingNode(const std::shared_ptr<Node> &, const bool &);
		std::shared_ptr<Node> parseInsNode(const std::shared_ptr<Node> &);
		std::shared_ptr<Node> parseUntilNode(const std::shared_ptr<Node> &, const bool &);
		std::shared_ptr<Node> parseBinOpNode(const std::shared_ptr<Node> &);
		std::shared_ptr<Node> parseCallNode(const std::shared_ptr<Node> &);
		std::shared_ptr<Node> parseIndexNode(const std::shared_ptr<Node> &);
		std::shared_ptr<Node> parseThenNode(const std::shared_ptr<Node> &);

		std::shared_ptr<Node> logErrorN(const std::string &, const Token &);
		ParamType logErrorPT(const std::string &, const Token &);
		std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> logErrorSN(const std::string &, const Token &);

	public:
		NodeParser(const std::vector<Token> &, const std::filesystem::path &);
		std::shared_ptr<Node> parse();
		static std::shared_ptr<Instruction> genParser(const std::shared_ptr<Node> &);
	};

	class Rossa
	{
	private:
		static const int getToken(const std::string &, size_t &, size_t &, size_t &, std::string &, RNumber &);
		static const char peekChar(const size_t &, const std::string &, const size_t &);
		static const char nextChar(const std::string &, size_t &, size_t &, size_t &);

	public:
		std::shared_ptr<Scope> main;

		static const std::map<std::string, signed int> bOperators;
		static const std::map<std::string, signed int> uOperators;

		static Hash MAIN_HASH;

		static const hash_ull HASH_THIS;
		static const hash_ull HASH_BLANK;
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

		Rossa(const std::vector<std::string> &);
		static void loadStandardFunctions(std::map<std::string, extf_t> &fmap);
		std::shared_ptr<Node> compileCode(const std::string &, const std::filesystem::path &) const;
		const Symbol runCode(const std::shared_ptr<Node> &, const bool &);
		static void printError(const RTError &);
		static const std::vector<Token> lexString(const std::string &, const std::filesystem::path &);

		~Rossa();
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
			RNumber valueNumber;
		};

		std::string valueString;
		std::shared_ptr<void> valuePointer;
		std::vector<Symbol> valueVector;
		std::map<size_t, std::map<sig_t, std::shared_ptr<const Function>>> valueFunction;
		sym_map_t valueDictionary;
		std::shared_ptr<Scope> valueObject;
		refc_ull references = 1;

		Value();
		Value(const type_sll &);
		Value(const bool &);
		Value(const std::shared_ptr<void> &);
		Value(const std::shared_ptr<Scope> &);
		Value(const sig_t &, const std::shared_ptr<const Function> &);
		Value(const RNumber &);
		Value(const std::vector<Symbol> &);
		Value(const sym_map_t &);
		Value(const std::string &);
		void clearData();
	};

	class Symbol
	{
	private:
		SymbolType type;
		Value *d;

	public:
		Symbol();
		Symbol(const SymbolType &);
		Symbol(const std::shared_ptr<void> &);
		Symbol(const type_sll &);
		Symbol(const RNumber &);
		Symbol(const bool &);
		Symbol(const std::vector<Symbol> &);
		Symbol(const std::shared_ptr<Scope> &);
		Symbol(const sig_t &, const std::shared_ptr<const Function> &);
		Symbol(const std::string &);
		Symbol(const sym_map_t &);
		Symbol(const Symbol &);
		~Symbol();

		void operator=(const Symbol &);
		static const Symbol allocate(const size_t &);
		const SymbolType getSymbolType() const;
		void setSymbolType(const SymbolType &);
		const RNumber &getNumber(const Token *, std::vector<Function> &) const;
		void *getPointer(const Token *, std::vector<Function> &) const;
		const sym_map_t &getDictionary(const Token *, std::vector<Function> &) const;
		const Symbol &indexVector(const size_t &, const Token *, std::vector<Function> &) const;
		const std::vector<Symbol> &getVector(const Token *, std::vector<Function> &) const;
		const std::string &getString(const Token *, std::vector<Function> &) const;
		const bool getBool(const Token *, std::vector<Function> &) const;
		const std::shared_ptr<Scope> &getObject(const Token *, std::vector<Function> &) const;
		const ValueType getValueType() const;
		const type_sll getAugValueType() const;
		const type_sll getTypeName(const Token *, std::vector<Function> &) const;
		const std::shared_ptr<const Function> getFunction(const std::vector<Symbol> &, const Token *, std::vector<Function> &) const;
		const Symbol &indexDict(const std::string &) const;
		const bool hasDictionaryKey(const std::string &) const;
		const size_t vectorSize() const;
		const size_t dictionarySize(const Token *, std::vector<Function> &) const;
		const std::string toString(const Token *, std::vector<Function> &) const;
		const std::string toCodeString() const;
		const Symbol call(const std::vector<Symbol> &, const Token *, std::vector<Function> &) const;
		void addFunctions(const Symbol *, const Token *) const;
		void set(const Symbol *, const Token *, const bool &, std::vector<Function> &) const;
		const bool equals(const Symbol *, const Token *, std::vector<Function> &) const;
		const bool nequals(const Symbol *, const Token *, std::vector<Function> &) const;
		const bool pureEquals(const Symbol *, const Token *, std::vector<Function> &) const;
		const bool pureNEquals(const Symbol *, const Token *, std::vector<Function> &) const;
		const bool operator==(const Symbol &) const;
		const bool operator!=(const Symbol &) const;
		const bool operator<(const Symbol &) const;
		const std::map<size_t, std::map<sig_t, std::shared_ptr<const Function>>> getFunctionOverloads(const Token *token, std::vector<Function> &stack_trace) const;
	};

	// INSTRUCTIONS -----------------------------------------------------------------------------

	class UnaryI : public Instruction
	{
	protected:
		const std::shared_ptr<Instruction> a;

	public:
		UnaryI(const InstructionType &, const std::shared_ptr<Instruction> &, const Token &);
		const std::shared_ptr<Instruction> getA() const;
	};

	class CastingI : public Instruction
	{
	protected:
		const hash_ull key;

	public:
		CastingI(const InstructionType &, const hash_ull &, const Token &);
		const hash_ull getKey() const;
	};

	class BinaryI : public UnaryI
	{
	protected:
		const std::shared_ptr<Instruction> b;

	public:
		BinaryI(const InstructionType &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const std::shared_ptr<Instruction> getB() const;
	};

	class ContainerI : public Instruction
	{
	protected:
		const Symbol d;

	public:
		ContainerI(const Symbol &d, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class DefineI : public Instruction
	{
	protected:
		const hash_ull key;
		const sig_t ftype;
		const std::vector<std::pair<LexerTokenType, hash_ull>> params;
		const std::shared_ptr<Instruction> body;
		const std::vector<hash_ull> captures;

	public:
		DefineI(const hash_ull &, const sig_t &, const std::vector<std::pair<LexerTokenType, hash_ull>> &, const std::shared_ptr<Instruction> &, const std::vector<hash_ull> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class SequenceI : public Instruction
	{
	protected:
		const std::vector<std::shared_ptr<Instruction>> children;

	public:
		SequenceI(const std::vector<std::shared_ptr<Instruction>> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class IfElseI : public Instruction
	{
	protected:
		const std::shared_ptr<Instruction> ifs;
		const std::shared_ptr<Instruction> body;
		const std::shared_ptr<Instruction> elses;

	public:
		IfElseI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class WhileI : public Instruction
	{
	protected:
		const std::shared_ptr<Instruction> whiles;
		const std::shared_ptr<Instruction> body;

	public:
		WhileI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ForI : public Instruction
	{
	protected:
		const hash_ull id;
		const std::shared_ptr<Instruction> fors;
		const std::shared_ptr<Instruction> body;

	public:
		ForI(const hash_ull &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class VariableI : public CastingI
	{
	public:
		VariableI(const hash_ull &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class DeclareI : public CastingI
	{
	protected:
		const type_sll vtype;
		const std::shared_ptr<Instruction> a;
		const bool isConst;

	public:
		DeclareI(const hash_ull &, const type_sll &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class IndexI : public BinaryI
	{
	public:
		IndexI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class InnerI : public BinaryI
	{
	public:
		InnerI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class CallI : public BinaryI
	{
	public:
		CallI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class AddI : public BinaryI
	{
	public:
		AddI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class SubI : public BinaryI
	{
	public:
		SubI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class MulI : public BinaryI
	{
	public:
		MulI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class DivI : public BinaryI
	{
	public:
		DivI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ModI : public BinaryI
	{
	public:
		ModI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class PowI : public BinaryI
	{
	public:
		PowI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class LessI : public BinaryI
	{
	public:
		LessI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class MoreI : public BinaryI
	{
	public:
		MoreI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ELessI : public BinaryI
	{
	public:
		ELessI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class EMoreI : public BinaryI
	{
	public:
		EMoreI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class EqualsI : public BinaryI
	{
	public:
		EqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class NEqualsI : public BinaryI
	{
	public:
		NEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class AndI : public BinaryI
	{
	public:
		AndI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class OrI : public BinaryI
	{
	public:
		OrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class BOrI : public BinaryI
	{
	public:
		BOrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class BAndI : public BinaryI
	{
	public:
		BAndI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class BXOrI : public BinaryI
	{
	public:
		BXOrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class BShiftLeftI : public BinaryI
	{
	public:
		BShiftLeftI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class BShiftRightI : public BinaryI
	{
	public:
		BShiftRightI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class SetI : public BinaryI
	{
	protected:
		const bool isConst;

	public:
		SetI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ReturnI : public UnaryI
	{
	public:
		ReturnI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ExternI : public UnaryI
	{
	protected:
		const std::string libname;
		const std::string fname;
		extf_t f;

	public:
		ExternI(const std::string &, const std::string &, const std::shared_ptr<Instruction> &a, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class LengthI : public UnaryI
	{
	public:
		LengthI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ClassI : public Instruction
	{
	protected:
		const hash_ull key;
		const ObjectType type;
		const std::shared_ptr<Instruction> body;
		const std::shared_ptr<Instruction> extends;

	public:
		ClassI(const hash_ull &, const ObjectType &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class NewI : public BinaryI
	{
	public:
		NewI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class CastToI : public BinaryI
	{
	public:
		CastToI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class AllocI : public UnaryI
	{
	public:
		AllocI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class UntilI : public BinaryI
	{
	protected:
		const std::shared_ptr<Instruction> step;
		const bool inclusive;

	public:
		UntilI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ScopeI : public Instruction
	{
	protected:
		const std::vector<std::shared_ptr<Instruction>> children;

	public:
		ScopeI(const std::vector<std::shared_ptr<Instruction>> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class MapI : public Instruction
	{
	protected:
		const std::map<std::string, std::shared_ptr<Instruction>> children;

	public:
		MapI(const std::map<std::string, std::shared_ptr<Instruction>> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ReferI : public UnaryI
	{
	public:
		ReferI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class SwitchI : public Instruction
	{
	protected:
		const std::shared_ptr<Instruction> switchs;
		const std::map<Symbol, size_t> cases_solved;
		const std::map<std::shared_ptr<Instruction>, size_t> cases_unsolved;
		const std::vector<std::shared_ptr<Instruction>> cases;
		const std::shared_ptr<Instruction> elses;

	public:
		SwitchI(const std::shared_ptr<Instruction> &, const std::map<Symbol, size_t> &, const std::map<std::shared_ptr<Instruction>, size_t> &, const std::vector<std::shared_ptr<Instruction>> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class TryCatchI : public BinaryI
	{
	protected:
		const hash_ull key;

	public:
		TryCatchI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const hash_ull &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ThrowI : public UnaryI
	{
	public:
		ThrowI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class PureEqualsI : public BinaryI
	{
	public:
		PureEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class PureNEqualsI : public BinaryI
	{
	public:
		PureNEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class CharNI : public UnaryI
	{
	public:
		CharNI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class CharSI : public UnaryI
	{
	public:
		CharSI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class DeclareVarsI : public Instruction
	{
	protected:
		const std::vector<hash_ull> keys;

	public:
		DeclareVarsI(const std::vector<hash_ull> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class ParseI : public UnaryI
	{
	public:
		ParseI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class TypeI : public UnaryI
	{
	public:
		TypeI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	class CallOpI : public Instruction
	{
	protected:
		const size_t id;
		const std::vector<std::shared_ptr<Instruction>> children;

	public:
		CallOpI(const size_t &, const std::vector<std::shared_ptr<Instruction>> &, const Token &);
		const Symbol evaluate(const std::shared_ptr<Scope> &, std::vector<Function> &) const override;
		const std::string compile() const override;
	};

	// NODES -----------------------------------------------------------------------------

	class ContainerNode : public Node
	{
	private:
		const Symbol s;

	public:
		ContainerNode(const Symbol &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class VectorNode : public Node
	{
	private:
		const std::vector<std::shared_ptr<Node>> args;
		bool scoped;

	public:
		VectorNode(const std::vector<std::shared_ptr<Node>> &, const bool &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
		const std::vector<std::shared_ptr<Node>> &getChildren();
	};

	class BreakNode : public Node
	{
	public:
		BreakNode(const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ContinueNode : public Node
	{
	public:
		ContinueNode(const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class IDNode : public Node
	{
	private:
		const hash_ull key;

	public:
		IDNode(const hash_ull &, const Token &);
		hash_ull getKey() const;
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class BIDNode : public Node
	{
	private:
		const std::string key;

	public:
		BIDNode(const std::string &, const Token &);
		const std::string getKey() const;
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class DefineNode : public Node
	{
	private:
		const hash_ull key;
		const sig_t ftype;
		const std::vector<std::pair<LexerTokenType, hash_ull>> params;
		const std::shared_ptr<Node> body;
		const std::vector<hash_ull> captures;

	public:
		DefineNode(const hash_ull &, const sig_t &, const std::vector<std::pair<LexerTokenType, hash_ull>> &, const std::shared_ptr<Node> &, const std::vector<hash_ull> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class NewNode : public Node
	{
	private:
		const std::shared_ptr<Node> object;
		const std::shared_ptr<Node> params;

	public:
		NewNode(const std::shared_ptr<Node> &, const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ClassNode : public Node
	{
	private:
		const hash_ull key;
		const int type;
		const std::vector<std::shared_ptr<Node>> body;
		const std::shared_ptr<Node> extends;

	public:
		ClassNode(const hash_ull &, const int &, const std::vector<std::shared_ptr<Node>> &, const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class VarNode : public Node
	{
	private:
		const std::vector<hash_ull> keys;

	public:
		VarNode(const std::vector<hash_ull> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class CallNode : public Node
	{
	private:
		const std::shared_ptr<Node> callee;
		const std::vector<std::shared_ptr<Node>> args;

	public:
		CallNode(const std::shared_ptr<Node> &, const std::vector<std::shared_ptr<Node>> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		std::shared_ptr<Node> getCallee();
		std::vector<std::shared_ptr<Node>> getArgs();
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ExternCallNode : public Node
	{
	private:
		const std::string libname;
		const std::string fname;
		const std::vector<std::shared_ptr<Node>> args;

	public:
		ExternCallNode(const std::string &, const std::string &, const std::vector<std::shared_ptr<Node>> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class CallBuiltNode : public Node
	{
	private:
		const LexerTokenType t;
		const std::shared_ptr<Node> arg;

	public:
		CallBuiltNode(const LexerTokenType &, const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ReturnNode : public Node
	{
	private:
		const std::shared_ptr<Node> a;

	public:
		ReturnNode(const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ReferNode : public Node
	{
	private:
		const std::shared_ptr<Node> a;

	public:
		ReferNode(const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class BinOpNode : public Node
	{
	private:
		const std::string op;
		std::shared_ptr<Node> a;
		std::shared_ptr<Node> b;

	public:
		BinOpNode(const std::string &, const std::shared_ptr<Node> &, const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		const std::string &getOp() const;
		std::shared_ptr<Node> getA() const;
		std::shared_ptr<Node> getB() const;
		void setA(const std::shared_ptr<Node> &);
		void setB(const std::shared_ptr<Node> &);
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class UnOpNode : public Node
	{
	private:
		const std::string op;
		const std::shared_ptr<Node> a;

	public:
		UnOpNode(const std::string &, const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ParenNode : public Node
	{
	private:
		const std::shared_ptr<Node> a;

	public:
		ParenNode(const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class InsNode : public Node
	{
	private:
		const std::shared_ptr<Node> callee;
		const std::shared_ptr<Node> arg;

	public:
		InsNode(const std::shared_ptr<Node> &, const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		std::shared_ptr<Node> getCallee();
		std::shared_ptr<Node> getArg();
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class IfElseNode : public Node
	{
	private:
		const std::shared_ptr<Node> ifs;
		const std::shared_ptr<Node> body;
		std::shared_ptr<Node> elses = nullptr;

	public:
		IfElseNode(const std::shared_ptr<Node> &, const std::shared_ptr<Node> &, const Token &);
		void setElse(const std::shared_ptr<Node> &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class WhileNode : public Node
	{
	private:
		const std::shared_ptr<Node> whiles;
		const std::vector<std::shared_ptr<Node>> body;

	public:
		WhileNode(const std::shared_ptr<Node> &, const std::vector<std::shared_ptr<Node>> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ForNode : public Node
	{
	private:
		hash_ull id;
		std::shared_ptr<Node> fors;
		std::vector<std::shared_ptr<Node>> body;

	public:
		ForNode(const hash_ull &, const std::shared_ptr<Node> &, const std::vector<std::shared_ptr<Node>> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class UntilNode : public Node
	{
	private:
		const std::shared_ptr<Node> a;
		const std::shared_ptr<Node> b;
		const std::shared_ptr<Node> step;
		const bool inclusive;

	public:
		UntilNode(const std::shared_ptr<Node> &, const std::shared_ptr<Node> &, const std::shared_ptr<Node> &, const bool &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class MapNode : public Node
	{
	private:
		const std::vector<std::pair<std::string, std::shared_ptr<Node>>> args;

	public:
		MapNode(const std::vector<std::pair<std::string, std::shared_ptr<Node>>> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class SwitchNode : public Node
	{
	private:
		const std::shared_ptr<Node> switchs;
		const std::map<std::shared_ptr<Node>, size_t> cases;
		const std::vector<std::shared_ptr<Node>> gotos;
		std::shared_ptr<Node> elses;

	public:
		SwitchNode(const std::shared_ptr<Node> &, const std::map<std::shared_ptr<Node>, size_t> &, const std::vector<std::shared_ptr<Node>> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		void setElse(const std::shared_ptr<Node> &);
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class TryCatchNode : public Node
	{
	private:
		const std::shared_ptr<Node> trys;
		const std::shared_ptr<Node> catchs;
		const hash_ull key;

	public:
		TryCatchNode(const std::shared_ptr<Node> &, const std::shared_ptr<Node> &, const hash_ull &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ThrowNode : public Node
	{
	private:
		const std::shared_ptr<Node> throws;

	public:
		ThrowNode(const std::shared_ptr<Node> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class CallOpNode : public Node
	{
	private:
		const size_t id;
		const std::vector<std::shared_ptr<Node>> args;

	public:
		CallOpNode(const size_t &, const std::vector<std::shared_ptr<Node>> &, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	namespace ops
	{
		const Symbol index(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol untilstep(const std::shared_ptr<Scope> &, const bool &, const Symbol &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol untilnostep(const std::shared_ptr<Scope> &, const bool &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		// Arithmetic
		const Symbol add(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol sub(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol mul(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol div(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol mod(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol pow(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		// Comparison
		const Symbol less(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol more(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol eless(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol emore(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		// Bit-Wise
		const Symbol bor(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol bxor(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol band(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol bshl(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
		const Symbol bshr(const std::shared_ptr<Scope> &, const Symbol &, const Symbol &, const Token *, std::vector<Function> &);
	}

	namespace dir
	{
		extern std::vector<std::filesystem::path> loaded;

		const std::filesystem::path getRuntimePath();
		const std::filesystem::path findFile(const std::filesystem::path &, const std::string &, const Token *token);
		const std::vector<std::string> compiledOptions(int, char const *[]);
	}

	namespace lib
	{
		extern std::map<std::string, std::map<std::string, extf_t>> loaded;
		extern std::vector<std::filesystem::path> libPaths;

		void loadLibrary(const std::filesystem::path &, const std::string &, const Token *token);
		extf_t loadFunction(const std::string &, const std::string &, const Token *);
	}
}