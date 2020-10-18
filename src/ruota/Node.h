#pragma once

#include "Ruota.h"

namespace ruota
{
	inline std::string colorASCII(const TextColor &color)
	{
		return "\033[" + std::to_string(color) + "m";
	}

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
		VectorNode(std::vector<std::shared_ptr<Node>>, bool, const Token &);
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

	class IDNode : public Node
	{
	private:
		hash_ull key;

	public:
		IDNode(hash_ull, const Token &);
		hash_ull getKey() const;
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class BIDNode : public Node
	{
	private:
		std::string key;

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
		hash_ull key;
		sig_t ftype;
		std::vector<std::pair<LexerTokenType, hash_ull>> params;
		std::shared_ptr<Node> body;

	public:
		DefineNode(hash_ull, sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>, std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class NewNode : public Node
	{
	private:
		std::shared_ptr<Node> object;
		std::shared_ptr<Node> params;

	public:
		NewNode(std::shared_ptr<Node>, std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ClassNode : public Node
	{
	private:
		hash_ull key;
		int type;
		std::vector<std::shared_ptr<Node>> body;
		std::shared_ptr<Node> extends;

	public:
		ClassNode(hash_ull, int, std::vector<std::shared_ptr<Node>>, std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class VarNode : public Node
	{
	private:
		std::vector<hash_ull> keys;

	public:
		VarNode(std::vector<hash_ull>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class CallNode : public Node
	{
	private:
		std::shared_ptr<Node> callee;
		std::vector<std::shared_ptr<Node>> args;

	public:
		CallNode(std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, const Token &);
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
		std::string id;
		std::vector<std::shared_ptr<Node>> args;

	public:
		ExternCallNode(const std::string &, std::vector<std::shared_ptr<Node>>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class CallBuiltNode : public Node
	{
	private:
		LexerTokenType t;
		std::shared_ptr<Node> arg;

	public:
		CallBuiltNode(LexerTokenType, std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ReturnNode : public Node
	{
	private:
		std::shared_ptr<Node> a;

	public:
		ReturnNode(std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ReferNode : public Node
	{
	private:
		std::shared_ptr<Node> a;

	public:
		ReferNode(std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class BinOpNode : public Node
	{
	private:
		std::string op;
		std::shared_ptr<Node> a;
		std::shared_ptr<Node> b;

	public:
		BinOpNode(const std::string &, std::shared_ptr<Node>, std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		const std::string &getOp() const;
		std::shared_ptr<Node> getA() const;
		std::shared_ptr<Node> getB() const;
		void setA(std::shared_ptr<Node>);
		void setB(std::shared_ptr<Node>);
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class UnOpNode : public Node
	{
	private:
		std::string op;
		std::shared_ptr<Node> a;

	public:
		UnOpNode(const std::string &, std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ParenNode : public Node
	{
	private:
		std::shared_ptr<Node> a;

	public:
		ParenNode(std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class CastToNode : public Node
	{
	private:
		ValueType convert;
		std::shared_ptr<Node> a;

	public:
		CastToNode(ValueType, std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class InsNode : public Node
	{
	private:
		std::shared_ptr<Node> callee;
		std::shared_ptr<Node> arg;

	public:
		InsNode(std::shared_ptr<Node>, std::shared_ptr<Node>, const Token &);
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
		std::shared_ptr<Node> ifs;
		std::shared_ptr<Node> body;
		std::shared_ptr<Node> elses = nullptr;

	public:
		IfElseNode(std::shared_ptr<Node>, std::shared_ptr<Node>, const Token &);
		void setElse(std::shared_ptr<Node>);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class WhileNode : public Node
	{
	private:
		std::shared_ptr<Node> whiles;
		std::vector<std::shared_ptr<Node>> body;

	public:
		WhileNode(std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, const Token &);
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
		ForNode(hash_ull, std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class UntilNode : public Node
	{
	private:
		std::shared_ptr<Node> a;
		std::shared_ptr<Node> b;
		std::shared_ptr<Node> step = nullptr;
		bool inclusive;

	public:
		UntilNode(std::shared_ptr<Node>, std::shared_ptr<Node>, std::shared_ptr<Node>, bool, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class MapNode : public Node
	{
	private:
		std::vector<std::pair<hash_ull, std::shared_ptr<Node>>> args;

	public:
		MapNode(std::vector<std::pair<hash_ull, std::shared_ptr<Node>>>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class SwitchNode : public Node
	{
	private:
		std::shared_ptr<Node> switchs;
		std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> cases;
		std::shared_ptr<Node> elses;

	public:
		SwitchNode(std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, std::shared_ptr<Node>>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		void setElse(std::shared_ptr<Node>);
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class TryCatchNode : public Node
	{
	private:
		std::shared_ptr<Node> trys;
		std::shared_ptr<Node> catchs;
		hash_ull key;

	public:
		TryCatchNode(std::shared_ptr<Node>, std::shared_ptr<Node>, hash_ull, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};

	class ThrowNode : public Node
	{
	private:
		std::shared_ptr<Node> throws;

	public:
		ThrowNode(std::shared_ptr<Node>, const Token &);
		std::shared_ptr<Instruction> genParser() const override;
		bool isConst() const override;
		std::stringstream printTree(std::string, bool) const override;
		std::shared_ptr<Node> fold() const override;
	};
}