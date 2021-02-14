#pragma once

#define _ROSSA_VERSION_ "v1.14.4-alpha"
#define COERCE_PTR(v, t) reinterpret_cast<t *>(v)

#define ROSSA_DEHASH(x) Rossa::MAIN_HASH.deHash(x)
#define ROSSA_HASH(x) Rossa::MAIN_HASH.hashValue(x)

#include "Locale.h"
#include "RNumber.h"
#include "Keywords.h"
#include "Format.h"

#include <map>
#include <vector>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <variant>
#include <type_traits>

#define ROSSA_EXT_SIG(name, args, token, hash, stack_trace) inline const sym_t name(const sym_vec_t &args, const token_t *token, Hash &hash, trace_t &stack_trace)
#define ADD_EXT(name) fmap[#name] = name

#ifndef _WIN32
#include <limits.h>
#include <unistd.h>
#include <dlfcn.h>
#define colorASCII(c) "\033[" + std::to_string(c) + "m"
#define EXPORT_FUNCTIONS(name) extern "C" void name##_rossaExportFunctions(std::map<std::string, extf_t> &fmap)
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define EXPORT_FUNCTIONS(name) extern "C" __declspec(dllexport) void name##_rossaExportFunctions(std::map<std::string, extf_t> &fmap)
#endif

struct token_t;
struct sym_t;
struct param_t;
struct fsig_t;
struct scope_t;
struct node_scope_t;

class Hash;
class Instruction;
class Function;
class Scope;
class Node;
class NodeParser;
class Rossa;
class Value;

typedef unsigned long long hash_ull;
typedef unsigned long long refc_ull;
typedef signed long long type_sll;

typedef std::vector<std::pair<token_t, Function>> trace_t;

typedef std::shared_ptr<Node> node_ptr_t;
typedef std::shared_ptr<const Instruction> i_ptr_t;
typedef std::shared_ptr<Function> func_ptr_t;

typedef std::vector<sym_t> sym_vec_t;
typedef std::vector<node_ptr_t> node_vec_t;
typedef std::vector<i_ptr_t> i_vec_t;
typedef std::vector<param_t> param_vec_t;
typedef std::vector<type_sll> aug_type_t;
typedef std::vector<node_scope_t> ns_vec_t;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

typedef std::map<const std::string, const sym_t> sym_map_t;
typedef std::map<const size_t, std::map<const fsig_t, func_ptr_t>> f_map_t;
typedef std::map<const hash_ull, const sym_t> hash_sym_map_t;

typedef const sym_t(*extf_t)(const sym_vec_t &, const token_t *, Hash &, trace_t &);
typedef void (*export_fns_t)(std::map<std::string, extf_t> &);
typedef std::string(*cm_fns_t)();

#ifndef _WIN32
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
#else
enum TextColor
{
	BLACK_TEXT = 0,
	RED_TEXT = FOREGROUND_RED,
	GREEN_TEXT = FOREGROUND_GREEN,
	YELLOW_TEXT = FOREGROUND_RED | FOREGROUND_GREEN,
	BLUE_TEXT = FOREGROUND_BLUE,
	MAGENTA_TEXT = FOREGROUND_RED | FOREGROUND_BLUE,
	CYAN_TEXT = FOREGROUND_GREEN | FOREGROUND_BLUE,
	WHITE_TEXT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	BRIGHT_BLACK_TEXT = FOREGROUND_INTENSITY,
	BRIGHT_RED_TEXT = FOREGROUND_RED | FOREGROUND_INTENSITY,
	BRIGHT_GREEN_TEXT = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	BRIGHT_YELLOW_TEXT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	BRIGHT_BLUE_TEXT = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	BRIGHT_MAGENTA_TEXT = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	BRIGHT_CYAN_TEXT = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	BRIGHT_WHITE_TEXT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,

	RESET_TEXT = WHITE_TEXT
};
#endif

inline void printc(const std::string &s, const TextColor &color)
{
#ifndef _WIN32
	std::cout << colorASCII(color) << s << colorASCII(0);
#else
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
	std::cout << s;
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

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
	TOK_ENUM = -28,
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
	TOK_DELETE = -42,
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
	TOK_NO_PARAM_LAMBDA = -60,
	TOK_VAR_ARGS = -61,
	TOK_CONST = -62
};

class Hash
{
private:
	std::vector<std::string> variable_hash;
public:
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

	inline const std::vector<std::string> getHashTable() const
	{
		return variable_hash;
	}
};

struct fsig_t
{
private:
	const param_vec_t values;
public:
	fsig_t();
	fsig_t(const param_vec_t &values);
	const size_t validity(const sym_vec_t &, trace_t &stack_trace) const;
	const std::string toString() const;
	const std::string toCodeString() const;
	const bool operator<(const fsig_t &) const;
	const bool operator==(const fsig_t &) const;
};

struct param_t
{
private:
	std::vector<aug_type_t> ancestors;
	aug_type_t base;
	param_vec_t qualifiers;
public:
	param_t(const std::vector<aug_type_t> &, const aug_type_t &);
	param_t(const std::vector<aug_type_t> &, const aug_type_t &, const param_vec_t &);
	void addQualifier(const param_t &);
	const param_vec_t getQualifiers() const;
	const aug_type_t getBase() const;
	const std::string toString() const;
	const std::string toCodeString() const;
	const bool operator<(const param_t &) const;
	const size_t operator&(const param_t &) const;
	const bool operator==(const param_t &) const;
};

struct token_t
{
	std::filesystem::path filename;
	std::string line;
	size_t lineNumber;
	size_t distance;
	std::string valueString;
	number_t valueNumber;
	int type = NULL_TOK;
};

class rossa_error : public std::runtime_error
{
private:
	const token_t token;
	const trace_t stack_trace;

public:
	rossa_error(const std::string &, const token_t &, const trace_t &);
	const token_t &getToken() const;
	const trace_t &getTrace() const;
};

struct scope_t
{
private:
	Scope *scope;

public:
	enum type_t
	{
		WEAK,
		STRONG
	} type;

	enum scope_type_t
	{
		SCOPE_O,
		STRUCT_O,
		STATIC_O,
		INSTANCE_O,
		VIRTUAL_O
	};

	scope_t();
	scope_t(Scope *, const type_t &);
	scope_t(const hash_ull &key);
	scope_t(const scope_t *, const hash_ull &);
	scope_t(const scope_t *, const scope_type_t &, const i_ptr_t &, const hash_ull &, const scope_t *, const std::vector<aug_type_t> &);
	scope_t(Scope *, const aug_type_t &, const std::vector<aug_type_t> &);

	scope_t(const scope_t &);
	~scope_t();
	void operator=(const scope_t &);
	const bool operator==(const scope_t &) const;

	const sym_t instantiate(const sym_vec_t &, const token_t *, trace_t &) const;
	const bool extendsObject(const aug_type_t &) const;
	const scope_type_t getType() const;
	const i_ptr_t getBody() const;
	const param_t getTypeVec() const;
	const std::string getKey() const;
	const bool hasValue(const hash_ull &) const;
	const sym_t getThis(const token_t *, trace_t &) const;

	const sym_t &getVariable(const hash_ull &, const token_t *, trace_t &) const;
	const sym_t &createVariable(const hash_ull &, const token_t *) const;
	const sym_t &createVariable(const hash_ull &, const sym_t &, const token_t *) const;

	Scope *getPtr() const;
};

struct node_scope_t
{
	hash_ull id;
	std::vector<hash_ull> var_ids;
};

struct f_wrapper
{
	f_map_t map;
	func_ptr_t varg = nullptr;

	f_wrapper(const f_map_t &, const func_ptr_t &);
};

class Value
{
	friend class sym_t;
public:
	enum type_t
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
		POINTER = -10,
		ANY = -11
	} type;

private:
	std::variant<
		bool,
		number_t,
		aug_type_t,
		std::string,
		std::shared_ptr<void>,
		sym_vec_t,
		f_wrapper,
		sym_map_t,
		scope_t> value;
	/*union
	{
		bool valueBool;
		number_t valueNumber;
	};

	aug_type_t valueType;
	std::string valueString;
	std::shared_ptr<void> valuePointer;
	sym_vec_t valueVector;
	f_map_t valueFunction;
	func_ptr_t valueVARGFunction = nullptr;
	sym_map_t valueDictionary;
	scope_t valueObject; */
	refc_ull references = 1;

	Value();
	Value(const aug_type_t &);
	Value(const bool &);
	Value(const std::shared_ptr<void> &);
	Value(const scope_t &);
	Value(const fsig_t &, const func_ptr_t &);
	Value(const func_ptr_t &);
	Value(const number_t &);
	Value(const sym_vec_t &);
	Value(const sym_map_t &);
	Value(const std::string &);
	void clearData();
};

struct sym_t
{
private:
	Value *d;

	sym_t(const std::shared_ptr<void> &);
	sym_t(const aug_type_t &);
	sym_t(const number_t &);
	sym_t(const bool &);
	sym_t(const sym_vec_t &);
	sym_t(const scope_t &);
	sym_t(const fsig_t &, const func_ptr_t &);
	sym_t(const func_ptr_t &);
	sym_t(const std::string &);
	sym_t(const sym_map_t &);

public:
	enum type_t
	{
		ID_RETURN,
		ID_BREAK,
		ID_CONTINUE,
		ID_CASUAL,
		ID_REFER
	} type;

	sym_t(const type_t &);
	sym_t(const sym_t &);
	sym_t();

	static const sym_t Pointer(const std::shared_ptr<void> &);
	static const sym_t TypeName(const aug_type_t &);
	static const sym_t Number(const number_t &);
	static const sym_t Boolean(const bool &);
	static const sym_t Array(const sym_vec_t &);
	static const sym_t Object(const scope_t &);
	static const sym_t FunctionSIG(const fsig_t &, const func_ptr_t &);
	static const sym_t FunctionVARG(const func_ptr_t &);
	static const sym_t String(const std::string &);
	static const sym_t Dictionary(const sym_map_t &);

	~sym_t();

	void operator=(const sym_t &);
	static const sym_t allocate(const size_t &);
	const type_t getSymbolType() const;
	void setSymbolType(const type_t &);
	const number_t &getNumber(const token_t *, trace_t &) const;
	void *getPointer(const token_t *, trace_t &) const;
	const sym_map_t &getDictionary(const token_t *, trace_t &) const;
	const sym_t &indexVector(const size_t &, const token_t *, trace_t &) const;
	const sym_vec_t &getVector(const token_t *, trace_t &) const;
	const std::string getString(const token_t *, trace_t &) const;
	const bool getBool(const token_t *, trace_t &) const;
	const bool hasVarg(const token_t *, trace_t &) const;
	scope_t *getObject(const token_t *, trace_t &) const;
	const Value::type_t getValueType() const;
	const param_t getAugValueType() const;
	const aug_type_t getTypeName(const token_t *, trace_t &) const;
	const func_ptr_t getFunction(const sym_vec_t &, const token_t *, trace_t &) const;
	const func_ptr_t &getVARGFunction(const token_t *, trace_t &) const;
	const sym_t &indexDict(const std::string &) const;
	const bool hasDictionaryKey(const std::string &) const;
	const size_t vectorSize() const;
	const size_t dictionarySize(const token_t *, trace_t &) const;
	const std::string toString(const token_t *, trace_t &) const;
	const std::string toCodeString() const;
	const sym_t call(const sym_vec_t &, const token_t *, trace_t &) const;
	void addFunctions(const sym_t *, const token_t *) const;
	void nullify() const;
	void set(const sym_t *, const token_t *, const bool &, trace_t &) const;
	const bool equals(const sym_t *, const token_t *, trace_t &) const;
	const bool nequals(const sym_t *, const token_t *, trace_t &) const;
	const bool pureEquals(const sym_t *, const token_t *, trace_t &) const;
	const bool pureNEquals(const sym_t *, const token_t *, trace_t &) const;
	const bool operator==(const sym_t &) const;
	const bool operator!=(const sym_t &) const;
	const bool operator<(const sym_t &) const;
	const f_map_t &getFunctionOverloads(const token_t *, trace_t &) const;
	void shift() const;

	const sym_t clone() const;
};

class Instruction
{
protected:
	const token_t token;

public:
	enum type_t
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
		VARG_DEFINE,
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
	B_NOT_I,
	TYPE_I,
	CALL_OP_I,
	GET_THIS_I,
	DELETE_I,
	UN_ADD_I,
	NEG_I,
	NOT_I,
	CONCAT_I
	} const type;

	Instruction(const type_t &, const token_t &);
	virtual const sym_t evaluate(const scope_t *, trace_t &) const = 0;
	const type_t getType() const;
	virtual ~Instruction();
};

class Scope
{
	friend class scope_t;

public:
	Scope *getParent() const;

private:
	const scope_t::scope_type_t type;
	Scope *const parent;
	refc_ull references = 1;
	hash_sym_map_t values;
	const i_ptr_t body;
	//hash_ull hashed_key;
	aug_type_t name_trace;
	std::vector<aug_type_t> extensions;

	void traceName(const hash_ull &);

	Scope(const scope_t::scope_type_t &, Scope *, const i_ptr_t &, const hash_ull &);
	Scope(Scope *, const aug_type_t &, const std::vector<aug_type_t> &);

	const sym_t &getVariable(const hash_ull &, const token_t *, trace_t &) const;
	const sym_t &createVariable(const hash_ull &, const token_t *);
	const sym_t &createVariable(const hash_ull &, const sym_t &, const token_t *);
	const sym_t getThis(const token_t *, trace_t &);

	~Scope();
};

class Function : public std::enable_shared_from_this<Function>
{
private:
	const hash_ull key;
	Scope *parent;
	const std::vector<std::pair<LexerTokenType, hash_ull>> params;
	const i_ptr_t body;
	const hash_sym_map_t captures;
	const bool isVargs;
	const sym_t evaluateVARGS(const sym_vec_t &, const token_t *, trace_t &) const;

public:
	Function(const hash_ull &, Scope *, const std::vector<std::pair<LexerTokenType, hash_ull>> &, const i_ptr_t &, const hash_sym_map_t &);
	Function(const hash_ull &, Scope *, const i_ptr_t &, const hash_sym_map_t &);
	const sym_t evaluate(const sym_vec_t &, const token_t *, trace_t &) const;
	const size_t getArgSize() const;
	const hash_ull getKey() const;
	const scope_t getParent() const;
	const std::vector<std::pair<LexerTokenType, hash_ull>> &getParams() const;
	void shift();
};

class Node
{
	friend class NodeParser;

protected:
	const ns_vec_t path;

	enum type_t
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
		CALL_OP_NODE,
		VARG_DEFINE_NODE,
		DELETE_NODE
	} const type;
	const token_t token;

public:
	Node(const ns_vec_t &, const type_t &, const token_t &);
	const type_t getType() const;
	const token_t getToken() const;

	virtual i_ptr_t genParser() const = 0;
	virtual bool isConst() const = 0;
	virtual void printTree(std::string, bool) const = 0;
	virtual const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const = 0;
};

class NodeParser
{
private:
	hash_ull scope_i = 0;
	std::vector<std::pair<std::vector<hash_ull>, sym_t>> *consts;

	const std::vector<token_t> tokens;
	const std::filesystem::path currentFile;

	unsigned int index = 0;
	token_t currentToken;
	void nextToken();
	node_ptr_t parseNumNode(ns_vec_t *);
	node_ptr_t parseBoolNode(ns_vec_t *);
	node_ptr_t parseIDNode(ns_vec_t *);
	node_ptr_t parseBIDNode(ns_vec_t *);
	node_ptr_t parseEntryNode(ns_vec_t *);
	node_ptr_t parseExprNode(ns_vec_t *);
	node_ptr_t parseEquNode(ns_vec_t *);
	node_ptr_t parseVectorNode(ns_vec_t *);
	node_ptr_t parseUnitNode(ns_vec_t *);
	node_ptr_t parseBaseNode(ns_vec_t *);
	node_ptr_t parseUnOpNode(ns_vec_t *);
	node_ptr_t parseMapNode(ns_vec_t *);
	node_ptr_t parseIfElseNode(ns_vec_t *);
	node_ptr_t parseWhileNode(ns_vec_t *);
	node_ptr_t parseForNode(ns_vec_t *);
	std::pair<fsig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> parseSigNode(ns_vec_t *);
	node_ptr_t parseDefineNode(ns_vec_t *);
	node_ptr_t parseLambdaNode(ns_vec_t *);
	node_ptr_t parseNPLambdaNode(ns_vec_t *);
	node_ptr_t parseExternNode(ns_vec_t *);
	node_ptr_t parseExternCallNode(ns_vec_t *);
	node_ptr_t parseCallOpNode(ns_vec_t *);
	node_ptr_t parseCallBuiltNode(ns_vec_t *);
	node_ptr_t parseClassNode(ns_vec_t *);
	node_ptr_t parseNewNode(ns_vec_t *);
	node_ptr_t parseLoadNode(ns_vec_t *);
	node_ptr_t parseSwitchNode(ns_vec_t *);
	node_ptr_t parseTryCatchNode(ns_vec_t *);
	node_ptr_t parseTypeNode(ns_vec_t *);
	param_t parseParamTypeNode(ns_vec_t *, const aug_type_t &);
	node_ptr_t parseTrailingNode(ns_vec_t *, const node_ptr_t &, const bool &);
	node_ptr_t parseInsNode(ns_vec_t *, const node_ptr_t &);
	node_ptr_t parseUntilNode(ns_vec_t *, const node_ptr_t &, const bool &);
	node_ptr_t parseBinOpNode(ns_vec_t *, const node_ptr_t &);
	node_ptr_t parseCallNode(ns_vec_t *, const node_ptr_t &);
	node_ptr_t parseIndexNode(ns_vec_t *, const node_ptr_t &);
	node_ptr_t parseThenNode(ns_vec_t *, const node_ptr_t &);

	node_ptr_t logErrorN(const std::string &, const token_t &);
	param_t logErrorPT(const std::string &, const token_t &);
	std::pair<fsig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> logErrorSN(const std::string &, const token_t &);

public:
	NodeParser(const std::vector<token_t> &, const std::filesystem::path &);
	node_ptr_t parse(ns_vec_t *, std::vector<std::pair<std::vector<hash_ull>, sym_t>> *);
	static i_ptr_t genParser(const node_ptr_t &);
};

class Rossa
{
private:
	std::vector<std::pair<std::vector<hash_ull>, sym_t>> consts;
	ns_vec_t scopes;
	static const int getToken(const std::string &, size_t &, size_t &, size_t &, std::string &, number_t &);
	static const char peekChar(const size_t &, const std::string &, const size_t &);
	static const char nextChar(const std::string &, size_t &, size_t &, size_t &);

public:
	scope_t main;

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
	static const hash_ull HASH_B_NOT;
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
	static const hash_ull HASH_VAR_ARGS;
	static const hash_ull HASH_LENGTH;
	static const hash_ull HASH_NOT;
	static const hash_ull HASH_CCT;

	Rossa(const std::vector<std::string> &);
	static void loadStandardFunctions(std::map<std::string, extf_t> &fmap);
	const node_ptr_t compileCode(const std::string &, const std::filesystem::path &);
	const sym_t runCode(const node_ptr_t &, const bool &);
	static void printError(const rossa_error &);
	static const std::vector<token_t> lexString(const std::string &, const std::filesystem::path &);

	~Rossa();
};

// INSTRUCTIONS -----------------------------------------------------------------------------

class UnaryI : public Instruction
{
protected:
	const i_ptr_t a;

public:
	UnaryI(const type_t &, const i_ptr_t &, const token_t &);
	const i_ptr_t getA() const;
};

class CastingI : public Instruction
{
protected:
	const hash_ull key;

public:
	CastingI(const type_t &, const hash_ull &, const token_t &);
	const hash_ull getKey() const;
};

class BinaryI : public UnaryI
{
protected:
	const i_ptr_t b;

public:
	BinaryI(const type_t &, const i_ptr_t &, const i_ptr_t &, const token_t &);
	const i_ptr_t getB() const;
};

class ContainerI : public Instruction
{
protected:
	const sym_t d;

public:
	ContainerI(const sym_t &d, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class DefineI : public Instruction
{
protected:
	const hash_ull key;
	const fsig_t ftype;
	const std::vector<std::pair<LexerTokenType, hash_ull>> params;
	const i_ptr_t body;
	const std::vector<hash_ull> captures;

public:
	DefineI(const hash_ull &, const fsig_t &, const std::vector<std::pair<LexerTokenType, hash_ull>> &, const i_ptr_t &, const std::vector<hash_ull> &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class VargDefineI : public Instruction
{
protected:
	const hash_ull key;
	const i_ptr_t body;
	const std::vector<hash_ull> captures;

public:
	VargDefineI(const hash_ull &, const i_ptr_t &, const std::vector<hash_ull> &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class SequenceI : public Instruction
{
protected:
	const i_vec_t children;

public:
	SequenceI(const i_vec_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class IfElseI : public Instruction
{
protected:
	const i_ptr_t ifs;
	const i_ptr_t body;
	const i_ptr_t elses;

public:
	IfElseI(const i_ptr_t &, const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class WhileI : public Instruction
{
protected:
	const i_ptr_t whiles;
	const i_vec_t body;

public:
	WhileI(const i_ptr_t &, const i_vec_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ForI : public Instruction
{
protected:
	const hash_ull id;
	const i_ptr_t fors;
	const i_vec_t body;

public:
	ForI(const hash_ull &, const i_ptr_t &, const i_vec_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class VariableI : public CastingI
{
public:
	VariableI(const hash_ull &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class GetThisI : public CastingI
{
public:
	GetThisI(const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class DeclareI : public CastingI
{
protected:
	const type_sll vtype;
	const i_ptr_t a;
	const bool isConst;

public:
	DeclareI(const hash_ull &, const type_sll &, const i_ptr_t &, const bool &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class CallI : public BinaryI
{
public:
	CallI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class AddI : public BinaryI
{
public:
	AddI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class SubI : public BinaryI
{
public:
	SubI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class MulI : public BinaryI
{
public:
	MulI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class DivI : public BinaryI
{
public:
	DivI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ModI : public BinaryI
{
public:
	ModI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class PowI : public BinaryI
{
public:
	PowI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class LessI : public BinaryI
{
public:
	LessI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class EqualsI : public BinaryI
{
public:
	EqualsI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class NEqualsI : public BinaryI
{
public:
	NEqualsI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class AndI : public BinaryI
{
public:
	AndI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class OrI : public BinaryI
{
public:
	OrI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class BOrI : public BinaryI
{
public:
	BOrI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class BAndI : public BinaryI
{
public:
	BAndI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class BXOrI : public BinaryI
{
public:
	BXOrI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class BShiftLeftI : public BinaryI
{
public:
	BShiftLeftI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class BShiftRightI : public BinaryI
{
public:
	BShiftRightI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class BNotI : public UnaryI
{
public:
	BNotI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class SetI : public BinaryI
{
protected:
	const bool isConst;

public:
	SetI(const i_ptr_t &, const i_ptr_t &, const bool &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ExternI : public UnaryI
{
protected:
	const std::string libname;
	const std::string fname;
	extf_t f;

public:
	ExternI(const std::string &, const std::string &, const i_ptr_t &a, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class LengthI : public UnaryI
{
public:
	LengthI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ClassI : public Instruction
{
protected:
	const hash_ull key;
	const scope_t::scope_type_t type;
	const i_ptr_t body;
	const i_ptr_t extends;

public:
	ClassI(const hash_ull &, const scope_t::scope_type_t &, const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class NewI : public BinaryI
{
public:
	NewI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class CastToI : public BinaryI
{
public:
	CastToI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class AllocI : public UnaryI
{
public:
	AllocI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class UntilI : public BinaryI
{
protected:
	const i_ptr_t step;
	const bool inclusive;

public:
	UntilI(const i_ptr_t &, const i_ptr_t &, const i_ptr_t &, const bool &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ScopeI : public Instruction
{
protected:
	const i_vec_t children;

public:
	ScopeI(const i_vec_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class MapI : public Instruction
{
protected:
	const std::map<std::string, i_ptr_t> children;

public:
	MapI(const std::map<std::string, i_ptr_t> &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ReferI : public UnaryI
{
public:
	ReferI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class SwitchI : public Instruction
{
protected:
	const i_ptr_t switchs;
	const std::map<sym_t, size_t> cases_solved;
	const std::map<i_ptr_t, size_t> cases_unsolved;
	const i_vec_t cases;
	const i_ptr_t elses;

public:
	SwitchI(const i_ptr_t &, const std::map<sym_t, size_t> &, const std::map<i_ptr_t, size_t> &, const i_vec_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class TryCatchI : public BinaryI
{
protected:
	const hash_ull key;

public:
	TryCatchI(const i_ptr_t &, const i_ptr_t &, const hash_ull &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ThrowI : public UnaryI
{
public:
	ThrowI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class PureEqualsI : public BinaryI
{
public:
	PureEqualsI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class PureNEqualsI : public BinaryI
{
public:
	PureNEqualsI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class CharNI : public UnaryI
{
public:
	CharNI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class CharSI : public UnaryI
{
public:
	CharSI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class DeclareVarsI : public Instruction
{
protected:
	const std::vector<hash_ull> keys;

public:
	DeclareVarsI(const std::vector<hash_ull> &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ParseI : public UnaryI
{
public:
	ParseI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class TypeI : public UnaryI
{
public:
	TypeI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class CallOpI : public Instruction
{
protected:
	const size_t id;
	const i_vec_t children;

public:
	CallOpI(const size_t &, const i_vec_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class DeleteI : public UnaryI
{
public:
	DeleteI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class UnAddI : public UnaryI
{
public:
	UnAddI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class NegI : public UnaryI
{
public:
	NegI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class NotI : public UnaryI
{
public:
	NotI(const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

class ConcatI : public BinaryI
{
public:
	ConcatI(const i_ptr_t &, const i_ptr_t &, const token_t &);
	const sym_t evaluate(const scope_t *, trace_t &) const override;
};

// NODES -----------------------------------------------------------------------------

class ContainerNode : public Node
{
private:
	const sym_t s;

public:
	ContainerNode(const ns_vec_t &, const sym_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class VectorNode : public Node
{
private:
	const node_vec_t args;
	bool scoped;

public:
	VectorNode(const ns_vec_t &, const node_vec_t &, const bool &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
	const node_vec_t &getChildren();
};

class BreakNode : public Node
{
public:
	BreakNode(const ns_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ContinueNode : public Node
{
public:
	ContinueNode(const ns_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class IDNode : public Node
{
private:
	const hash_ull key;

public:
	IDNode(const ns_vec_t &, const hash_ull &, const token_t &);
	hash_ull getKey() const;
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class BIDNode : public Node
{
private:
	const std::string key;

public:
	BIDNode(const ns_vec_t &, const std::string &, const token_t &);
	const std::string getKey() const;
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class DefineNode : public Node
{
private:
	const hash_ull key;
	const fsig_t ftype;
	const std::vector<std::pair<LexerTokenType, hash_ull>> params;
	const node_ptr_t body;
	const std::vector<hash_ull> captures;

public:
	DefineNode(const ns_vec_t &, const hash_ull &, const fsig_t &, const std::vector<std::pair<LexerTokenType, hash_ull>> &, const node_ptr_t &, const std::vector<hash_ull> &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class VargDefineNode : public Node
{
private:
	const hash_ull key;
	const node_ptr_t body;
	const std::vector<hash_ull> captures;

public:
	VargDefineNode(const ns_vec_t &, const hash_ull &, const node_ptr_t &, const std::vector<hash_ull> &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class NewNode : public Node
{
private:
	const node_ptr_t object;
	const node_ptr_t params;

public:
	NewNode(const ns_vec_t &, const node_ptr_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ClassNode : public Node
{
private:
	const hash_ull key;
	const int type;
	const node_vec_t body;
	const node_ptr_t extends;

public:
	ClassNode(const ns_vec_t &, const hash_ull &, const int &, const node_vec_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class VarNode : public Node
{
private:
	const std::vector<hash_ull> keys;

public:
	VarNode(const ns_vec_t &, const std::vector<hash_ull> &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class CallNode : public Node
{
private:
	const node_ptr_t callee;
	const node_vec_t args;

public:
	CallNode(const ns_vec_t &, const node_ptr_t &, const node_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	node_ptr_t getCallee() const;
	node_vec_t getArgs() const;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ExternCallNode : public Node
{
private:
	const std::string libname;
	const std::string fname;
	const node_vec_t args;

public:
	ExternCallNode(const ns_vec_t &, const std::string &, const std::string &, const node_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class CallBuiltNode : public Node
{
private:
	const LexerTokenType t;
	const node_ptr_t arg;

public:
	CallBuiltNode(const ns_vec_t &, const LexerTokenType &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ReturnNode : public Node
{
private:
	const node_ptr_t a;

public:
	ReturnNode(const ns_vec_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ReferNode : public Node
{
private:
	const node_ptr_t a;

public:
	ReferNode(const ns_vec_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class BinOpNode : public Node
{
private:
	const std::string op;
	node_ptr_t a;
	node_ptr_t b;

public:
	BinOpNode(const ns_vec_t &, const std::string &, const node_ptr_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	const std::string &getOp() const;
	const node_ptr_t getA() const;
	const node_ptr_t getB() const;
	void setA(const node_ptr_t &);
	void setB(const node_ptr_t &);
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class UnOpNode : public Node
{
private:
	const std::string op;
	const node_ptr_t a;

public:
	UnOpNode(const ns_vec_t &, const std::string &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ParenNode : public Node
{
private:
	const node_ptr_t a;

public:
	ParenNode(const ns_vec_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class InsNode : public Node
{
private:
	const node_ptr_t callee;
	const node_ptr_t arg;

public:
	InsNode(const ns_vec_t &, const node_ptr_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	const node_ptr_t getCallee() const;
	const node_ptr_t getArg() const;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class IfElseNode : public Node
{
private:
	const node_ptr_t ifs;
	const node_ptr_t body;
	node_ptr_t elses = nullptr;

public:
	IfElseNode(const ns_vec_t &, const node_ptr_t &, const node_ptr_t &, const token_t &);
	void setElse(const node_ptr_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class WhileNode : public Node
{
private:
	const node_ptr_t whiles;
	const node_vec_t body;

public:
	WhileNode(const ns_vec_t &, const node_ptr_t &, const node_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ForNode : public Node
{
private:
	hash_ull id;
	node_ptr_t fors;
	node_vec_t body;

public:
	ForNode(const ns_vec_t &, const hash_ull &, const node_ptr_t &, const node_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class UntilNode : public Node
{
private:
	const node_ptr_t a;
	const node_ptr_t b;
	const node_ptr_t step;
	const bool inclusive;

public:
	UntilNode(const ns_vec_t &, const node_ptr_t &, const node_ptr_t &, const node_ptr_t &, const bool &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class MapNode : public Node
{
private:
	const std::vector<std::pair<std::string, node_ptr_t>> args;

public:
	MapNode(const ns_vec_t &, const std::vector<std::pair<std::string, node_ptr_t>> &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class SwitchNode : public Node
{
private:
	const node_ptr_t switchs;
	const std::map<node_ptr_t, size_t> cases;
	const node_vec_t gotos;
	node_ptr_t elses;

public:
	SwitchNode(const ns_vec_t &, const node_ptr_t &, const std::map<node_ptr_t, size_t> &, const node_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	void setElse(const node_ptr_t &);
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class TryCatchNode : public Node
{
private:
	const node_ptr_t trys;
	const node_ptr_t catchs;
	const hash_ull key;

public:
	TryCatchNode(const ns_vec_t &, const node_ptr_t &, const node_ptr_t &, const hash_ull &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class ThrowNode : public Node
{
private:
	const node_ptr_t throws;

public:
	ThrowNode(const ns_vec_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class CallOpNode : public Node
{
private:
	const size_t id;
	const node_vec_t args;

public:
	CallOpNode(const ns_vec_t &, const size_t &, const node_vec_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

class DeleteNode : public Node
{
private:
	const node_ptr_t del;

public:
	DeleteNode(const ns_vec_t &, const node_ptr_t &, const token_t &);
	i_ptr_t genParser() const override;
	bool isConst() const override;
	void printTree(std::string, bool) const override;
	const node_ptr_t fold(const std::vector<std::pair<std::vector<hash_ull>, sym_t>> &) const override;
};

namespace ops
{
	const sym_t index(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t call(const scope_t *, const i_ptr_t &, const sym_vec_t &, const token_t *, trace_t &);
	const sym_t untilstep(const scope_t *, const bool &, const sym_t &, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t untilnostep(const scope_t *, const bool &, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t cct(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	// Arithmetic
	const sym_t add(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t sub(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t mul(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t div(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t mod(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t pow(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	// Comparison
	const sym_t less(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t more(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t eless(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t emore(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	// Bit-Wise
	const sym_t bor(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t bxor(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t band(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t bshl(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t bshr(const scope_t *, const sym_t &, const sym_t &, const token_t *, trace_t &);
	const sym_t bnot(const scope_t *, const sym_t &, const token_t *, trace_t &);
	// unary
	const sym_t unadd(const scope_t *, const sym_t &, const token_t *, trace_t &);
	const sym_t neg(const scope_t *, const sym_t &, const token_t *, trace_t &);
	const sym_t unot(const scope_t *, const sym_t &, const token_t *, trace_t &);
}

namespace dir
{
	extern std::vector<std::filesystem::path> loaded;

	const std::filesystem::path getRuntimePath();
	const std::filesystem::path findFile(const std::filesystem::path &, const std::string &, const token_t *token);
}

namespace lib
{
	extern std::map<std::string, std::map<std::string, extf_t>> loaded;

	void loadLibrary(const std::filesystem::path &, const std::string &, const token_t *token);
	extf_t loadFunction(const std::string &, const std::string &, const token_t *);
}

inline const std::string getTypeString(const aug_type_t &t)
{
	std::string ret = "";
	int j = 0;
	for (auto &i : t) {
		if (j++ > 0)
			ret += ".";
		if (i >= 0)
			ret += ROSSA_DEHASH(i);
		else {
			switch (i) {
				case Value::type_t::NIL:
					ret += KEYWORD_NIL_NAME;
					break;
				case Value::type_t::ANY:
					ret += KEYWORD_ANY;
					break;
				case Value::type_t::NUMBER:
					ret += KEYWORD_NUMBER;
					break;
				case Value::type_t::STRING:
					ret += KEYWORD_STRING;
					break;
				case Value::type_t::BOOLEAN_D:
					ret += KEYWORD_BOOLEAN;
					break;
				case Value::type_t::ARRAY:
					ret += KEYWORD_ARRAY;
					break;
				case Value::type_t::FUNCTION:
					ret += KEYWORD_FUNCTION;
					break;
				case Value::type_t::DICTIONARY:
					ret += KEYWORD_DICTIONARY;
					break;
				case Value::type_t::OBJECT:
					ret += KEYWORD_OBJECT;
					break;
				case Value::type_t::POINTER:
					ret += KEYWORD_POINTER;
					break;
				case Value::type_t::TYPE_NAME:
					ret += KEYWORD_TYPE;
					break;
				default:
					return "<error-type>";
			}
		}
	}
	return ret;
}

inline const std::string deHashVec(const ns_vec_t &t)
{
	std::string ret = "";
	int j = 0;
	for (auto &i : t) {
		if (j++ > 0)
			ret += ".";
		ret += ROSSA_DEHASH(i.id) + "[";
		int k = 0;
		for (auto &v : i.var_ids) {
			if (k++ > 0)
				ret += ", ";
			ret += ROSSA_DEHASH(v);
		}
		ret += "]";
	}
	return ret;
}