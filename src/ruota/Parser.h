#ifndef PARSER_H
#define PARSER_H

#include "Ruota.h"

#include <boost/function.hpp>

class UnaryI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> a;

public:
	UnaryI(const InstructionType &, const std::shared_ptr<Instruction> &, const Token &);
	std::shared_ptr<Instruction> getA() const;
};

class CastingI : public Instruction
{
protected:
	const hashcode_t key;

public:
	CastingI(const InstructionType &, const hashcode_t &, const Token &);
	const hashcode_t getKey() const;
};

class BinaryI : public UnaryI
{
protected:
	const std::shared_ptr<Instruction> b;

public:
	BinaryI(const InstructionType &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	std::shared_ptr<Instruction> getB() const;
};

class ContainerI : public Instruction
{
protected:
	const Symbol d;

public:
	ContainerI(const Symbol &d, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class DefineI : public Instruction
{
protected:
	const hashcode_t key;
	const Signature ftype;
	const std::vector<std::pair<LexerTokenType, hashcode_t>> params;
	const std::shared_ptr<Instruction> body;

public:
	DefineI(const hashcode_t &, const Signature &, std::vector<std::pair<LexerTokenType, hashcode_t>>, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class SequenceI : public Instruction
{
protected:
	const std::vector<std::shared_ptr<Instruction>> children;

public:
	SequenceI(std::vector<std::shared_ptr<Instruction>>, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class IfElseI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> ifs;
	const std::shared_ptr<Instruction> body;
	const std::shared_ptr<Instruction> elses;

public:
	IfElseI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class WhileI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> whiles;
	const std::shared_ptr<Instruction> body;

public:
	WhileI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ForI : public Instruction
{
protected:
	const hashcode_t id;
	const std::shared_ptr<Instruction> fors;
	const std::shared_ptr<Instruction> body;

public:
	ForI(const hashcode_t &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class VariableI : public CastingI
{
public:
	VariableI(const hashcode_t &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class DeclareI : public CastingI
{
protected:
	const object_type_t vtype;
	const std::shared_ptr<Instruction> a;
	const bool isConst;

public:
	DeclareI(const hashcode_t &, const object_type_t &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class CallI : public BinaryI
{
public:
	CallI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class AddI : public BinaryI
{
public:
	AddI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class SubI : public BinaryI
{
public:
	SubI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class MulI : public BinaryI
{
public:
	MulI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class DivI : public BinaryI
{
public:
	DivI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ModI : public BinaryI
{
public:
	ModI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class PowI : public BinaryI
{
public:
	PowI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class LessI : public BinaryI
{
public:
	LessI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class EqualsI : public BinaryI
{
public:
	EqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class NEqualsI : public BinaryI
{
public:
	NEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class AndI : public BinaryI
{
public:
	AndI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class OrI : public BinaryI
{
public:
	OrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class BOrI : public BinaryI
{
public:
	BOrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class BAndI : public BinaryI
{
public:
	BAndI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class BXOrI : public BinaryI
{
public:
	BXOrI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class BShiftLeftI : public BinaryI
{
public:
	BShiftLeftI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class BShiftRightI : public BinaryI
{
public:
	BShiftRightI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class SetI : public BinaryI
{
protected:
const bool isConst;
public:
	SetI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ExternI : public UnaryI
{
protected:
	const std::string id;
	boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)> f;

public:
	ExternI(const std::string &, const std::shared_ptr<Instruction> &a, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class LengthI : public UnaryI
{
public:
	LengthI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class SizeI : public UnaryI
{
public:
	SizeI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ClassI : public Instruction
{
protected:
	const hashcode_t key;
	const ObjectType type;
	const std::shared_ptr<Instruction> body;
	const std::shared_ptr<Instruction> extends;

public:
	ClassI(hashcode_t, ObjectType, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class NewI : public BinaryI
{
public:
	NewI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class CastToI : public UnaryI
{
protected:
	ValueType convert;

public:
	CastToI(const std::shared_ptr<Instruction> &, const ValueType &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class AllocI : public UnaryI
{
public:
	AllocI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class UntilI : public BinaryI
{
protected:
	const std::shared_ptr<Instruction> step;
	const bool inclusive;

public:
	UntilI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const bool &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ScopeI : public Instruction
{
protected:
	const std::vector<std::shared_ptr<Instruction>> children;

public:
	ScopeI(std::vector<std::shared_ptr<Instruction>>, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class MapI : public Instruction
{
protected:
	const std::map<hashcode_t, std::shared_ptr<Instruction>> children;

public:
	MapI(std::map<hashcode_t, std::shared_ptr<Instruction>>, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ReferI : public UnaryI
{
public:
	ReferI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class SwitchI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> switchs;
	const std::map<Symbol, std::shared_ptr<Instruction>> cases_solved;
	const std::map<std::shared_ptr<Instruction>, std::shared_ptr<Instruction>> cases_unsolved;
	const std::shared_ptr<Instruction> elses;

public:
	SwitchI(const std::shared_ptr<Instruction> &, std::map<Symbol, std::shared_ptr<Instruction>>, std::map<std::shared_ptr<Instruction>, std::shared_ptr<Instruction>>, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class TryCatchI : public BinaryI
{
protected:
	const hashcode_t key;

public:
	TryCatchI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const hashcode_t &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class ThrowI : public UnaryI
{
public:
	ThrowI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class PureEqualsI : public BinaryI
{
public:
	PureEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class PureNEqualsI : public BinaryI
{
public:
	PureNEqualsI(const std::shared_ptr<Instruction> &, const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class CharNI : public UnaryI
{
public:
	CharNI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class CharSI : public UnaryI
{
public:
	CharSI(const std::shared_ptr<Instruction> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

class DeclareVarsI : public Instruction
{
protected:
	const std::vector<hashcode_t> keys;

public:
	DeclareVarsI(const std::vector<hashcode_t> &, const Token &);
	const Symbol evaluate(Scope *) const override;
};

#endif