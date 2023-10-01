#include "instruction.h"

#include "../rossa_error/rossa_error.h"
#include "../parameter/parameter.h"
#include "../operation/operation.h"
#include "../global/global.h"
#include "../node/node.h"
#include "../node_parser/node_parser.h"
#include "../parser/parser.h"

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(const instruction_type_enum &type, const token_t &token)
	: token{token}, type{type}
{
}

const instruction_type_enum Instruction::getType() const
{
	return type;
}

Instruction::~Instruction()
{
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(const instruction_type_enum &type, const ptr_instruction_t &a, const token_t &token)
	: Instruction(type, token), a{a}
{
}

const ptr_instruction_t UnaryI::getA() const
{
	return a;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastingI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

CastingI::CastingI(const instruction_type_enum &type, const hash_ull &key, const token_t &token)
	: Instruction(type, token), key{key}
{
}

const hash_ull CastingI::getKey() const
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(const instruction_type_enum &type, const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: UnaryI(type, a, token), b{b}
{
}

const ptr_instruction_t BinaryI::getB() const
{
	return b;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ContainerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ContainerI::ContainerI(const symbol_t &d, const token_t &token)
	: Instruction(CONTAINER, token), d{d}
{
}

const symbol_t ContainerI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return d;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(const hash_ull &key, const signature_t &ftype, const std::vector<std::pair<token_type_enum, hash_ull>> &params, const ptr_instruction_t &body, const std::vector<hash_ull> &captures, const token_t &token)
	: Instruction(DEFINE, token), key{key}, ftype{ftype}, params{params}, body{body}, captures{captures}
{
}

const symbol_t DefineI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	std::map<const hash_ull, const symbol_t> capturedVars;
	for (const hash_ull &e : captures)
	{
		capturedVars[e].set(&scope->getVariable(e, &token, stack_trace), &token, stack_trace);
	}
	ptr_function_t f = std::make_shared<function_t>(key, scope->getPtr(), params, body, capturedVars);
	if (key > 0)
	{
		return scope->createVariable(key, symbol_t::FunctionSIG(ftype, f), &token);
	}
	return symbol_t::FunctionSIG(ftype, f);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VargDefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VargDefineI::VargDefineI(const hash_ull &key, const ptr_instruction_t &body, const std::vector<hash_ull> &captures, const token_t &token)
	: Instruction(VARG_DEFINE, token), key{key}, body{body}, captures{captures}
{
}

const symbol_t VargDefineI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	std::map<const hash_ull, const symbol_t> capturedVars;
	for (const hash_ull &e : captures)
	{
		capturedVars[e].set(&scope->getVariable(e, &token, stack_trace), &token, stack_trace);
	}
	ptr_function_t f = std::make_shared<function_t>(key, scope->getPtr(), body, capturedVars);
	if (key > 0)
	{
		return scope->createVariable(key, symbol_t::FunctionVARG(static_cast<ptr_function_t>(f)), &token);
	}
	return symbol_t::FunctionVARG(static_cast<ptr_function_t>(f));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SequenceI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SequenceI::SequenceI(const std::vector<ptr_instruction_t> &children, const token_t &token)
	: Instruction(SEQUENCE, token), children{children}
{
}

const symbol_t SequenceI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	std::vector<symbol_t> evals;
	for (const ptr_instruction_t &e : children)
	{
		if (e->getType() == UNTIL_I || e->getType() == EACH_I)
		{
			std::vector<symbol_t> v = e->evaluate(scope, stack_trace).getVector(&token, stack_trace);
			evals.insert(evals.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
		}
		else
		{
			evals.push_back(e->evaluate(scope, stack_trace));
		}
	}
	return symbol_t::Array(evals);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IFElseI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

IfElseI::IfElseI(const ptr_instruction_t &ifs, const ptr_instruction_t &body, const ptr_instruction_t &elses, const token_t &token)
	: Instruction(IFELSE, token), ifs{ifs}, body{body}, elses{elses}
{
}

const symbol_t IfElseI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const object_t newScope(scope, 0);
	if (ifs->evaluate(&newScope, stack_trace).getBool(&token, stack_trace))
	{
		return body->evaluate(&newScope, stack_trace);
	}
	else if (elses)
	{
		return elses->evaluate(&newScope, stack_trace);
	}
	return symbol_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class WhileI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

WhileI::WhileI(const ptr_instruction_t &whiles, const std::vector<ptr_instruction_t> &body, const token_t &token)
	: Instruction(WHILE, token), whiles{whiles}, body{body}
{
}

const symbol_t WhileI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	while (whiles->evaluate(scope, stack_trace).getBool(&token, stack_trace))
	{
		const object_t newScope(scope, 0);
		for (const ptr_instruction_t &i : body)
		{
			const symbol_t temp = i->evaluate(&newScope, stack_trace);
			bool cflag = false;
			switch (temp.getSymbolType())
			{
			case symbol_t::type_t::ID_REFER:
			case symbol_t::type_t::ID_RETURN:
				return temp;
			case symbol_t::type_t::ID_BREAK:
				return symbol_t();
			case symbol_t::type_t::ID_CONTINUE:
				cflag = true;
				break;
			default:
				break;
			}
			if (cflag)
				break;
		}
	}
	return symbol_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ForI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ForI::ForI(const hash_ull &id, const ptr_instruction_t &fors, const std::vector<ptr_instruction_t> &body, const token_t &token)
	: Instruction(FOR, token), id{id}, fors{fors}, body{body}
{
}

const symbol_t ForI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const std::vector<symbol_t> evalFor = fors->evaluate(scope, stack_trace).getVector(&token, stack_trace);
	for (const symbol_t &e : evalFor)
	{
		const object_t newScope(scope, 0);
		newScope.createVariable(id, e, &token);
		bool cflag = false;
		for (const ptr_instruction_t &i : body)
		{
			const symbol_t temp = i->evaluate(&newScope, stack_trace);
			switch (temp.getSymbolType())
			{
			case symbol_t::type_t::ID_REFER:
			case symbol_t::type_t::ID_RETURN:
				return temp;
			case symbol_t::type_t::ID_BREAK:
				return symbol_t();
			case symbol_t::type_t::ID_CONTINUE:
				cflag = true;
				break;
			default:
				break;
			}
			if (cflag)
				break;
		}
	}
	return symbol_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(const hash_ull &key, const token_t &token)
	: CastingI(VARIABLE, key, token)
{
}

const symbol_t VariableI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return scope->getVariable(key, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class GetThisI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

GetThisI::GetThisI(const token_t &token)
	: CastingI(GET_THIS_I, key, token)
{
}

const symbol_t GetThisI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return scope->getThis(&token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareI::DeclareI(const hash_ull &key, const ptr_instruction_t &a, const bool &isConst, const token_t &token)
	: CastingI(DECLARE, key, token), a{a}, isConst{isConst}
{
}

const symbol_t DeclareI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t v = scope->createVariable(key, &token);
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	if (scope->hasValue(parser_t::HASH_SET) && evalA.getValueType() == value_type_enum::OBJECT)
	{
		ptr_function_t f = nullptr;
		try
		{
			f = scope->getVariable(parser_t::HASH_SET, &token, stack_trace).getFunction({v, evalA}, &token, stack_trace);
		}
		catch (const rossa_error_t &e)
		{
		}
		if (f)
		{
			function_evaluate(f, {v, evalA}, &token, stack_trace);
			return v;
		}
	}
	v.set(&evalA, &token, stack_trace);
	return v;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(INDEX, a, b, token)
{
}

const symbol_t IndexI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::index(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(INNER, a, b, token)
{
}

const symbol_t InnerI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType())
	{
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(&token, stack_trace);
		if (o->getType() != scope_type_enum::SCOPE_STATIC && o->getType() != scope_type_enum::SCOPE_INSTANCE)
			throw rossa_error_t(_CANNOT_INDEX_OBJECT_, token, stack_trace);
		return b->evaluate(o, stack_trace);
	}
	default:
		throw rossa_error_t(_CANNOT_INDEX_VALUE_, token, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

CallI::CallI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(INDEX, a, b, token)
{
}

const symbol_t CallI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return operation::call(scope, a, b->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AddI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AddI::AddI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(ADD, a, b, token)
{
}

const symbol_t AddI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::add(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SubI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SubI::SubI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(SUB, a, b, token)
{
}

const symbol_t SubI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::sub(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MulI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MulI::MulI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(MUL, a, b, token)
{
}

const symbol_t MulI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::mul(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DivI::DivI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(DIV, a, b, token)
{
}

const symbol_t DivI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::div(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ModI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ModI::ModI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(MOD, a, b, token)
{
}

const symbol_t ModI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::mod(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(POW_I, a, b, token)
{
}

const symbol_t PowI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::pow(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(LESS, a, b, token)
{
}

const symbol_t LessI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::less(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(MORE, a, b, token)
{
}

const symbol_t MoreI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::more(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(ELESS, a, b, token)
{
}

const symbol_t ELessI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::eless(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(EMORE, a, b, token)
{
}

const symbol_t EMoreI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::emore(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EqualsI::EqualsI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(EQUALS, a, b, token)
{
}

const symbol_t EqualsI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == value_type_enum::OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(parser_t::HASH_EQUALS))
		return scope->getVariable(parser_t::HASH_EQUALS, &token, stack_trace).call({evalA, evalB}, &token, stack_trace);

	return symbol_t::Boolean(evalA.equals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

NEqualsI::NEqualsI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(NEQUALS, a, b, token)
{
}

const symbol_t NEqualsI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == value_type_enum::OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(parser_t::HASH_NEQUALS))
		return scope->getVariable(parser_t::HASH_NEQUALS, &token, stack_trace).call({evalA, evalB}, &token, stack_trace);

	return symbol_t::Boolean(evalA.nequals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(AND, a, b, token)
{
}

const symbol_t AndI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	if (!a->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return symbol_t::Boolean(false);
	if (b->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return symbol_t::Boolean(true);
	return symbol_t::Boolean(false);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class OrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

OrI::OrI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(OR, a, b, token)
{
}

const symbol_t OrI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	if (a->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return symbol_t::Boolean(true);
	if (b->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return symbol_t::Boolean(true);
	return symbol_t::Boolean(false);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BOrI::BOrI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(B_OR, a, b, token)
{
}

const symbol_t BOrI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::bor(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BXOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BXOrI::BXOrI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(B_XOR, a, b, token)
{
}

const symbol_t BXOrI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::bxor(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BAndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BAndI::BAndI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(B_AND, a, b, token)
{
}

const symbol_t BAndI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::band(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftLeftI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftLeftI::BShiftLeftI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(B_SH_L, a, b, token)
{
}

const symbol_t BShiftLeftI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::bshl(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftRightI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftRightI::BShiftRightI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(B_SH_R, a, b, token)
{
}

const symbol_t BShiftRightI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::bshr(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(SET, a, b, token)
{
}

const symbol_t SetI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	if (scope->hasValue(parser_t::HASH_SET) && ((evalA.getValueType() == value_type_enum::OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(parser_t::HASH_SET)) || evalB.getValueType() == value_type_enum::OBJECT))
	{
		ptr_function_t f = nullptr;
		try
		{
			f = scope->getVariable(parser_t::HASH_SET, &token, stack_trace).getFunction({evalA, evalB}, &token, stack_trace);
		}
		catch (const rossa_error_t &e)
		{
		}
		if (f)
		{
			function_evaluate(f, {evalA, evalB}, &token, stack_trace);
			return evalA;
		}
	}
	evalA.set(&evalB, &token, stack_trace);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReturnI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ReturnI::ReturnI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(RETURN, a, token)
{
}

const symbol_t ReturnI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	symbol_t evalA = a->evaluate(scope, stack_trace);
	evalA.setSymbolType(symbol_t::type_t::ID_RETURN);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(const std::string &libname, const std::string &fname, const ptr_instruction_t &a, const token_t &token)
	: UnaryI(EXTERN, a, token), libname{libname}, fname{fname}
{
	this->f = global::loadFunction(libname, fname, &token);
}

const symbol_t ExternI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return f(a->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, parser_t::MAIN_HASH, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(LENGTH, a, token)
{
}

const symbol_t LengthI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType())
	{
	case value_type_enum::STRING:
	{
		std::string str = evalA.getString(&token, stack_trace);
		int c, i, ix, q;
		for (q = 0, i = 0, ix = str.size(); i < ix; i++, q++)
		{
			c = static_cast<unsigned char>(str[i]);
			if (c >= 0 && c <= 127)
				i += 0;
			else if ((c & 0xE0) == 0xC0)
				i += 1;
			else if ((c & 0xF0) == 0xE0)
				i += 2;
			else if ((c & 0xF8) == 0xF0)
				i += 3;
			else
				return symbol_t::Number(number_t::Long(evalA.getString(&token, stack_trace).size()));
		}
		return symbol_t::Number(number_t::Long(q));
	}
	case value_type_enum::DICTIONARY:
		return symbol_t::Number(number_t::Long(evalA.dictionarySize(&token, stack_trace)));
	case value_type_enum::ARRAY:
		return symbol_t::Number(number_t::Long(evalA.vectorSize()));
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(&token, stack_trace);
		if (o->hasValue(parser_t::HASH_LENGTH))
			return o->getVariable(parser_t::HASH_LENGTH, &token, stack_trace).call({}, &token, stack_trace);
	}
	default:
		throw rossa_error_t(_FAILURE_LENGTH_, token, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ClassI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ClassI::ClassI(const hash_ull &key, const scope_type_enum &type, const ptr_instruction_t &body, const ptr_instruction_t &extends, const token_t &token)
	: Instruction(CLASS_I, token), key{key}, type{type}, body{body}, extends{extends}
{
}

const symbol_t ClassI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	ptr_instruction_t nbody = body;
	object_t *ex = NULL;
	std::vector<aug_type_t> extensions;
	if (extends)
	{
		const symbol_t e = extends->evaluate(scope, stack_trace);
		if (e.getValueType() == value_type_enum::TYPE_NAME)
			extensions.push_back(e.getTypeName(&token, stack_trace).getBase());
		else
		{
			ex = e.getObject(&token, stack_trace);
			if (ex->getType() == scope_type_enum::SCOPE_STATIC)
				throw rossa_error_t(_FAILURE_EXTEND_, token, stack_trace);
			const ptr_instruction_t eb = ex->getBody();
			std::vector<ptr_instruction_t> temp;
			temp.push_back(body);
			temp.push_back(eb);
			nbody = std::make_shared<ScopeI>(temp, token);
		}
	}
	object_t o(scope, type, nbody, key, ex, extensions);
	if (type == scope_type_enum::SCOPE_STATIC)
		nbody->evaluate(&o, stack_trace);
	return scope->createVariable(key, symbol_t::Object(o), &token);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NewI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

NewI::NewI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(NEW_I, a, b, token)
{
}

const symbol_t NewI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const auto &base = a->evaluate(scope, stack_trace).getObject(&token, stack_trace);
	return base->instantiate(b->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastToI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

CastToI::CastToI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(CAST_TO_I, a, b, token)
{
}

const symbol_t CastToI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const parameter_t convert = b->evaluate(scope, stack_trace).getTypeName(&token, stack_trace);

	switch (evalA.getValueType())
	{
	case value_type_enum::NUMBER:
		switch (convert.getBase().back())
		{
		case value_type_enum::NUMBER:
			return evalA;
		case value_type_enum::STRING:
			return symbol_t::String(evalA.toString(&token, stack_trace));
		case value_type_enum::BOOLEAN_D:
			return symbol_t::Boolean(evalA.getNumber(&token, stack_trace).getLong() != 0);
		default:
			break;
		}
		break;
	case value_type_enum::STRING:
		switch (convert.getBase().back())
		{
		case value_type_enum::NUMBER:
			try
			{
				const std::string s = evalA.getString(&token, stack_trace);
				if (s.length() > 2 && s[0] == '0' && isalpha(s[1]))
				{
					switch (s[1])
					{
					case 'b':
					case 'B':
						return symbol_t::Number(number_t::Long(std::stoll(s.substr(2), nullptr, 2)));
					default:
						return symbol_t::Number(number_t::Long(std::stoll(s, nullptr, 0)));
					}
				}
				return symbol_t::Number(number_t::Double(std::stold(s)));
			}
			catch (const std::invalid_argument &e)
			{
				throw rossa_error_t(global::format(_FAILURE_STR_TO_NUM_, {evalA.getString(&token, stack_trace)}), token, stack_trace);
			}
		case value_type_enum::STRING:
			return evalA;
		case value_type_enum::BOOLEAN_D:
			return symbol_t::Boolean(evalA.getString(&token, stack_trace) == KEYWORD_TRUE);
		case value_type_enum::ARRAY:
		{
			std::string str = evalA.getString(&token, stack_trace);
			std::vector<symbol_t> nv;
			int last = 0;
			int c, i, ix, q, s;
			for (q = 0, i = 0, ix = str.size(); i < ix; i++, q++)
			{
				c = static_cast<unsigned char>(str[i]);
				if (c >= 0 && c <= 127)
				{
					i += 0;
					s = 1;
				}
				else if ((c & 0xE0) == 0xC0)
				{
					i += 1;
					s = 2;
				}
				else if ((c & 0xF0) == 0xE0)
				{
					i += 2;
					s = 3;
				}
				else if ((c & 0xF8) == 0xF0)
				{
					i += 3;
					s = 4;
				}
				else
				{
					nv.clear();
					for (size_t i = 0; i < str.size(); i++)
						nv.push_back(symbol_t::String(std::string(1, str[i])));
					return symbol_t::Array(nv);
				}
				nv.push_back(symbol_t::String(str.substr(last, s)));
				last = i + 1;
			}
			return symbol_t::Array(nv);
		}
		case value_type_enum::TYPE_NAME:
		{
			const std::string s = evalA.getString(&token, stack_trace);
			if (s == KEYWORD_NUMBER)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::NUMBER}));
			if (s == KEYWORD_STRING)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::STRING}));
			if (s == KEYWORD_BOOLEAN)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::BOOLEAN_D}));
			if (s == KEYWORD_ARRAY)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::ARRAY}));
			if (s == KEYWORD_DICTIONARY)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::DICTIONARY}));
			if (s == KEYWORD_FUNCTION)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::FUNCTION}));
			if (s == KEYWORD_OBJECT)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::OBJECT}));
			if (s == KEYWORD_TYPE)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::TYPE_NAME}));
			if (s == KEYWORD_NIL_NAME)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::NIL}));
			if (s == KEYWORD_POINTER)
				return symbol_t::TypeName(parameter_t({}, {value_type_enum::POINTER}));
			// TODO
			// return symbol_t::TypeName(ROSSA_HASH(evalA.getString(&token, stack_trace)));
		}
		default:
			break;
		}
		break;
	case value_type_enum::BOOLEAN_D:
		switch (convert.getBase().back())
		{
		case value_type_enum::NUMBER:
			return symbol_t::Number(number_t::Long(evalA.getBool(&token, stack_trace) ? 1 : 0));
		case value_type_enum::STRING:
			return symbol_t::String(evalA.getBool(&token, stack_trace) ? "true" : "false");
		case value_type_enum::BOOLEAN_D:
			return evalA;
		default:
			break;
		}
		break;
	case value_type_enum::ARRAY:
		switch (convert.getBase().back())
		{
		case value_type_enum::STRING:
			return symbol_t::String(evalA.toString(&token, stack_trace));
		case value_type_enum::ARRAY:
			return evalA;
		case value_type_enum::DICTIONARY:
		{
			const std::vector<symbol_t> v = evalA.getVector(&token, stack_trace);
			std::map<const std::string, const symbol_t> nd;
			for (size_t i = 0; i < v.size(); i++)
				nd.insert({std::to_string(i), v[i]});
			return symbol_t::Dictionary(nd);
		}
		default:
			break;
		}
		break;
	case value_type_enum::DICTIONARY:
		switch (convert.getBase().back())
		{
		case value_type_enum::STRING:
			return symbol_t::String(evalA.toString(&token, stack_trace));
		case value_type_enum::ARRAY:
		{
			const std::map<const std::string, const symbol_t> dict = evalA.getDictionary(&token, stack_trace);
			std::vector<symbol_t> nv;
			for (const std::pair<const std::string, symbol_t> &e : dict)
			{
				std::vector<symbol_t> l = {symbol_t::String(e.first), e.second};
				nv.push_back(symbol_t::Array(l));
			}
			return symbol_t::Array(nv);
		}
		case value_type_enum::DICTIONARY:
			return evalA;
		default:
			break;
		}
		break;
	case value_type_enum::OBJECT:
	{
		if (convert == evalA.getAugValueType())
			return evalA;
		const hash_ull fname = ROSSA_HASH("->" + convert.toString());
		object_t *o = evalA.getObject(&token, stack_trace);
		if (o->hasValue(fname))
			return o->getVariable(fname, &token, stack_trace).call({}, &token, stack_trace);
		break;
	}
	case value_type_enum::TYPE_NAME:
		switch (convert.getBase().back())
		{
		case value_type_enum::STRING:
			return symbol_t::String("Type::" + evalA.getTypeName(&token, stack_trace).toString());
		case value_type_enum::TYPE_NAME:
			return evalA;
		default:
			break;
		}
		break;
	case value_type_enum::NIL:
		switch (convert.getBase().back())
		{
		case value_type_enum::STRING:
			return symbol_t::String(KEYWORD_NIL);
		default:
			break;
		}
		break;
	default:
		break;
	}

	const hash_ull fname = ROSSA_HASH("->" + convert.toString());
	return scope->getVariable(fname, &token, stack_trace).call({evalA}, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AllocI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

AllocI::AllocI(const ptr_instruction_t &a, const ptr_instruction_t &setall, const token_t &token)
	: UnaryI(ALLOC_I, a, token),
	  setall(setall)
{
}

const symbol_t AllocI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const long_int_t evalA = a->evaluate(scope, stack_trace).getNumber(&token, stack_trace).getLong();
	if (evalA < 0)
		throw rossa_error_t(_FAILURE_ALLOC_, token, stack_trace);
	if (setall)
	{
		auto evalB = setall->evaluate(scope, stack_trace);
		return symbol_t::allocateAs(evalA, &evalB, &token, stack_trace);
	}
	return symbol_t::allocate(evalA);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(const ptr_instruction_t &a, const ptr_instruction_t &b, const ptr_instruction_t &step, const bool &inclusive, const token_t &token)
	: BinaryI(UNTIL_I, a, b, token), step(step), inclusive{inclusive}
{
}

const symbol_t UntilI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	if (step == nullptr)
		return operation::untilnostep(scope, inclusive, evalA, evalB, &token, stack_trace);
	else
		return operation::untilstep(scope, inclusive, evalA, evalB, step->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ScopeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ScopeI::ScopeI(const std::vector<ptr_instruction_t> &children, const token_t &token)
	: Instruction(SCOPE_I, token), children{children}
{
}

const symbol_t ScopeI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	for (const ptr_instruction_t &e : children)
	{
		const symbol_t eval = e->evaluate(scope, stack_trace);
		if (eval.getSymbolType() != symbol_t::type_t::ID_CASUAL)
			return eval;
	}
	return symbol_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MapI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

MapI::MapI(const std::map<std::string, ptr_instruction_t> &children, const token_t &token)
	: Instruction(MAP_I, token), children{children}
{
}

const symbol_t MapI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	std::map<const std::string, const symbol_t> evals;
	for (const std::pair<std::string, ptr_instruction_t> &e : children)
	{
		const symbol_t eval = e.second->evaluate(scope, stack_trace);
		if (eval.getValueType() == value_type_enum::NIL)
			continue;
		evals.insert({e.first, eval});
	}
	return symbol_t::Dictionary(evals);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReferI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ReferI::ReferI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(REFER_I, a, token)
{
}

const symbol_t ReferI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	symbol_t evalA = a->evaluate(scope, stack_trace);
	evalA.setSymbolType(symbol_t::type_t::ID_REFER);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SwitchI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SwitchI::SwitchI(const ptr_instruction_t &switchs, const std::map<symbol_t, size_t> &cases_solved, const std::map<ptr_instruction_t, size_t> &cases_unsolved, const std::vector<ptr_instruction_t> &cases, const ptr_instruction_t &elses, const token_t &token)
	: Instruction(SWITCH_I, token), switchs{switchs}, cases_solved{cases_solved}, cases_unsolved{cases_unsolved}, cases{cases}, elses{elses}
{
}

const symbol_t SwitchI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const object_t newScope(scope, 0);
	const symbol_t eval = switchs->evaluate(&newScope, stack_trace);
	size_t index = 0;
	const auto it = cases_solved.find(eval);
	if (it != cases_solved.end())
	{
		index = it->second;
	}
	else if (!cases_unsolved.empty())
	{
		for (const std::pair<const ptr_instruction_t, const size_t> &e : cases_unsolved)
		{
			const symbol_t evalE = e.first->evaluate(&newScope, stack_trace);
			if (evalE.equals(&eval, &token, stack_trace))
			{
				index = e.second;
				break;
			}
		}
	}

	if (index > 0)
	{
		return cases[index - 1]->evaluate(&newScope, stack_trace);
	}
	else if (elses)
	{
		return elses->evaluate(&newScope, stack_trace);
	}

	return symbol_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TryCatchI                                                                                        */
/*-------------------------------------------------------------------------------------------------------*/

TryCatchI::TryCatchI(const ptr_instruction_t &a, const ptr_instruction_t &b, const hash_ull &key, const token_t &token)
	: BinaryI(TRY_CATCH_I, a, b, token), key{key}
{
}

const symbol_t TryCatchI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	try
	{
		const object_t newScope(scope, 0);
		return a->evaluate(&newScope, stack_trace);
	}
	catch (const rossa_error_t &e)
	{
		const object_t newScope(scope, 0);
		newScope.createVariable(key, symbol_t::String(std::string(e.what())), &token);
		return b->evaluate(&newScope, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ThrowI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ThrowI::ThrowI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(THROW_I, a, token)
{
}

const symbol_t ThrowI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	throw rossa_error_t(evalA.getString(&token, stack_trace), token, stack_trace);
	return symbol_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureEqualsI::PureEqualsI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(PURE_EQUALS, a, b, token)
{
}

const symbol_t PureEqualsI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);
	return symbol_t::Boolean(evalA.pureEquals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureNEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureNEqualsI::PureNEqualsI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(PURE_NEQUALS, a, b, token)
{
}

const symbol_t PureNEqualsI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);
	return symbol_t::Boolean(evalA.pureNEquals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharNI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharNI::CharNI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(CHARN_I, a, token)
{
}

const symbol_t CharNI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const std::string evalA = a->evaluate(scope, stack_trace).getString(&token, stack_trace);
	std::vector<symbol_t> nv;
	for (const unsigned char &c : evalA)
		nv.push_back(symbol_t::Number(number_t::Long(c)));
	return symbol_t::Array(nv);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharSI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharSI::CharSI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(CHARS_I, a, token)
{
}

const symbol_t CharSI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType())
	{
	case value_type_enum::NUMBER:
		return symbol_t::String(std::string(1, static_cast<char>(evalA.getNumber(&token, stack_trace).getLong())));
	case value_type_enum::ARRAY:
	{
		std::string ret = "";
		const std::vector<symbol_t> v = evalA.getVector(&token, stack_trace);
		for (const symbol_t &e : v)
			ret.push_back(static_cast<char>(e.getNumber(&token, stack_trace).getLong()));
		return symbol_t::String(ret);
	}
	default:
		throw rossa_error_t(_FAILURE_TO_STR_, token, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareVarsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareVarsI::DeclareVarsI(const std::vector<hash_ull> &keys, const token_t &token)
	: Instruction(DECLARE_VARS_I, token), keys{keys}
{
}

const symbol_t DeclareVarsI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	std::vector<symbol_t> newvs;
	for (const hash_ull &k : keys)
		newvs.push_back(scope->createVariable(k, &token));
	return symbol_t::Array(newvs);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ParseI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ParseI::ParseI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(ALLOC_I, a, token)
{
}

const symbol_t ParseI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const std::string evalA = a->evaluate(scope, stack_trace).getString(&token, stack_trace);

	const std::vector<token_t> tokens = parser_t::lexString(evalA, std::filesystem::current_path() / KEYWORD_NIL);
	node_parser_t np(tokens, std::filesystem::current_path() / KEYWORD_NIL);

	std::vector<std::pair<std::vector<hash_ull>, symbol_t>> consts;
	std::vector<node_scope_t> scopes;
	return np.parse(&scopes, &consts)->fold(consts)->genParser()->evaluate(scope, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BNotI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

BNotI::BNotI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(B_NOT_I, a, token)
{
}

const symbol_t BNotI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return operation::bnot(
		scope,
		a->evaluate(scope, stack_trace),
		&token,
		stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TypeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

TypeI::TypeI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(TYPE_I, a, token)
{
}

const symbol_t TypeI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return symbol_t::TypeName(a->evaluate(scope, stack_trace).getAugValueType());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallOpI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CallOpI::CallOpI(const size_t &id, const std::vector<ptr_instruction_t> &children, const token_t &token)
	: Instruction(CALL_OP_I, token), id{id}, children{children}
{
}

const symbol_t CallOpI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	switch (id)
	{
	case 0:
		return operation::index(NULL,
								children[0]->evaluate(scope, stack_trace),
								children[1]->evaluate(scope, stack_trace),
								&token, stack_trace);
	case 1:
		return operation::untilnostep(NULL,
									  false,
									  children[0]->evaluate(scope, stack_trace),
									  children[1]->evaluate(scope, stack_trace),
									  &token, stack_trace);
	case 2:
		return operation::untilstep(NULL,
									false,
									children[0]->evaluate(scope, stack_trace),
									children[1]->evaluate(scope, stack_trace),
									children[2]->evaluate(scope, stack_trace),
									&token, stack_trace);
	case 3:
		return operation::untilnostep(NULL,
									  true,
									  children[0]->evaluate(scope, stack_trace),
									  children[1]->evaluate(scope, stack_trace),
									  &token, stack_trace);
	case 4:
		return operation::untilstep(NULL,
									true,
									children[0]->evaluate(scope, stack_trace),
									children[1]->evaluate(scope, stack_trace),
									children[2]->evaluate(scope, stack_trace),
									&token, stack_trace);
	case 5:
		return operation::add(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 6:
		return operation::sub(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 7:
		return operation::mul(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 8:
		return operation::div(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 9:
		return operation::mod(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 10:
		return operation::pow(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 11:
		return operation::less(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   children[1]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 12:
		return operation::more(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   children[1]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 13:
		return operation::eless(NULL,
								children[0]->evaluate(scope, stack_trace),
								children[1]->evaluate(scope, stack_trace),
								&token, stack_trace);
	case 14:
		return operation::emore(NULL,
								children[0]->evaluate(scope, stack_trace),
								children[1]->evaluate(scope, stack_trace),
								&token, stack_trace);
	case 15:
		return operation::bor(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 16:
		return operation::bxor(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   children[1]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 17:
		return operation::band(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   children[1]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 18:
		return operation::bshl(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   children[1]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 19:
		return operation::bshr(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   children[1]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 20:
		return operation::call(scope,
							   children[0],
							   children[1]->evaluate(scope, stack_trace).getVector(&token, stack_trace),
							   &token, stack_trace);
	case 21:
		return operation::bnot(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 22:
		return operation::unadd(NULL,
								children[0]->evaluate(scope, stack_trace),
								&token, stack_trace);
	case 23:
		return operation::neg(NULL,
							  children[0]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 24:
		return operation::unot(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 25:
		return operation::cct(scope,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 26:
		return operation::del(scope,
							  children[0]->evaluate(scope, stack_trace),
							  children[1]->evaluate(scope, stack_trace),
							  &token, stack_trace);
	case 27:
		return operation::hash(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	case 28:
		return operation::fdiv(NULL,
							   children[0]->evaluate(scope, stack_trace),
							   children[1]->evaluate(scope, stack_trace),
							   &token, stack_trace);
	default:
		return symbol_t();
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeleteI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

DeleteI::DeleteI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(DELETE_I, a, b, token)
{
}

const symbol_t DeleteI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);
	return operation::del(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnAddI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UnAddI::UnAddI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(UN_ADD_I, a, token)
{
}

const symbol_t UnAddI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return operation::unadd(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NegI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

NegI::NegI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(NEG_I, a, token)
{
}

const symbol_t NegI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return operation::neg(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NotI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

NotI::NotI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(NOT_I, a, token)
{
}

const symbol_t NotI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return operation::unot(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ConcatI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ConcatI::ConcatI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(CONCAT_I, a, b, token)
{
}

const symbol_t ConcatI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::cct(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetIndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetIndexI::SetIndexI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(SET_INDEX_I, a, b, token)
{
}

const symbol_t SetIndexI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalAVector = evalA.getVector(&token, stack_trace);
	auto evalBVector = b->evaluate(scope, stack_trace).getVector(&token, stack_trace);
	if (evalAVector.size() != evalBVector.size())
	{
		throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, token, stack_trace);
	}
	for (size_t i = 0; i < evalAVector.size(); i++)
	{
		evalAVector[i].set(&evalBVector[i], &token, stack_trace);
	}
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class HashI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

HashI::HashI(const ptr_instruction_t &a, const token_t &token)
	: UnaryI(HASH_I, a, token)
{
}

const symbol_t HashI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	return operation::hash(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EachI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EachI::EachI(const hash_ull &id, const ptr_instruction_t &eachs, const ptr_instruction_t &wheres, const ptr_instruction_t &body, const token_t &token)
	: Instruction(EACH_I, token), id{id}, eachs{eachs}, wheres{wheres}, body{body}
{
}

const symbol_t EachI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const std::vector<symbol_t> evalFor = eachs->evaluate(scope, stack_trace).getVector(&token, stack_trace);
	std::vector<symbol_t> list;
	for (const symbol_t &e : evalFor)
	{
		const object_t newScope(scope, 0);
		newScope.createVariable(id, e, &token);
		symbol_t r = e;
		if (wheres)
		{
			auto check = wheres->evaluate(&newScope, stack_trace);
			if (!check.getBool(&token, stack_trace))
				continue;
		}
		if (body)
		{
			r = body->evaluate(&newScope, stack_trace);
			if (r.getSymbolType() == symbol_t::type_t::ID_REFER)
			{
				r.setSymbolType(symbol_t::type_t::ID_CASUAL);
			}
		}
		list.push_back(r);
	}
	return symbol_t::Array(list);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class FDivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

FDivI::FDivI(const ptr_instruction_t &a, const ptr_instruction_t &b, const token_t &token)
	: BinaryI(FDIV_I, a, b, token)
{
}

const symbol_t FDivI::evaluate(const object_t *scope, trace_t &stack_trace) const
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);
	const symbol_t evalB = b->evaluate(scope, stack_trace);

	return operation::fdiv(scope, evalA, evalB, &token, stack_trace);
}