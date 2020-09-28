#ifndef PARSER_H
#define PARSER_H

#include "Ruota.h"

#include <boost/function.hpp>

class UnaryI : public Instruction
{
protected:
	Instruction *a;

public:
	UnaryI(I_TYPE, Instruction *, Token *);
	Instruction *getA() const;
	virtual ~UnaryI();
};

class CastingI : public Instruction
{
protected:
	hashcode_t key;

public:
	CastingI(I_TYPE, hashcode_t, Token *);
	hashcode_t getKey() const;
};

class BinaryI : public UnaryI
{
protected:
	Instruction *b;

public:
	BinaryI(I_TYPE, Instruction *, Instruction *, Token *);
	Instruction *getB() const;
	virtual ~BinaryI();
};

class Container : public Instruction
{
protected:
	Symbol d;

public:
	Container(const Symbol &d, Token *);
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
	DefineI(hashcode_t, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>>, std::shared_ptr<Instruction>, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class Sequence : public Instruction
{
protected:
	std::vector<Instruction *> children;

public:
	Sequence(std::vector<Instruction *>, Token *);
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
	IfElseI(Instruction *, Instruction *, Instruction *, Token *);
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
	WhileI(Instruction *, Instruction *, Token *);
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
	ForI(hashcode_t, Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~ForI();
};

class VariableI : public CastingI
{
public:
	VariableI(hashcode_t, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class DeclareI : public CastingI
{
public:
	DeclareI(hashcode_t, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CallI : public BinaryI
{
public:
	CallI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class AddI : public BinaryI
{
public:
	AddI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class SubI : public BinaryI
{
public:
	SubI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class MulI : public BinaryI
{
public:
	MulI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class DivI : public BinaryI
{
public:
	DivI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ModI : public BinaryI
{
public:
	ModI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class PowI : public BinaryI
{
public:
	PowI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class LessI : public BinaryI
{
public:
	LessI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class Equals : public BinaryI
{
public:
	Equals(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class NEquals : public BinaryI
{
public:
	NEquals(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class AndI : public BinaryI
{
public:
	AndI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class OrI : public BinaryI
{
public:
	OrI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BOrI : public BinaryI
{
public:
	BOrI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BAndI : public BinaryI
{
public:
	BAndI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BXOrI : public BinaryI
{
public:
	BXOrI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BShiftLeft : public BinaryI
{
public:
	BShiftLeft(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class BShiftRight : public BinaryI
{
public:
	BShiftRight(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class SetI : public BinaryI
{
public:
	SetI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ExternI : public UnaryI
{
protected:
	std::string id;
	boost::function<const Symbol(std::vector<Symbol>, Token*)> f;

public:
	ExternI(const std::string &, Instruction *a, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class LengthI : public UnaryI
{
public:
	LengthI(Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class SizeI : public UnaryI
{
public:
	SizeI(Instruction *, Token *);
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
	ClassI(hashcode_t, OBJECT_TYPE, std::shared_ptr<Instruction>, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	~ClassI();
};

class NewI : public BinaryI
{
public:
	NewI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CastToI : public UnaryI
{
protected:
	D_TYPE convert;

public:
	CastToI(Instruction *, D_TYPE, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class AllocI : public UnaryI
{
public:
	AllocI(Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class UntilI : public BinaryI
{
public:
	UntilI(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ScopeI : public Instruction
{
protected:
	std::vector<Instruction *> children;

public:
	ScopeI(std::vector<Instruction *>, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~ScopeI();
};

class MapI : public Instruction
{
protected:
	std::map<hashcode_t, Instruction *> children;

public:
	MapI(std::map<hashcode_t, Instruction *>, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~MapI();
};

class ReferI : public UnaryI
{
public:
	ReferI(Instruction *, Token *);
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
	SwitchI(Instruction *, std::map<Symbol, Instruction *>, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
	virtual ~SwitchI();
};

class TryCatchI : public BinaryI
{
protected:
	hashcode_t key;
public:
	TryCatchI(Instruction *, Instruction *, hashcode_t, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class ThrowI : public UnaryI
{
public:
	ThrowI(Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class PureEquals : public BinaryI
{
public:
	PureEquals(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class PureNEquals : public BinaryI
{
public:
	PureNEquals(Instruction *, Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CharNI : public UnaryI
{
public:
	CharNI(Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

class CharSI : public UnaryI
{
public:
	CharSI(Instruction *, Token *);
	const Symbol evaluate(Scope &) const override;
	const std::string toString(bool) const override;
};

#endif