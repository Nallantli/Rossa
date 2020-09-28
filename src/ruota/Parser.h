#ifndef PARSER_H
#define PARSER_H

#include "Ruota.h"

#include <boost/function.hpp>

class UnaryI : public Instruction
{
protected:
	Instruction *a;

public:
	UnaryI(I_TYPE, Instruction *);
	Instruction *getA() const;
	virtual ~UnaryI();
};

class CastingI : public Instruction
{
protected:
	hashcode_t key;

public:
	CastingI(I_TYPE, hashcode_t);
	hashcode_t getKey() const;
};

class BinaryI : public UnaryI
{
protected:
	Instruction *b;

public:
	BinaryI(I_TYPE, Instruction *, Instruction *);
	Instruction *getB() const;
	virtual ~BinaryI();
};

class Container : public Instruction
{
protected:
	Symbol d;

public:
	Container(const Symbol &d);
	const Symbol evaluate(Scope &) const override;
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
	DefineI(hashcode_t, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>>, std::shared_ptr<Instruction>);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class Sequence : public Instruction
{
protected:
	std::vector<Instruction *> children;

public:
	Sequence(std::vector<Instruction *>);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~Sequence();
};

class IfElseI : public Instruction
{
protected:
	Instruction *ifs;
	Instruction *body;
	Instruction *elses;

public:
	IfElseI(Instruction *, Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~IfElseI();
};

class WhileI : public Instruction
{
protected:
	Instruction *whiles;
	Instruction *body;

public:
	WhileI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~WhileI();
};

class ForI : public Instruction
{
protected:
	hashcode_t id;
	Instruction *fors;
	Instruction *body;

public:
	ForI(hashcode_t, Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~ForI();
};

class VariableI : public CastingI
{
public:
	VariableI(hashcode_t);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class DeclareI : public CastingI
{
public:
	DeclareI(hashcode_t);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CallI : public BinaryI
{
public:
	CallI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class AddI : public BinaryI
{
public:
	AddI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class SubI : public BinaryI
{
public:
	SubI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class MulI : public BinaryI
{
public:
	MulI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class DivI : public BinaryI
{
public:
	DivI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ModI : public BinaryI
{
public:
	ModI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class PowI : public BinaryI
{
public:
	PowI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class LessI : public BinaryI
{
public:
	LessI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class Equals : public BinaryI
{
public:
	Equals(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class NEquals : public BinaryI
{
public:
	NEquals(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class AndI : public BinaryI
{
public:
	AndI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class OrI : public BinaryI
{
public:
	OrI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BOrI : public BinaryI
{
public:
	BOrI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BAndI : public BinaryI
{
public:
	BAndI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BXOrI : public BinaryI
{
public:
	BXOrI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BShiftLeft : public BinaryI
{
public:
	BShiftLeft(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BShiftRight : public BinaryI
{
public:
	BShiftRight(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class SetI : public BinaryI
{
public:
	SetI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ExternI : public UnaryI
{
protected:
	std::string id;
	boost::function<const Symbol(std::vector<Symbol>)> f;

public:
	ExternI(const std::string &, Instruction *a);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class LengthI : public UnaryI
{
public:
	LengthI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class SizeI : public UnaryI
{
public:
	SizeI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ClassI : public Instruction
{
protected:
	hashcode_t key;
	OBJECT_TYPE type;
	std::shared_ptr<Instruction> body;
	Instruction * extends;

public:
	ClassI(hashcode_t, OBJECT_TYPE, std::shared_ptr<Instruction>, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	~ClassI();
};

class NewI : public BinaryI
{
public:
	NewI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CastToI : public UnaryI
{
protected:
	D_TYPE convert;

public:
	CastToI(Instruction *, D_TYPE);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class AllocI : public UnaryI
{
public:
	AllocI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class UntilI : public BinaryI
{
public:
	UntilI(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ScopeI : public Instruction
{
protected:
	std::vector<Instruction *> children;

public:
	ScopeI(std::vector<Instruction *>);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~ScopeI();
};

class MapI : public Instruction
{
protected:
	std::map<hashcode_t, Instruction *> children;

public:
	MapI(std::map<hashcode_t, Instruction *>);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~MapI();
};

class ReferI : public UnaryI
{
public:
	ReferI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class SwitchI : public Instruction
{
protected:
	Instruction * switchs;
	Instruction * elses;
	std::map<Symbol, Instruction *> cases;

public:
	SwitchI(Instruction *, std::map<Symbol, Instruction *>, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~SwitchI();
};

class TryCatchI : public BinaryI
{
protected:
	hashcode_t key;
public:
	TryCatchI(Instruction *, Instruction *, hashcode_t);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ThrowI : public UnaryI
{
public:
	ThrowI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class PureEquals : public BinaryI
{
public:
	PureEquals(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class PureNEquals : public BinaryI
{
public:
	PureNEquals(Instruction *, Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CharNI : public UnaryI
{
public:
	CharNI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CharSI : public UnaryI
{
public:
	CharSI(Instruction *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

#endif