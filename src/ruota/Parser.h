#ifndef PARSER_H
#define PARSER_H

#include "Ruota.h"

#include <boost/function.hpp>

class UnaryI : public Instruction
{
protected:
	std::shared_ptr<Instruction> a;

public:
	UnaryI(I_TYPE, std::shared_ptr<Instruction>, const Token *);
	std::shared_ptr<Instruction> getA() const;
};

class CastingI : public Instruction
{
protected:
	hashcode_t key;

public:
	CastingI(I_TYPE, hashcode_t, const Token *);
	hashcode_t getKey() const;
};

class BinaryI : public UnaryI
{
protected:
	std::shared_ptr<Instruction> b;

public:
	BinaryI(I_TYPE, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	std::shared_ptr<Instruction> getB() const;
};

class Container : public Instruction
{
protected:
	Symbol d;

public:
	Container(const Symbol &d, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class DefineI : public Instruction
{
protected:
	hashcode_t key;
	D_TYPE ftype;
	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params;
	std::shared_ptr<Instruction> body;

public:
	DefineI(hashcode_t, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class Sequence : public Instruction
{
protected:
	std::vector<std::shared_ptr<Instruction>> children;

public:
	Sequence(std::vector<std::shared_ptr<Instruction>>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class IfElseI : public Instruction
{
protected:
	std::shared_ptr<Instruction> ifs;
	std::shared_ptr<Instruction> body;
	std::shared_ptr<Instruction> elses;

public:
	IfElseI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class WhileI : public Instruction
{
protected:
	std::shared_ptr<Instruction> whiles;
	std::shared_ptr<Instruction> body;

public:
	WhileI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ForI : public Instruction
{
protected:
	hashcode_t id;
	std::shared_ptr<Instruction> fors;
	std::shared_ptr<Instruction> body;

public:
	ForI(hashcode_t, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class VariableI : public CastingI
{
public:
	VariableI(hashcode_t, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class DeclareI : public CastingI
{
public:
	DeclareI(hashcode_t, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class CallI : public BinaryI
{
public:
	CallI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class AddI : public BinaryI
{
public:
	AddI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class SubI : public BinaryI
{
public:
	SubI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class MulI : public BinaryI
{
public:
	MulI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class DivI : public BinaryI
{
public:
	DivI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ModI : public BinaryI
{
public:
	ModI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class PowI : public BinaryI
{
public:
	PowI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class LessI : public BinaryI
{
public:
	LessI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class Equals : public BinaryI
{
public:
	Equals(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class NEquals : public BinaryI
{
public:
	NEquals(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class AndI : public BinaryI
{
public:
	AndI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class OrI : public BinaryI
{
public:
	OrI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class BOrI : public BinaryI
{
public:
	BOrI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class BAndI : public BinaryI
{
public:
	BAndI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class BXOrI : public BinaryI
{
public:
	BXOrI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class BShiftLeft : public BinaryI
{
public:
	BShiftLeft(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class BShiftRight : public BinaryI
{
public:
	BShiftRight(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class SetI : public BinaryI
{
public:
	SetI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ExternI : public UnaryI
{
protected:
	std::string id;
	boost::function<const Symbol(std::vector<Symbol>, const Token *)> f;

public:
	ExternI(const std::string &, std::shared_ptr<Instruction> a, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class LengthI : public UnaryI
{
public:
	LengthI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class SizeI : public UnaryI
{
public:
	SizeI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ClassI : public Instruction
{
protected:
	hashcode_t key;
	OBJECT_TYPE type;
	std::shared_ptr<Instruction> body;
	std::shared_ptr<Instruction> extends;

public:
	ClassI(hashcode_t, OBJECT_TYPE, std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class NewI : public BinaryI
{
public:
	NewI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class CastToI : public UnaryI
{
protected:
	D_TYPE convert;

public:
	CastToI(std::shared_ptr<Instruction>, D_TYPE, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class AllocI : public UnaryI
{
public:
	AllocI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class UntilI : public BinaryI
{
public:
	UntilI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ScopeI : public Instruction
{
protected:
	std::vector<std::shared_ptr<Instruction>> children;

public:
	ScopeI(std::vector<std::shared_ptr<Instruction>>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class MapI : public Instruction
{
protected:
	std::map<hashcode_t, std::shared_ptr<Instruction>> children;

public:
	MapI(std::map<hashcode_t, std::shared_ptr<Instruction>>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ReferI : public UnaryI
{
public:
	ReferI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class SwitchI : public Instruction
{
protected:
	std::shared_ptr<Instruction> switchs;
	std::shared_ptr<Instruction> elses;
	std::map<Symbol, std::shared_ptr<Instruction>> cases;

public:
	SwitchI(std::shared_ptr<Instruction>, std::map<Symbol, std::shared_ptr<Instruction>>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class TryCatchI : public BinaryI
{
protected:
	hashcode_t key;

public:
	TryCatchI(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, hashcode_t, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class ThrowI : public UnaryI
{
public:
	ThrowI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class PureEquals : public BinaryI
{
public:
	PureEquals(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class PureNEquals : public BinaryI
{
public:
	PureNEquals(std::shared_ptr<Instruction>, std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class CharNI : public UnaryI
{
public:
	CharNI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

class CharSI : public UnaryI
{
public:
	CharSI(std::shared_ptr<Instruction>, const Token *);
	const Symbol evaluate(Scope *) const override;
	const std::string toString(bool) const override;
};

#endif