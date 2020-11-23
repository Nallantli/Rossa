#pragma once

#include "Rossa.h"

namespace rossa
{
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
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class DefineI : public Instruction
	{
	protected:
		const hash_ull key;
		const sig_t ftype;
		const std::vector<std::pair<LexerTokenType, hash_ull>> params;
		const std::shared_ptr<Instruction> body;

	public:
		DefineI(const hash_ull &, const sig_t &, std::vector<std::pair<LexerTokenType, hash_ull>>, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class SequenceI : public Instruction
	{
	protected:
		const std::vector<std::shared_ptr<Instruction>> children;

	public:
		SequenceI(std::vector<std::shared_ptr<Instruction>>, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class IfElseI : public Instruction
	{
	protected:
		const std::shared_ptr<Instruction> ifs;
		const std::shared_ptr<Instruction> body;
		const std::shared_ptr<Instruction> elses;

	public:
		IfElseI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class WhileI : public Instruction
	{
	protected:
		const std::shared_ptr<Instruction> whiles;
		const std::shared_ptr<Instruction> body;

	public:
		WhileI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ForI : public Instruction
	{
	protected:
		const hash_ull id;
		const std::shared_ptr<Instruction> fors;
		const std::shared_ptr<Instruction> body;

	public:
		ForI(const hash_ull &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class VariableI : public CastingI
	{
	public:
		VariableI(const hash_ull &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class DeclareI : public CastingI
	{
	protected:
		const type_sll vtype;
		const std::shared_ptr<Instruction> a;
		const bool isConst;

	public:
		DeclareI(const hash_ull &, const type_sll &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class IndexI : public BinaryI
	{
	public:
		IndexI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class InnerI : public BinaryI
	{
	public:
		InnerI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class CallI : public BinaryI
	{
	public:
		CallI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class AddI : public BinaryI
	{
	public:
		AddI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class SubI : public BinaryI
	{
	public:
		SubI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class MulI : public BinaryI
	{
	public:
		MulI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class DivI : public BinaryI
	{
	public:
		DivI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ModI : public BinaryI
	{
	public:
		ModI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class PowI : public BinaryI
	{
	public:
		PowI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class LessI : public BinaryI
	{
	public:
		LessI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class MoreI : public BinaryI
	{
	public:
		MoreI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ELessI : public BinaryI
	{
	public:
		ELessI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class EMoreI : public BinaryI
	{
	public:
		EMoreI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class EqualsI : public BinaryI
	{
	public:
		EqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class NEqualsI : public BinaryI
	{
	public:
		NEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class AndI : public BinaryI
	{
	public:
		AndI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class OrI : public BinaryI
	{
	public:
		OrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class BOrI : public BinaryI
	{
	public:
		BOrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class BAndI : public BinaryI
	{
	public:
		BAndI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class BXOrI : public BinaryI
	{
	public:
		BXOrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class BShiftLeftI : public BinaryI
	{
	public:
		BShiftLeftI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class BShiftRightI : public BinaryI
	{
	public:
		BShiftRightI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class SetI : public BinaryI
	{
	protected:
		const bool isConst;

	public:
		SetI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ReturnI : public UnaryI
	{
	public:
		ReturnI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ExternI : public UnaryI
	{
	protected:
		const std::string id;
		boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)> f;

	public:
		ExternI(const std::string &, const std::shared_ptr<Instruction> &a, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class LengthI : public UnaryI
	{
	public:
		LengthI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class SizeI : public UnaryI
	{
	public:
		SizeI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
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
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class NewI : public BinaryI
	{
	public:
		NewI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class CastToI : public BinaryI
	{
	public:
		CastToI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class AllocI : public UnaryI
	{
	public:
		AllocI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class UntilI : public BinaryI
	{
	protected:
		const std::shared_ptr<Instruction> step;
		const bool inclusive;

	public:
		UntilI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ScopeI : public Instruction
	{
	protected:
		const std::vector<std::shared_ptr<Instruction>> children;

	public:
		ScopeI(const std::vector<std::shared_ptr<Instruction>> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class MapI : public Instruction
	{
	protected:
		const std::map<std::string, std::shared_ptr<Instruction>> children;

	public:
		MapI(const std::map<std::string, std::shared_ptr<Instruction>> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ReferI : public UnaryI
	{
	public:
		ReferI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
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
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class TryCatchI : public BinaryI
	{
	protected:
		const hash_ull key;

	public:
		TryCatchI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const hash_ull &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ThrowI : public UnaryI
	{
	public:
		ThrowI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class PureEqualsI : public BinaryI
	{
	public:
		PureEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class PureNEqualsI : public BinaryI
	{
	public:
		PureNEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class CharNI : public UnaryI
	{
	public:
		CharNI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class CharSI : public UnaryI
	{
	public:
		CharSI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class DeclareVarsI : public Instruction
	{
	protected:
		const std::vector<hash_ull> keys;

	public:
		DeclareVarsI(const std::vector<hash_ull> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class ParseI : public UnaryI
	{
	public:
		ParseI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};

	class TypeI : public UnaryI
	{
	public:
		TypeI(const std::shared_ptr<Instruction> &, const Token &);
		const Symbol evaluate(Scope *, std::vector<Function> &) const override;
	};
}