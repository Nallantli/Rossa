#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "../rossa.h"
#include "../symbol/symbol.h"
#include "../object/object.h"
#include "../signature/signature.h"
#include "../function/function.h"

class Instruction
{
protected:
	const token_t token;
	const instruction_type_enum type;

public:
	Instruction(const instruction_type_enum &, const token_t &);
	virtual const symbol_t evaluate(const object_t *, trace_t &) const = 0;
	const instruction_type_enum getType() const;
	virtual ~Instruction();
};

/**
 * Unary Prototype
 */
class UnaryI : public Instruction
{
protected:
	const ptr_instruction_t a;

public:
	UnaryI(const instruction_type_enum &, const ptr_instruction_t &, const token_t &);
	const ptr_instruction_t getA() const;
};

/**
 * Prototype for instructions that operate on a key value
 */
class CastingI : public Instruction
{
protected:
	const hash_ull key;

public:
	CastingI(const instruction_type_enum &, const hash_ull &, const token_t &);
	const hash_ull getKey() const;
};

/**
 * Binary Protoype
 */
class BinaryI : public UnaryI
{
protected:
	const ptr_instruction_t b;

public:
	BinaryI(const instruction_type_enum &, const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const ptr_instruction_t getB() const;
};

/**
 * Raw value instruction (push/move)
 */
class ContainerI : public Instruction
{
protected:
	const symbol_t d;

public:
	ContainerI(const symbol_t &d, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Define and set Function
 */
class DefineI : public Instruction
{
protected:
	const hash_ull key;
	const signature_t ftype;
	const std::vector<std::pair<token_type_enum, hash_ull>> params;
	const ptr_instruction_t body;
	const std::vector<hash_ull> captures;

public:
	DefineI(const hash_ull &, const signature_t &, const std::vector<std::pair<token_type_enum, hash_ull>> &, const ptr_instruction_t &, const std::vector<hash_ull> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Define and set Function with variable arguments
 */
class VargDefineI : public Instruction
{
protected:
	const hash_ull key;
	const ptr_instruction_t body;
	const std::vector<hash_ull> captures;

public:
	VargDefineI(const hash_ull &, const ptr_instruction_t &, const std::vector<hash_ull> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Sequential instructions
 * `<SEQ>`
 */
class SequenceI : public Instruction
{
protected:
	const std::vector<ptr_instruction_t> children;

public:
	SequenceI(const std::vector<ptr_instruction_t> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * If-Then-Else Instruction
 * `if <EXPR> then { <SEQ> } (elif <EXPR> then { <SEQ> })* (else { <SEQ> })`
 */
class IfElseI : public Instruction
{
protected:
	const ptr_instruction_t ifs;
	const ptr_instruction_t body;
	const ptr_instruction_t elses;

public:
	IfElseI(const ptr_instruction_t &, const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * While-Do Instruction
 * `while <EXPR> do { <SEQ> }`
 */
class WhileI : public Instruction
{
protected:
	const ptr_instruction_t whiles;
	const std::vector<ptr_instruction_t> body;

public:
	WhileI(const ptr_instruction_t &, const std::vector<ptr_instruction_t> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * For loop instruction
 * `for <VAR> in <EXPR> do { <SEQ> }`
 */
class ForI : public Instruction
{
protected:
	const hash_ull id;
	const ptr_instruction_t fors;
	const std::vector<ptr_instruction_t> body;

public:
	ForI(const hash_ull &, const ptr_instruction_t &, const std::vector<ptr_instruction_t> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Variable push/move
 * `<VAR>`
 */
class VariableI : public CastingI
{
public:
	VariableI(const hash_ull &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Fetch `this` of most local object
 * `this`
 */
class GetThisI : public CastingI
{
public:
	GetThisI(const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Declare and set variable
 * `<VAR> := <EXPR>`
 */
class DeclareI : public CastingI
{
protected:
	const ptr_instruction_t a;
	const bool isConst;

public:
	DeclareI(const hash_ull &, const ptr_instruction_t &, const bool &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Index instruction
 * `<EXPR> [ <EXPR> ]`
 */
class IndexI : public BinaryI
{
public:
	IndexI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Enter interior scope
 * `<EXPR> . <EXPR>`
 */
class InnerI : public BinaryI
{
public:
	InnerI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Call function
 * `<EXPR> ( <EXPR> (, <EXPR>)* )`
 */
class CallI : public BinaryI
{
public:
	CallI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Addition
 * `<EXPR> + <EXPR>`
 */
class AddI : public BinaryI
{
public:
	AddI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Subtraction
 * `<EXPR> - <EXPR>`
 */
class SubI : public BinaryI
{
public:
	SubI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Multiplication
 * `<EXPR> * <EXPR>`
 */
class MulI : public BinaryI
{
public:
	MulI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Division
 * `<EXPR> / <EXPR>`
 */
class DivI : public BinaryI
{
public:
	DivI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Modulo
 * `<EXPR> % <EXPR>`
 */
class ModI : public BinaryI
{
public:
	ModI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Exponentiation
 * `<EXPR> ** <EXPR>`
 */
class PowI : public BinaryI
{
public:
	PowI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Less-than
 * `<EXPR> < <EXPR>`
 */
class LessI : public BinaryI
{
public:
	LessI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * More-than
 * `<EXPR> > <EXPR>`
 */
class MoreI : public BinaryI
{
public:
	MoreI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Less-than or Equal
 * `<EXPR> <= <EXPR>`
 */
class ELessI : public BinaryI
{
public:
	ELessI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Mode-than or Equal
 * `<EXPR> >= <EXPR>`
 */
class EMoreI : public BinaryI
{
public:
	EMoreI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Equal
 * `<EXPR> == <EXPR>`
 */
class EqualsI : public BinaryI
{
public:
	EqualsI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Not Equal
 * `<EXPR> != <EXPR>`
 */
class NEqualsI : public BinaryI
{
public:
	NEqualsI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Boolean And
 * `<EXPR> && <EXPR>`
 */
class AndI : public BinaryI
{
public:
	AndI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Boolean Or
 * `<EXPR> || <EXPR>
 */
class OrI : public BinaryI
{
public:
	OrI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Binary Or
 * `<EXPR> | <EXPR>`
 */
class BOrI : public BinaryI
{
public:
	BOrI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Binary And
 * `<EXPR> & <EXPR>`
 */
class BAndI : public BinaryI
{
public:
	BAndI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Binary X-Or
 * `<EXPR> ^ <EXPR>`
 */
class BXOrI : public BinaryI
{
public:
	BXOrI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Left Bit Shift
 * `<EXPR> << <EXPR>`
 */
class BShiftLeftI : public BinaryI
{
public:
	BShiftLeftI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Right Bit Shift
 * `<EXPR> >> <EXPR>`
 */
class BShiftRightI : public BinaryI
{
public:
	BShiftRightI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Binary Not
 * `~ <EXPR>`
 */
class BNotI : public UnaryI
{
public:
	BNotI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Set Value
 * `<EXPR> = <EXPR>`
 */
class SetI : public BinaryI
{
public:
	SetI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Return value to most local function frame
 * `return <EXPR>`
 */
class ReturnI : public UnaryI
{
public:
	ReturnI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Call a function loaded from external library
 * `extern <LIB> . <FUNC> ( (<EXPR> (, <EXPR>)*) )`
 */
class ExternI : public UnaryI
{
protected:
	const std::string libname;
	const std::string fname;
	extf_t f;

public:
	ExternI(const std::string &, const std::string &, const ptr_instruction_t &a, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Get Length
 * `len( <EXPR> )`
 * `<EXPR> . len()`
 */
class LengthI : public UnaryI
{
public:
	LengthI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Declare object
 * `struct|static|virtual <NAME> (: <NAME>) { <SEQ> }`
 */
class ClassI : public Instruction
{
protected:
	const hash_ull key;
	const scope_type_enum type;
	const ptr_instruction_t body;
	const ptr_instruction_t extends;

public:
	ClassI(const hash_ull &, const scope_type_enum &, const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Instantiate object and call `init()`
 * `new <NAME> ( (<EXPR> (, <EXPR>)*) )`
 */
class NewI : public BinaryI
{
public:
	NewI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Cast to type
 * `<EXPR> -> <EXPR>`
 */
class CastToI : public BinaryI
{
public:
	CastToI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Allocate array size
 * `alloc( <EXPR> )`
 */
class AllocI : public UnaryI
{
public:
	AllocI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Range [a ... b)
 * `<EXPR> .. <EXPR>`
 *  or [a ... b]
 * `<EXPR> .+ <EXPR>`
 */
class UntilI : public BinaryI
{
protected:
	const ptr_instruction_t step;
	const bool inclusive;

public:
	UntilI(const ptr_instruction_t &, const ptr_instruction_t &, const ptr_instruction_t &, const bool &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Execute sequence and downscope
 * <SEQ>
 */
class ScopeI : public Instruction
{
protected:
	const std::vector<ptr_instruction_t> children;

public:
	ScopeI(const std::vector<ptr_instruction_t> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Create Dictionary value
 * `{ (<STRING> : <EXPR>)* }`
 */
class MapI : public Instruction
{
protected:
	const std::map<std::string, ptr_instruction_t> children;

public:
	MapI(const std::map<std::string, ptr_instruction_t> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Return value as reference
 * `refer <EXPR>`
 */
class ReferI : public UnaryI
{
public:
	ReferI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Switch statement
 * `switch <EXPR> of { case <CONST> do { <SEQ> }|<EXPR>; }`
 */
class SwitchI : public Instruction
{
protected:
	const ptr_instruction_t switchs;
	const std::map<symbol_t, size_t> cases_solved;
	const std::map<ptr_instruction_t, size_t> cases_unsolved;
	const std::vector<ptr_instruction_t> cases;
	const ptr_instruction_t elses;

public:
	SwitchI(const ptr_instruction_t &, const std::map<symbol_t, size_t> &, const std::map<ptr_instruction_t, size_t> &, const std::vector<ptr_instruction_t> &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Try-Catch
 * `try { <SEQ> } catch <VAR> then { <SEQ> }`
 */
class TryCatchI : public BinaryI
{
protected:
	const hash_ull key;

public:
	TryCatchI(const ptr_instruction_t &, const ptr_instruction_t &, const hash_ull &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

class ThrowI : public UnaryI
{
public:
	ThrowI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Object equals by address
 * `<EXPR> === <EXPR>`
 */
class PureEqualsI : public BinaryI
{
public:
	PureEqualsI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Object not equal by address
 * `<EXPR> !=== <EXPR>`
 */
class PureNEqualsI : public BinaryI
{
public:
	PureNEqualsI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * String to sequence of character values
 * `codes( <EXPR> )`
 */
class CharNI : public UnaryI
{
public:
	CharNI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Character values to String
 * `chars( <EXPR> )`
 */
class CharSI : public UnaryI
{
public:
	CharSI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Mass declare
 * `var <VAR> (, <VAR>)*
 */
class DeclareVarsI : public Instruction
{
protected:
	const std::vector<hash_ull> keys;

public:
	DeclareVarsI(const std::vector<hash_ull> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Parse at runtime
 * `parse( <EXPR> )`
 */
class ParseI : public UnaryI
{
public:
	ParseI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Get value type
 * `$ <EXPR>`
 */
class TypeI : public UnaryI
{
public:
	TypeI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Execute operation by index
 * `call_op <NUM> ( <EXPR> )
 */
class CallOpI : public Instruction
{
protected:
	const size_t id;
	const std::vector<ptr_instruction_t> children;

public:
	CallOpI(const size_t &, const std::vector<ptr_instruction_t> &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Delete key from dictionary
 * `<EXPR> delete <EXPR>
 */
class DeleteI : public BinaryI
{
public:
	DeleteI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Unary addition (superfluous)
 * `+ <EXPR>`
 */
class UnAddI : public UnaryI
{
public:
	UnAddI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Unary negation
 * `- <EXPR>`
 */
class NegI : public UnaryI
{
public:
	NegI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Unary Boolean Negation
 * `! <EXPR>`
 */
class NotI : public UnaryI
{
public:
	NotI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Concatenation
 * `<EXPR> ++ <EXPR>
 */
class ConcatI : public BinaryI
{
public:
	ConcatI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Set all indices of array
 * `<EXPR> .= <EXPR>`
 */
class SetIndexI : public BinaryI
{
public:
	SetIndexI(const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Get hash value
 * `@ <EXPR>`
 */
class HashI : public UnaryI
{
public:
	HashI(const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

/**
 * Each loop instruction
 * `each <VAR> in <EXPR> (where <EXPR>) do <EXPR>`
 */
class EachI : public Instruction
{
protected:
	const hash_ull id;
	const ptr_instruction_t eachs;
	const ptr_instruction_t wheres;
	const ptr_instruction_t body;

public:
	EachI(const hash_ull &, const ptr_instruction_t &, const ptr_instruction_t &, const ptr_instruction_t &, const token_t &);
	const symbol_t evaluate(const object_t *, trace_t &) const override;
};

#endif