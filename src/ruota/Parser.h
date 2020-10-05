#ifndef PARSER_H
#define PARSER_H

#include "Ruota.h"

#include <boost/function.hpp>

class UnaryI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> a;

public:
	UnaryI(InstructionType, std::shared_ptr<Instruction>, const Token);
	std::shared_ptr<Instruction> getA() const;
};

class CastingI : public Instruction
{
protected:
	const hashcode_t key;

public:
	CastingI(InstructionType, hashcode_t, const Token);
	const hashcode_t getKey() const;
};

class BinaryI : public UnaryI
{
protected:
	const std::shared_ptr<Instruction> b;

public:
	BinaryI(InstructionType, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	std::shared_ptr<Instruction> getB() const;
};

class ContainerI : public Instruction
{
protected:
	const Symbol d;

public:
	ContainerI(const Symbol &d, const Token);
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
	DefineI(hashcode_t, Signature ftype, std::vector<std::pair<LexerTokenType, hashcode_t>>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class SequenceI : public Instruction
{
protected:
	const std::vector<std::shared_ptr<Instruction>> children;

public:
	SequenceI(std::vector<std::shared_ptr<Instruction>>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class IfElseI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> ifs;
	const std::shared_ptr<Instruction> body;
	const std::shared_ptr<Instruction> elses;

public:
	IfElseI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class WhileI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> whiles;
	const std::shared_ptr<Instruction> body;

public:
	WhileI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ForI : public Instruction
{
protected:
	const hashcode_t id;
	const std::shared_ptr<Instruction> fors;
	const std::shared_ptr<Instruction> body;

public:
	ForI(hashcode_t, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class VariableI : public CastingI
{
public:
	VariableI(hashcode_t, const Token);
	const Symbol evaluate(Scope *) const override;
};

class DeclareI : public CastingI
{
public:
	DeclareI(hashcode_t, const Token);
	const Symbol evaluate(Scope *) const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class CallI : public BinaryI
{
public:
	CallI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class AddI : public BinaryI
{
public:
	AddI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class SubI : public BinaryI
{
public:
	SubI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class MulI : public BinaryI
{
public:
	MulI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class DivI : public BinaryI
{
public:
	DivI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ModI : public BinaryI
{
public:
	ModI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class PowI : public BinaryI
{
public:
	PowI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class LessI : public BinaryI
{
public:
	LessI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class EqualsI : public BinaryI
{
public:
	EqualsI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class NEqualsI : public BinaryI
{
public:
	NEqualsI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class AndI : public BinaryI
{
public:
	AndI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class OrI : public BinaryI
{
public:
	OrI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class BOrI : public BinaryI
{
public:
	BOrI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class BAndI : public BinaryI
{
public:
	BAndI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class BXOrI : public BinaryI
{
public:
	BXOrI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class BShiftLeftI : public BinaryI
{
public:
	BShiftLeftI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class BShiftRightI : public BinaryI
{
public:
	BShiftRightI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class SetI : public BinaryI
{
public:
	SetI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ExternI : public UnaryI
{
protected:
	const std::string id;
	boost::function<const Symbol(std::vector<Symbol>, const Token *)> f;

public:
	ExternI(const std::string &, std::shared_ptr<Instruction> a, const Token);
	const Symbol evaluate(Scope *) const override;
};

class LengthI : public UnaryI
{
public:
	LengthI(std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class SizeI : public UnaryI
{
public:
	SizeI(std::shared_ptr<Instruction>, const Token);
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
	ClassI(hashcode_t, ObjectType, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class NewI : public BinaryI
{
public:
	NewI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class CastToI : public UnaryI
{
protected:
	ValueType convert;

public:
	CastToI(std::shared_ptr<Instruction>, ValueType, const Token);
	const Symbol evaluate(Scope *) const override;
};

class AllocI : public UnaryI
{
public:
	AllocI(std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class UntilI : public BinaryI
{
protected:
	const bool inclusive;
	const std::shared_ptr<Instruction> step;

public:
	UntilI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, bool, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ScopeI : public Instruction
{
protected:
	const std::vector<std::shared_ptr<Instruction>> children;

public:
	ScopeI(std::vector<std::shared_ptr<Instruction>>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class MapI : public Instruction
{
protected:
	const std::map<hashcode_t, std::shared_ptr<Instruction>> children;

public:
	MapI(std::map<hashcode_t, std::shared_ptr<Instruction>>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ReferI : public UnaryI
{
public:
	ReferI(std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class SwitchI : public Instruction
{
protected:
	const std::shared_ptr<Instruction> switchs;
	const std::shared_ptr<Instruction> elses;
	const std::map<Symbol, std::shared_ptr<Instruction>> cases;

public:
	SwitchI(std::shared_ptr<Instruction>, std::map<Symbol, std::shared_ptr<Instruction>>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class TryCatchI : public BinaryI
{
protected:
	const hashcode_t key;

public:
	TryCatchI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, hashcode_t, const Token);
	const Symbol evaluate(Scope *) const override;
};

class ThrowI : public UnaryI
{
public:
	ThrowI(std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class PureEqualsI : public BinaryI
{
public:
	PureEqualsI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class PureNEqualsI : public BinaryI
{
public:
	PureNEqualsI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class CharNI : public UnaryI
{
public:
	CharNI(std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

class CharSI : public UnaryI
{
public:
	CharSI(std::shared_ptr<Instruction>, const Token);
	const Symbol evaluate(Scope *) const override;
};

#endif