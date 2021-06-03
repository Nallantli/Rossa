#include "Rossa.h"
#include "Operator.hpp"

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(const type_t &type, const token_t &token)
	: token{ token }
	, type{ type }
{}

const Instruction::type_t Instruction::getType() const
{
	return type;
}

Instruction::~Instruction()
{}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(const type_t &type, const i_ptr_t &a, const token_t &token)
	: Instruction(type, token)
	, a{ a }
{}

const i_ptr_t UnaryI::getA() const
{
	return a;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastingI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

CastingI::CastingI(const type_t &type, const hash_ull &key, const token_t &token)
	: Instruction(type, token)
	, key{ key }
{}

const hash_ull CastingI::getKey() const
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(const type_t &type, const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: UnaryI(type, a, token)
	, b{ b }
{}

const i_ptr_t BinaryI::getB() const
{
	return b;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ContainerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ContainerI::ContainerI(const sym_t &d, const token_t &token)
	: Instruction(CONTAINER, token)
	, d{ d }
{}

const sym_t ContainerI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return d;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(const hash_ull &key, const fsig_t &ftype, const std::vector<std::pair<LexerTokenType, hash_ull>> &params, const i_ptr_t &body, const std::vector<hash_ull> &captures, const token_t &token)
	: Instruction(DEFINE, token)
	, key{ key }
	, ftype{ ftype }
	, params{ params }
	, body{ body }
	, captures{ captures }
{}

const sym_t DefineI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	hash_sym_map_t capturedVars;
	for (const hash_ull &e : captures) {
		capturedVars[e].set(&scope->getVariable(e, &token, stack_trace), &token, stack_trace);
	}
	func_ptr_t f = std::make_shared<Function>(key, scope->getPtr(), params, body, capturedVars);
	if (key > 0) {
		return scope->createVariable(key, sym_t::FunctionSIG(ftype, f), &token);
	}
	return sym_t::FunctionSIG(ftype, f);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VargDefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VargDefineI::VargDefineI(const hash_ull &key, const i_ptr_t &body, const std::vector<hash_ull> &captures, const token_t &token)
	: Instruction(VARG_DEFINE, token)
	, key{ key }
	, body{ body }
	, captures{ captures }
{}

const sym_t VargDefineI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	hash_sym_map_t capturedVars;
	for (const hash_ull &e : captures) {
		capturedVars[e].set(&scope->getVariable(e, &token, stack_trace), &token, stack_trace);
	}
	func_ptr_t f = std::make_shared<Function>(key, scope->getPtr(), body, capturedVars);
	if (key > 0) {
		return scope->createVariable(key, sym_t::FunctionVARG(static_cast<func_ptr_t>(f)), &token);
	}
	return sym_t::FunctionVARG(static_cast<func_ptr_t>(f));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SequenceI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SequenceI::SequenceI(const i_vec_t &children, const token_t &token)
	: Instruction(SEQUENCE, token)
	, children{ children }
{}

const sym_t SequenceI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	sym_vec_t evals;
	for (const i_ptr_t &e : children) {
		if (e->getType() == UNTIL_I) {
			sym_vec_t v = e->evaluate(scope, stack_trace).getVector(&token, stack_trace);
			evals.insert(evals.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
		} else {
			evals.push_back(e->evaluate(scope, stack_trace));
		}
	}
	return sym_t::Array(evals);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IFElseI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

IfElseI::IfElseI(const i_ptr_t &ifs, const i_ptr_t &body, const i_ptr_t &elses, const token_t &token)
	: Instruction(IFELSE, token)
	, ifs{ ifs }
	, body{ body }
	, elses{ elses }
{}

const sym_t IfElseI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const scope_t newScope(scope, 0);
	if (ifs->evaluate(&newScope, stack_trace).getBool(&token, stack_trace)) {
		return body->evaluate(&newScope, stack_trace);
	} else if (elses) {
		return elses->evaluate(&newScope, stack_trace);
	}
	return sym_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class WhileI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

WhileI::WhileI(const i_ptr_t &whiles, const i_vec_t &body, const token_t &token)
	: Instruction(WHILE, token)
	, whiles{ whiles }
	, body{ body }
{}

const sym_t WhileI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	while (whiles->evaluate(scope, stack_trace).getBool(&token, stack_trace)) {
		const scope_t newScope(scope, 0);
		for (const i_ptr_t &i : body) {
			const sym_t temp = i->evaluate(&newScope, stack_trace);
			bool cflag = false;
			switch (temp.getSymbolType()) {
				case sym_t::type_t::ID_REFER:
				case sym_t::type_t::ID_RETURN:
					return temp;
				case sym_t::type_t::ID_BREAK:
					return sym_t();
				case sym_t::type_t::ID_CONTINUE:
					cflag = true;
					break;
				default:
					break;
			}
			if (cflag)
				break;
		}
	}
	return sym_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ForI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ForI::ForI(const hash_ull &id, const i_ptr_t &fors, const i_vec_t &body, const token_t &token)
	: Instruction(FOR, token)
	, id{ id }
	, fors{ fors }
	, body{ body }
{}

const sym_t ForI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_vec_t evalFor = fors->evaluate(scope, stack_trace).getVector(&token, stack_trace);
	for (const sym_t &e : evalFor) {
		const scope_t newScope(scope, 0);
		newScope.createVariable(id, e, &token);
		bool cflag = false;
		for (const i_ptr_t &i : body) {
			const sym_t temp = i->evaluate(&newScope, stack_trace);
			switch (temp.getSymbolType()) {
				case sym_t::type_t::ID_REFER:
				case sym_t::type_t::ID_RETURN:
					return temp;
				case sym_t::type_t::ID_BREAK:
					return sym_t();
				case sym_t::type_t::ID_CONTINUE:
					cflag = true;
					break;
				default:
					break;
			}
			if (cflag)
				break;
		}
	}
	return sym_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(const hash_ull &key, const token_t &token)
	: CastingI(VARIABLE, key, token)
{}

const sym_t VariableI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return scope->getVariable(key, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class GetThisI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

GetThisI::GetThisI(const token_t &token)
	: CastingI(GET_THIS_I, key, token)
{}

const sym_t GetThisI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return scope->getThis(&token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareI::DeclareI(const hash_ull &key, const type_sll &vtype, const i_ptr_t &a, const bool &isConst, const token_t &token)
	: CastingI(DECLARE, key, token)
	, vtype{ vtype }
	, a{ a }
	, isConst{ isConst }
{}

const sym_t DeclareI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t v = scope->createVariable(key, &token);
	const sym_t evalA = a->evaluate(scope, stack_trace);
	v.set(&evalA, &token, stack_trace);
	return v;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(INDEX, a, b, token)
{}

const sym_t IndexI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::index(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(INNER, a, b, token)
{}

const sym_t InnerI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType()) {
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(&token, stack_trace);
			if (o->getType() != scope_t::scope_type_t::STATIC_O && o->getType() != scope_t::scope_type_t::INSTANCE_O)
				throw rossa_error(_CANNOT_INDEX_OBJECT_, token, stack_trace);
			return b->evaluate(o, stack_trace);
		}
		default:
			throw rossa_error(_CANNOT_INDEX_VALUE_, token, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

CallI::CallI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(INDEX, a, b, token)
{}

const sym_t CallI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return ops::call(scope, a, b->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AddI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AddI::AddI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(ADD, a, b, token)
{}

const sym_t AddI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::add(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SubI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SubI::SubI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(SUB, a, b, token)
{}

const sym_t SubI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::sub(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MulI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MulI::MulI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(MUL, a, b, token)
{}

const sym_t MulI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::mul(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DivI::DivI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(DIV, a, b, token)
{}

const sym_t DivI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::div(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ModI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ModI::ModI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(MOD, a, b, token)
{}

const sym_t ModI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::mod(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(POW_I, a, b, token)
{}

const sym_t PowI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::pow(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(LESS, a, b, token)
{}

const sym_t LessI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::less(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(MORE, a, b, token)
{}

const sym_t MoreI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::more(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(ELESS, a, b, token)
{}

const sym_t ELessI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::eless(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(EMORE, a, b, token)
{}

const sym_t EMoreI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::emore(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EqualsI::EqualsI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(EQUALS, a, b, token)
{}

const sym_t EqualsI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == Value::type_t::OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Rossa::HASH_EQUALS))
		return scope->getVariable(Rossa::HASH_EQUALS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);

	return sym_t::Boolean(evalA.equals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

NEqualsI::NEqualsI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(NEQUALS, a, b, token)
{}

const sym_t NEqualsI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == Value::type_t::OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Rossa::HASH_NEQUALS))
		return scope->getVariable(Rossa::HASH_NEQUALS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);

	return sym_t::Boolean(evalA.nequals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(AND, a, b, token)
{}

const sym_t AndI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	if (!a->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return sym_t::Boolean(false);
	if (b->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return sym_t::Boolean(true);
	return sym_t::Boolean(false);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class OrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

OrI::OrI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(OR, a, b, token)
{}

const sym_t OrI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	if (a->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return sym_t::Boolean(true);
	if (b->evaluate(scope, stack_trace).getBool(&token, stack_trace))
		return sym_t::Boolean(true);
	return sym_t::Boolean(false);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BOrI::BOrI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(B_OR, a, b, token)
{}

const sym_t BOrI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::bor(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BXOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BXOrI::BXOrI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(B_XOR, a, b, token)
{}

const sym_t BXOrI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::bxor(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BAndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BAndI::BAndI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(B_AND, a, b, token)
{}

const sym_t BAndI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::band(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftLeftI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftLeftI::BShiftLeftI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(B_SH_L, a, b, token)
{}

const sym_t BShiftLeftI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::bshl(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftRightI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftRightI::BShiftRightI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(B_SH_R, a, b, token)
{}

const sym_t BShiftRightI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::bshr(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(SET, a, b, token)
{}

const sym_t SetI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == Value::type_t::OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Rossa::HASH_SET)) {
		try {
			return scope->getVariable(Rossa::HASH_SET, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
		} catch (const rossa_error &e) {
			evalA.set(&evalB, &token, stack_trace);
		}
	} else {
		evalA.set(&evalB, &token, stack_trace);
	}
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReturnI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ReturnI::ReturnI(const i_ptr_t &a, const token_t &token)
	: UnaryI(RETURN, a, token)
{}

const sym_t ReturnI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	sym_t evalA = a->evaluate(scope, stack_trace);
	evalA.setSymbolType(sym_t::type_t::ID_RETURN);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(const std::string &libname, const std::string &fname, const i_ptr_t &a, const token_t &token)
	: UnaryI(EXTERN, a, token)
	, libname{ libname }
	, fname{ fname }
{
	this->f = lib::loadFunction(libname, fname, &token);
}

const sym_t ExternI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return f(a->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, Rossa::MAIN_HASH, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(const i_ptr_t &a, const token_t &token)
	: UnaryI(LENGTH, a, token)
{}

const sym_t LengthI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType()) {
		case Value::type_t::STRING:
		{
			std::string str = evalA.getString(&token, stack_trace);
			int c, i, ix, q;
			for (q = 0, i = 0, ix = str.size(); i < ix; i++, q++) {
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
					return sym_t::Number(number_t::Long(evalA.getString(&token, stack_trace).size()));
			}
			return sym_t::Number(number_t::Long(q));
		}
		case Value::type_t::DICTIONARY:
			return sym_t::Number(number_t::Long(evalA.dictionarySize(&token, stack_trace)));
		case Value::type_t::ARRAY:
			return sym_t::Number(number_t::Long(evalA.vectorSize()));
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Rossa::HASH_LENGTH))
				return o->getVariable(Rossa::HASH_LENGTH, &token, stack_trace).call({ }, &token, stack_trace);
		}
		default:
			throw rossa_error(_FAILURE_LENGTH_, token, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ClassI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ClassI::ClassI(const hash_ull &key, const scope_t::scope_type_t &type, const i_ptr_t &body, const i_ptr_t &extends, const token_t &token)
	: Instruction(CLASS_I, token)
	, key{ key }
	, type{ type }
	, body{ body }
	, extends{ extends }
{}

const sym_t ClassI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	i_ptr_t nbody = body;
	scope_t *ex = NULL;
	std::vector<aug_type_t> extensions;
	if (extends) {
		const sym_t e = extends->evaluate(scope, stack_trace);
		if (e.getValueType() == Value::type_t::TYPE_NAME)
			extensions.push_back(e.getTypeName(&token, stack_trace).getBase());
		else {
			ex = e.getObject(&token, stack_trace);
			if (ex->getType() == scope_t::scope_type_t::STATIC_O)
				throw rossa_error(_FAILURE_EXTEND_, token, stack_trace);
			const i_ptr_t eb = ex->getBody();
			i_vec_t temp;
			temp.push_back(body);
			temp.push_back(eb);
			nbody = std::make_shared<ScopeI>(temp, token);
		}
	}
	scope_t o(scope, type, nbody, key, ex, extensions);
	if (type == scope_t::scope_type_t::STATIC_O)
		nbody->evaluate(&o, stack_trace);
	return scope->createVariable(key, sym_t::Object(o), &token);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NewI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

NewI::NewI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(NEW_I, a, b, token)
{}

const sym_t NewI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const auto &base = a->evaluate(scope, stack_trace).getObject(&token, stack_trace);
	return base->instantiate(b->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastToI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

CastToI::CastToI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(CAST_TO_I, a, b, token)
{}

const sym_t CastToI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const param_t convert = b->evaluate(scope, stack_trace).getTypeName(&token, stack_trace);

	switch (evalA.getValueType()) {
		case Value::type_t::NUMBER:
			switch (convert.getBase().back()) {
				case Value::type_t::NUMBER:
					return evalA;
				case Value::type_t::STRING:
					return sym_t::String(evalA.toString(&token, stack_trace));
				case Value::type_t::BOOLEAN_D:
					return sym_t::Boolean(evalA.getNumber(&token, stack_trace).getLong() != 0);
				default:
					break;
			}
			break;
		case Value::type_t::STRING:
			switch (convert.getBase().back()) {
				case Value::type_t::NUMBER:
					try {
						const std::string s = evalA.getString(&token, stack_trace);
						if (s.length() > 2 && s[0] == '0' && isalpha(s[1])) {
							switch (s[1]) {
								case 'b':
								case 'B':
									return sym_t::Number(number_t::Long(std::stoll(s.substr(2), nullptr, 2)));
								default:
									return sym_t::Number(number_t::Long(std::stoll(s, nullptr, 0)));
							}
						}
						return sym_t::Number(number_t::Double(std::stold(s)));
					} catch (const std::invalid_argument &e) {
						throw rossa_error(format::format(_FAILURE_STR_TO_NUM_, { evalA.getString(&token, stack_trace) }), token, stack_trace);
					}
				case Value::type_t::STRING:
					return evalA;
				case Value::type_t::BOOLEAN_D:
					return sym_t::Boolean(evalA.getString(&token, stack_trace) == KEYWORD_TRUE);
				case Value::type_t::ARRAY:
				{
					std::string str = evalA.getString(&token, stack_trace);
					sym_vec_t nv;
					int last = 0;
					int c, i, ix, q, s;
					for (q = 0, i = 0, ix = str.size(); i < ix; i++, q++) {
						c = static_cast<unsigned char>(str[i]);
						if (c >= 0 && c <= 127) {
							i += 0;
							s = 1;
						} else if ((c & 0xE0) == 0xC0) {
							i += 1;
							s = 2;
						} else if ((c & 0xF0) == 0xE0) {
							i += 2;
							s = 3;
						} else if ((c & 0xF8) == 0xF0) {
							i += 3;
							s = 4;
						} else {
							nv.clear();
							for (size_t i = 0; i < str.size(); i++)
								nv.push_back(sym_t::String(std::string(1, str[i])));
							return sym_t::Array(nv);
						}
						nv.push_back(sym_t::String(str.substr(last, s)));
						last = i + 1;
					}
					return sym_t::Array(nv);
				}
				case Value::type_t::TYPE_NAME:
				{
					const std::string s = evalA.getString(&token, stack_trace);
					if (s == KEYWORD_NUMBER)
						return sym_t::TypeName(param_t({}, { Value::type_t::NUMBER }));
					if (s == KEYWORD_STRING)
						return sym_t::TypeName(param_t({}, { Value::type_t::STRING }));
					if (s == KEYWORD_BOOLEAN)
						return sym_t::TypeName(param_t({}, { Value::type_t::BOOLEAN_D }));
					if (s == KEYWORD_ARRAY)
						return sym_t::TypeName(param_t({}, { Value::type_t::ARRAY }));
					if (s == KEYWORD_DICTIONARY)
						return sym_t::TypeName(param_t({}, { Value::type_t::DICTIONARY }));
					if (s == KEYWORD_FUNCTION)
						return sym_t::TypeName(param_t({}, { Value::type_t::FUNCTION }));
					if (s == KEYWORD_OBJECT)
						return sym_t::TypeName(param_t({}, { Value::type_t::OBJECT }));
					if (s == KEYWORD_TYPE)
						return sym_t::TypeName(param_t({}, { Value::type_t::TYPE_NAME }));
					if (s == KEYWORD_NIL_NAME)
						return sym_t::TypeName(param_t({}, { Value::type_t::NIL }));
					if (s == KEYWORD_POINTER)
						return sym_t::TypeName(param_t({}, { Value::type_t::POINTER }));
					//TODO
					//return sym_t::TypeName(ROSSA_HASH(evalA.getString(&token, stack_trace)));
				}
				default:
					break;
			}
			break;
		case Value::type_t::BOOLEAN_D:
			switch (convert.getBase().back()) {
				case Value::type_t::NUMBER:
					return sym_t::Number(number_t::Long(evalA.getBool(&token, stack_trace) ? 1 : 0));
				case Value::type_t::STRING:
					return sym_t::String(evalA.getBool(&token, stack_trace) ? "true" : "false");
				case Value::type_t::BOOLEAN_D:
					return evalA;
				default:
					break;
			}
			break;
		case Value::type_t::ARRAY:
			switch (convert.getBase().back()) {
				case Value::type_t::STRING:
					return sym_t::String(evalA.toString(&token, stack_trace));
				case Value::type_t::ARRAY:
					return evalA;
				case Value::type_t::DICTIONARY:
				{
					const sym_vec_t v = evalA.getVector(&token, stack_trace);
					sym_map_t nd;
					for (size_t i = 0; i < v.size(); i++)
						nd.insert({ std::to_string(i), v[i] });
					return sym_t::Dictionary(nd);
				}
				default:
					break;
			}
			break;
		case Value::type_t::DICTIONARY:
			switch (convert.getBase().back()) {
				case Value::type_t::STRING:
					return sym_t::String(evalA.toString(&token, stack_trace));
				case Value::type_t::ARRAY:
				{
					const sym_map_t dict = evalA.getDictionary(&token, stack_trace);
					sym_vec_t nv;
					for (const std::pair<const std::string, sym_t> &e : dict) {
						sym_vec_t l = { sym_t::String(e.first), e.second };
						nv.push_back(sym_t::Array(l));
					}
					return sym_t::Array(nv);
				}
				case Value::type_t::DICTIONARY:
					return evalA;
				default:
					break;
			}
			break;
		case Value::type_t::OBJECT:
		{
			if (convert == evalA.getAugValueType())
				return evalA;
			const hash_ull fname = ROSSA_HASH("->" + convert.toString());
			scope_t *o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(fname))
				return o->getVariable(fname, &token, stack_trace).call({ }, &token, stack_trace);
			break;
		}
		case Value::type_t::TYPE_NAME:
			switch (convert.getBase().back()) {
				case Value::type_t::STRING:
					return sym_t::String("Type::" + evalA.getTypeName(&token, stack_trace).toString());
				case Value::type_t::TYPE_NAME:
					return evalA;
				default:
					break;
			}
			break;
		case Value::type_t::NIL:
			switch (convert.getBase().back()) {
				case Value::type_t::STRING:
					return sym_t::String(KEYWORD_NIL);
				default:
					break;
			}
			break;
		default:
			break;
	}

	const hash_ull fname = ROSSA_HASH("->" + convert.toString());
	return scope->getVariable(fname, &token, stack_trace).call({ evalA }, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AllocI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

AllocI::AllocI(const i_ptr_t &a, const token_t &token)
	: UnaryI(ALLOC_I, a, token)
{}

const sym_t AllocI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const long_int_t evalA = a->evaluate(scope, stack_trace).getNumber(&token, stack_trace).getLong();
	if (evalA < 0)
		throw rossa_error(_FAILURE_ALLOC_, token, stack_trace);
	return sym_t::allocate(evalA);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(const i_ptr_t &a, const i_ptr_t &b, const i_ptr_t &step, const bool &inclusive, const token_t &token)
	: BinaryI(UNTIL_I, a, b, token)
	, step(step)
	, inclusive{ inclusive }
{}

const sym_t UntilI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	if (step == nullptr)
		return ops::untilnostep(scope, inclusive, evalA, evalB, &token, stack_trace);
	else
		return ops::untilstep(scope, inclusive, evalA, evalB, step->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ScopeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ScopeI::ScopeI(const i_vec_t &children, const token_t &token)
	: Instruction(SCOPE_I, token)
	, children{ children }
{}

const sym_t ScopeI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	for (const i_ptr_t &e : children) {
		const sym_t eval = e->evaluate(scope, stack_trace);
		if (eval.getSymbolType() != sym_t::type_t::ID_CASUAL)
			return eval;
	}
	return sym_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MapI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

MapI::MapI(const std::map<std::string, i_ptr_t> &children, const token_t &token)
	: Instruction(MAP_I, token)
	, children{ children }
{}

const sym_t MapI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	sym_map_t evals;
	for (const std::pair<std::string, i_ptr_t> &e : children) {
		const sym_t eval = e.second->evaluate(scope, stack_trace);
		if (eval.getValueType() == Value::type_t::NIL)
			continue;
		evals.insert({ e.first, eval });
	}
	return sym_t::Dictionary(evals);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReferI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ReferI::ReferI(const i_ptr_t &a, const token_t &token)
	: UnaryI(REFER_I, a, token)
{}

const sym_t ReferI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	sym_t evalA = a->evaluate(scope, stack_trace);
	evalA.setSymbolType(sym_t::type_t::ID_REFER);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SwitchI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SwitchI::SwitchI(const i_ptr_t &switchs, const std::map<sym_t, size_t> &cases_solved, const std::map<i_ptr_t, size_t> &cases_unsolved, const i_vec_t &cases, const i_ptr_t &elses, const token_t &token)
	: Instruction(SWITCH_I, token)
	, switchs{ switchs }
	, cases_solved{ cases_solved }
	, cases_unsolved{ cases_unsolved }
	, cases{ cases }
	, elses{ elses }
{}

const sym_t SwitchI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const scope_t newScope(scope, 0);
	const sym_t eval = switchs->evaluate(&newScope, stack_trace);
	size_t index = 0;
	const auto it = cases_solved.find(eval);
	if (it != cases_solved.end()) {
		index = it->second;
	} else if (!cases_unsolved.empty()) {
		for (const std::pair<const i_ptr_t, const size_t> &e : cases_unsolved) {
			const sym_t evalE = e.first->evaluate(&newScope, stack_trace);
			if (evalE.equals(&eval, &token, stack_trace)) {
				index = e.second;
				break;
			}
		}
	}

	if (index > 0) {
		return cases[index - 1]->evaluate(&newScope, stack_trace);
	} else if (elses) {
		return elses->evaluate(&newScope, stack_trace);
	}

	return sym_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TryCatchI                                                                                        */
/*-------------------------------------------------------------------------------------------------------*/

TryCatchI::TryCatchI(const i_ptr_t &a, const i_ptr_t &b, const hash_ull &key, const token_t &token)
	: BinaryI(TRY_CATCH_I, a, b, token)
	, key{ key }
{}

const sym_t TryCatchI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	try {
		const scope_t newScope(scope, 0);
		return a->evaluate(&newScope, stack_trace);
	} catch (const rossa_error &e) {
		const scope_t newScope(scope, 0);
		newScope.createVariable(key, sym_t::String(std::string(e.what())), &token);
		return b->evaluate(&newScope, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ThrowI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ThrowI::ThrowI(const i_ptr_t &a, const token_t &token)
	: UnaryI(THROW_I, a, token)
{}

const sym_t ThrowI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	throw rossa_error(evalA.getString(&token, stack_trace), token, stack_trace);
	return sym_t();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureEqualsI::PureEqualsI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(PURE_EQUALS, a, b, token)
{}

const sym_t PureEqualsI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);
	return sym_t::Boolean(evalA.pureEquals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureNEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureNEqualsI::PureNEqualsI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(PURE_NEQUALS, a, b, token)
{}

const sym_t PureNEqualsI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);
	return sym_t::Boolean(evalA.pureNEquals(&evalB, &token, stack_trace));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharNI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharNI::CharNI(const i_ptr_t &a, const token_t &token)
	: UnaryI(CHARN_I, a, token)
{}

const sym_t CharNI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const std::string evalA = a->evaluate(scope, stack_trace).getString(&token, stack_trace);
	sym_vec_t nv;
	for (const unsigned char &c : evalA)
		nv.push_back(sym_t::Number(number_t::Long(c)));
	return sym_t::Array(nv);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharSI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharSI::CharSI(const i_ptr_t &a, const token_t &token)
	: UnaryI(CHARS_I, a, token)
{}

const sym_t CharSI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType()) {
		case Value::type_t::NUMBER:
			return sym_t::String(std::string(1, static_cast<char>(evalA.getNumber(&token, stack_trace).getLong())));
		case Value::type_t::ARRAY:
		{
			std::string ret = "";
			const sym_vec_t v = evalA.getVector(&token, stack_trace);
			for (const sym_t &e : v)
				ret.push_back(static_cast<char>(e.getNumber(&token, stack_trace).getLong()));
			return sym_t::String(ret);
		}
		default:
			throw rossa_error(_FAILURE_TO_STR_, token, stack_trace);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareVarsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareVarsI::DeclareVarsI(const std::vector<hash_ull> &keys, const token_t &token)
	: Instruction(DECLARE_VARS_I, token)
	, keys{ keys }
{}

const sym_t DeclareVarsI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	sym_vec_t newvs;
	for (const hash_ull &k : keys)
		newvs.push_back(scope->createVariable(k, &token));
	return sym_t::Array(newvs);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ParseI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ParseI::ParseI(const i_ptr_t &a, const token_t &token)
	: UnaryI(ALLOC_I, a, token)
{}

const sym_t ParseI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const std::string evalA = a->evaluate(scope, stack_trace).getString(&token, stack_trace);

	const std::vector<token_t> tokens = Rossa::lexString(evalA, std::filesystem::current_path() / KEYWORD_NIL);
	NodeParser np(tokens, std::filesystem::current_path() / KEYWORD_NIL);

	std::vector<std::pair<std::vector<hash_ull>, sym_t>> consts;
	ns_vec_t scopes;
	return np.parse(&scopes, &consts)->fold(consts)->genParser()->evaluate(scope, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BNotI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

BNotI::BNotI(const i_ptr_t &a, const token_t &token)
	: UnaryI(B_NOT_I, a, token)
{}

const sym_t BNotI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return ops::bnot(
		scope,
		a->evaluate(scope, stack_trace),
		&token,
		stack_trace
	);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TypeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

TypeI::TypeI(const i_ptr_t &a, const token_t &token)
	: UnaryI(TYPE_I, a, token)
{}

const sym_t TypeI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return sym_t::TypeName(a->evaluate(scope, stack_trace).getAugValueType());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallOpI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CallOpI::CallOpI(const size_t &id, const i_vec_t &children, const token_t &token)
	: Instruction(CALL_OP_I, token)
	, id{ id }
	, children{ children }
{}

const sym_t CallOpI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	switch (id) {
		case 0:
			return ops::index(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 1:
			return ops::untilnostep(NULL,
				false,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 2:
			return ops::untilstep(NULL,
				false,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				children[2]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 3:
			return ops::untilnostep(NULL,
				true,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 4:
			return ops::untilstep(NULL,
				true,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				children[2]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 5:
			return ops::add(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 6:
			return ops::sub(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 7:
			return ops::mul(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 8:
			return ops::div(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 9:
			return ops::mod(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 10:
			return ops::pow(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 11:
			return ops::less(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 12:
			return ops::more(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 13:
			return ops::eless(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 14:
			return ops::emore(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 15:
			return ops::bor(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 16:
			return ops::bxor(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 17:
			return ops::band(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 18:
			return ops::bshl(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 19:
			return ops::bshr(NULL,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 20:
			return ops::call(scope,
				children[0],
				children[1]->evaluate(scope, stack_trace).getVector(&token, stack_trace),
				&token, stack_trace);
		case 21:
			return ops::bnot(NULL,
				children[0]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 22:
			return ops::unadd(NULL,
				children[0]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 23:
			return ops::neg(NULL,
				children[0]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 24:
			return ops::unot(NULL,
				children[0]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 25:
			return ops::cct(scope,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 26:
			return ops::del(scope,
				children[0]->evaluate(scope, stack_trace),
				children[1]->evaluate(scope, stack_trace),
				&token, stack_trace);
		case 27:
			return ops::hash(NULL,
				children[0]->evaluate(scope, stack_trace),
				&token, stack_trace);
		default:
			return sym_t();
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeleteI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

DeleteI::DeleteI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(DELETE_I, a, b, token)
{}

const sym_t DeleteI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);
	return ops::del(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnAddI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UnAddI::UnAddI(const i_ptr_t &a, const token_t &token)
	: UnaryI(UN_ADD_I, a, token)
{}

const sym_t UnAddI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return ops::unadd(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NegI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

NegI::NegI(const i_ptr_t &a, const token_t &token)
	: UnaryI(NEG_I, a, token)
{}

const sym_t NegI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return ops::neg(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NotI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

NotI::NotI(const i_ptr_t &a, const token_t &token)
	: UnaryI(NOT_I, a, token)
{}

const sym_t NotI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return ops::unot(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ConcatI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ConcatI::ConcatI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(CONCAT_I, a, b, token)
{}

const sym_t ConcatI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);

	return ops::cct(scope, evalA, evalB, &token, stack_trace);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetIndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetIndexI::SetIndexI(const i_ptr_t &a, const i_ptr_t &b, const token_t &token)
	: BinaryI(SET_INDEX_I, a, b, token)
{}

const sym_t SetIndexI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	const sym_t evalA = a->evaluate(scope, stack_trace);
	const sym_t evalB = b->evaluate(scope, stack_trace);
	for (auto &a : evalA.getVector(&token, stack_trace)) {
		a.set(&evalB, &token, stack_trace);
	}
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class HashI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

HashI::HashI(const i_ptr_t &a, const token_t &token)
	: UnaryI(HASH_I, a, token)
{}

const sym_t HashI::evaluate(const scope_t *scope, trace_t &stack_trace) const
{
	return ops::hash(scope, a->evaluate(scope, stack_trace), &token, stack_trace);
}