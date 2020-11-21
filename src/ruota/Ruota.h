#pragma once

#define _RUOTA_VERSION_ "v1.6.11-alpha"
#define RUOTA_EXT_SYM(name, args, token, hash, stack_trace) extern "C" BOOST_SYMBOL_EXPORT const Symbol name(std::vector<Symbol> args, const Token *token, Hash &hash, std::vector<Function> &stack_trace)
#define COERCE_PTR(v, t) reinterpret_cast<t *>(v)

#define RUOTA_DEHASH(x) Ruota::MAIN_HASH.deHash(x)
#define RUOTA_HASH(x) Ruota::MAIN_HASH.hashValue(x)

#define colorASCII(c) "\033[" + std::to_string(c) + "m"
#define PRINTC(s, c) std::cout << colorASCII(c) << s << colorASCII(0)

#include "Locale.h"
#include "RNumber.h"
#include "Keywords.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <iostream>

namespace ruota
{
	class Instruction;
	class Function;
	class Scope;
	class Node;
	class NodeParser;
	class Ruota;
	class Value;
	class Symbol;

	typedef unsigned long long hash_ull;
	typedef unsigned long long refc_ull;
	typedef signed long long type_sll;
	typedef std::vector<type_sll> sig_t;
	typedef std::map<std::string, Symbol> sym_map_t;

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
		TOK_OF = -59
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
		B_SH_R,
		DECLARE_VARS_I
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
		PAREN_NODE
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
	}

	struct Token
	{
		boost::filesystem::path filename;
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
		const boost::filesystem::path currentFile;

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
		std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> parseSigNode(ValueType start);
		std::shared_ptr<Node> parseDefineNode();
		std::shared_ptr<Node> parseLambdaNode();
		std::shared_ptr<Node> parseExternNode();
		std::shared_ptr<Node> parseExternCallNode();
		std::shared_ptr<Node> parseCallBuiltNode();
		std::shared_ptr<Node> parseClassNode();
		std::shared_ptr<Node> parseNewNode();
		std::shared_ptr<Node> parseLoadNode();
		std::shared_ptr<Node> parseSwitchNode();
		std::shared_ptr<Node> parseTryCatchNode();
		std::shared_ptr<Node> parseTypeNode();
		std::shared_ptr<Node> parseTrailingNode(std::shared_ptr<Node>, bool);
		std::shared_ptr<Node> parseInsNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseUntilNode(std::shared_ptr<Node>, bool);
		std::shared_ptr<Node> parseCastToNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseBinOpNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseCallNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseIndexNode(std::shared_ptr<Node>);
		std::shared_ptr<Node> parseThenNode(std::shared_ptr<Node>);

		std::shared_ptr<Node> logErrorN(const std::string &, const Token);
		std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> logErrorSN(const std::string &, const Token);

	public:
		NodeParser(const std::vector<Token> &, const boost::filesystem::path &);
		std::shared_ptr<Node> parse();
		static std::shared_ptr<Instruction> genParser(std::shared_ptr<Node>);
	};

	class Ruota
	{
	private:
		Scope main;

		static const int getToken(const std::string &, size_t &, size_t &, size_t &, std::string &, RNumber &);
		static const char peekChar(const size_t &, const std::string &, const size_t &);
		static const char nextChar(const std::string &, size_t &, size_t &, size_t &);

	public:
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

		static const hash_ull HASH_TO_STRING;
		static const hash_ull HASH_TO_NUMBER;
		static const hash_ull HASH_TO_BOOLEAN;
		static const hash_ull HASH_TO_VECTOR;
		static const hash_ull HASH_TO_DICTIONARY;

		Ruota(std::vector<std::string>);
		std::shared_ptr<Node> compileCode(const std::string &, boost::filesystem::path) const;
		const Symbol runCode(std::shared_ptr<Node>, bool);
		static void printError(const RTError &);
		static const std::vector<Token> lexString(const std::string &, const boost::filesystem::path &);
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
		std::unordered_map<size_t, std::map<sig_t, std::shared_ptr<const Function>>> valueFunction;
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
		Scope *getObject(const Token *, std::vector<Function> &) const;
		const ValueType getValueType() const;
		const type_sll getAugValueType() const;
		const type_sll getTypeName(const Token *, std::vector<Function> &) const;
		const std::shared_ptr<const Function> &getFunction(const std::vector<Symbol> &, const Token *, std::vector<Function> &) const;
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
	};

	namespace dir
	{
		extern std::vector<boost::filesystem::path> loaded;

		inline boost::filesystem::path findFile(const boost::filesystem::path &currentDir, const std::string &filename, const Token *token)
		{
			auto currentDirCheck = currentDir / filename;
			if (boost::filesystem::exists(currentDirCheck))
				return currentDirCheck;
			auto libDirCheck = boost::dll::program_location().parent_path() / "lib" / filename;
			if (boost::filesystem::exists(libDirCheck))
				return libDirCheck;
			std::vector<Function> stack_trace;
			throw RTError((boost::format(_FILE_NOT_FOUND_) % filename).str(), *token, stack_trace);
		}
	} // namespace rdir

	namespace lib
	{
		extern std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)>> loaded;

		inline void loadFunction(const boost::filesystem::path &currentDir, const std::string &rawlibname, const std::string &fname, const Token *token)
		{
			std::string libname = rawlibname;

#ifdef __unix__
			libname += ".so";
#else
			libname += ".dll";
#endif

			std::string search = rawlibname + "$" + fname;
			if (loaded.find(search) != loaded.end())
				return;

			try {
				loaded[search] = boost::dll::import<const Symbol(std::vector<Symbol>, const Token *, Hash &)>(dir::findFile(currentDir, libname, token), fname);
			} catch (const boost::wrapexcept<boost::system::system_error> &e) {
				std::vector<Function> stack_trace;
				throw RTError((boost::format("Error loading `%1%`: %2%") % search % e.what()).str(), *token, stack_trace);
			}
		}
	}
}