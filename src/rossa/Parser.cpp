#include "../../bin/include/Rossa.h"

using namespace rossa;

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(const InstructionType &type, const Token &token) : type(type), token(token)
{}

InstructionType Instruction::getType() const
{
	return type;
}

Instruction::~Instruction()
{}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(const InstructionType &type, const std::shared_ptr<Instruction> &a, const Token &token) : Instruction(type, token), a(a)
{}

const std::shared_ptr<Instruction> UnaryI::getA() const
{
	return a;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastingI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

CastingI::CastingI(const InstructionType &type, const hash_ull &key, const Token &token) : Instruction(type, token), key(key)
{}

const hash_ull CastingI::getKey() const
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(const InstructionType &type, const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : UnaryI(type, a, token), b(b)
{}

const std::shared_ptr<Instruction> BinaryI::getB() const
{
	return b;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ContainerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ContainerI::ContainerI(const Symbol &d, const Token &token) : Instruction(CONTAINER, token), d(d)
{}

const Symbol ContainerI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	return d;
}

const std::string ContainerI::compile() const
{
	return C_UNARY("ContainerI", d.toCodeString());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(const hash_ull &key, const sig_t &ftype, const std::vector<std::pair<LexerTokenType, hash_ull>> &params, const std::shared_ptr<Instruction> &body, const Token &token) : Instruction(DEFINE, token), key(key), ftype(ftype), params(params), body(body)
{}

const Symbol DefineI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto f = std::make_shared<Function>(key, scope, params, body);
	if (key > 0) {
		auto d = Symbol(ftype, f);
		return scope->createVariable(key, d, &token);
	}
	return Symbol(ftype, f);
}

const std::string DefineI::compile() const
{
	std::string ca = "{";
	size_t i = 0;
	for (auto &e : params) {
		if (i++ > 0)
			ca += ", ";
		ca += "{static_cast<LexerTokenType>(" + std::to_string(e.first) + "), " + std::to_string(e.second) + "}";
	}
	ca += "}";
	return C_QUATERNARY("DefineI", std::to_string(key), sig::toCodeString(ftype), ca, body->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SequenceI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SequenceI::SequenceI(const std::vector<std::shared_ptr<Instruction>> &children, const Token &token) : Instruction(SEQUENCE, token), children(children)
{}

const Symbol SequenceI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	std::vector<Symbol> evals;
	for (auto &e : children) {
		if (e->getType() == UNTIL_I) {
			auto eval = e->evaluate(scope, stack_trace);
			auto v = eval.getVector(&token, stack_trace);
			evals.insert(evals.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
		} else {
			auto eval = e->evaluate(scope, stack_trace);
			evals.push_back(eval);
		}
	}
	return Symbol(evals);
}

const std::string SequenceI::compile() const
{
	std::string ca = "{";
	size_t i = 0;
	for (auto &e : children) {
		if (i++ > 0)
			ca += ", ";
		ca += e->compile();
	}
	ca += "}";
	return C_UNARY("SequenceI", ca);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IFElseI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

IfElseI::IfElseI(const std::shared_ptr<Instruction> &ifs, const std::shared_ptr<Instruction> &body, const std::shared_ptr<Instruction> &elses, const Token &token) : Instruction(IFELSE, token), ifs(ifs), body(body), elses(elses)
{}

const Symbol IfElseI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	Scope newScope(scope, 0);
	auto evalIf = ifs->evaluate(&newScope, stack_trace);
	if (evalIf.getBool(&token, stack_trace))
		return body->evaluate(&newScope, stack_trace);
	else if (elses)
		return elses->evaluate(&newScope, stack_trace);
	return Symbol();
}

const std::string IfElseI::compile() const
{
	if (elses)
		return C_TRINARY("IfElseI", ifs->compile(), body->compile(), elses->compile());
	return C_TRINARY("IfElseI", ifs->compile(), body->compile(), "nullptr");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class WhileI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

WhileI::WhileI(const std::shared_ptr<Instruction> &whiles, const std::shared_ptr<Instruction> &body, const Token &token) : Instruction(WHILE, token), whiles(whiles), body(body)
{}

const Symbol WhileI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	Scope newScope(scope, 0);
	while (whiles->evaluate(scope, stack_trace).getBool(&token, stack_trace)) {
		auto temp = body->evaluate(&newScope, stack_trace);
		switch (temp.getSymbolType()) {
			case ID_REFER:
			case ID_RETURN:
				return temp;
			case ID_BREAK:
				return Symbol();
			case ID_CONTINUE:
				continue;
			default:
				newScope.clear();
				break;
		}
	}
	return Symbol();
}

const std::string WhileI::compile() const
{
	return C_BINARY("WhileI", whiles->compile(), body->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ForI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ForI::ForI(const hash_ull &id, const std::shared_ptr<Instruction> &fors, const std::shared_ptr<Instruction> &body, const Token &token) : Instruction(FOR, token), id(id), fors(fors), body(body)
{}

const Symbol ForI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalFor = fors->evaluate(scope, stack_trace).getVector(&token, stack_trace);
	Scope newScope(scope, 0);
	for (auto &e : evalFor) {
		newScope.createVariable(id, e, &token);
		auto temp = body->evaluate(&newScope, stack_trace);
		switch (temp.getSymbolType()) {
			case ID_REFER:
			case ID_RETURN:
				return temp;
			case ID_BREAK:
				return Symbol();
			case ID_CONTINUE:
				continue;
			default:
				newScope.clear();
				break;
		}
	}
	return Symbol();
}

const std::string ForI::compile() const
{
	return C_TRINARY("ForI", std::to_string(id), fors->compile(), body->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(const hash_ull &key, const Token &token) : CastingI(VARIABLE, key, token)
{}

const Symbol VariableI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	if (key == Rossa::HASH_THIS)
		return scope->getThis(&token, stack_trace);
	return scope->getVariable(key, &token, stack_trace);
}

const std::string VariableI::compile() const
{
	return C_UNARY("VariableI", std::to_string(key));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareI::DeclareI(const hash_ull &key, const type_sll &vtype, const std::shared_ptr<Instruction> &a, const bool &isConst, const Token &token) : CastingI(DECLARE, key, token), vtype(vtype), a(a), isConst(isConst)
{}

const Symbol DeclareI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto v = scope->createVariable(key, &token);
	auto evalA = a->evaluate(scope, stack_trace);
	v.set(&evalA, &token, isConst, stack_trace);
	return v;
}

const std::string DeclareI::compile() const
{
	return C_QUATERNARY("DeclareI", std::to_string(key), std::to_string(vtype), a->compile(), isConst ? "true" : "false");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(INDEX, a, b, token)
{}

const Symbol IndexI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::index(scope, evalA, evalB, &token, stack_trace);
}

const std::string IndexI::compile() const
{
	return C_BINARY("IndexI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(INNER, a, b, token)
{}

const Symbol InnerI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType()) {
		case DICTIONARY:
			if (b->getType() == VARIABLE)
				return evalA.indexDict(ROSSA_DEHASH(reinterpret_cast<VariableI *>(b.get())->getKey()));
			throw RTError(_CANNOT_ENTER_DICTIONARY_, token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->getType() != STATIC_O && o->getType() != INSTANCE_O)
				throw RTError(_CANNOT_INDEX_OBJECT_, token, stack_trace);
			return b->evaluate(o, stack_trace);
		}
		default:
			throw RTError(_CANNOT_INDEX_VALUE_, token, stack_trace);
	}
}

const std::string InnerI::compile() const
{
	return C_BINARY("InnerI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

CallI::CallI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(INDEX, a, b, token)
{}

const Symbol CallI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto args = b->evaluate(scope, stack_trace).getVector(&token, stack_trace);
	switch (a->getType()) {
		case INNER:
		{
			auto evalA = reinterpret_cast<InnerI *>(a.get())->getA()->evaluate(scope, stack_trace);
			switch (evalA.getValueType()) {
				case OBJECT:
				{
					auto evalB = reinterpret_cast<InnerI *>(a.get())->getB()->evaluate(evalA.getObject(&token, stack_trace), stack_trace);
					return evalB.call(args, &token, stack_trace);
				}
				case DICTIONARY:
				{
					auto evalB = reinterpret_cast<InnerI *>(a.get())->getB()->evaluate(scope, stack_trace);
					return evalB.call(args, &token, stack_trace);
				}
				default:
				{
					auto evalB = reinterpret_cast<InnerI *>(a.get())->getB()->evaluate(scope, stack_trace);
					std::vector<Symbol> params;
					params.push_back(evalA);
					params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

					if (evalB.getValueType() == OBJECT) {
						auto o = evalB.getObject(&token, stack_trace);
						if (o->hasValue(Rossa::HASH_CALL))
							return o->getVariable(Rossa::HASH_CALL, &token, stack_trace).call(args, &token, stack_trace);
					}

					return evalB.call(params, &token, stack_trace);
				}
			}
		}
		default:
		{
			auto evalA = a->evaluate(scope, stack_trace);

			if (evalA.getValueType() == OBJECT) {
				auto o = evalA.getObject(&token, stack_trace);
				if (o->hasValue(Rossa::HASH_CALL))
					return o->getVariable(Rossa::HASH_CALL, &token, stack_trace).call(args, &token, stack_trace);
			}

			return evalA.call(args, &token, stack_trace);
		}
	}
}

const std::string CallI::compile() const
{
	return C_BINARY("CallI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AddI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AddI::AddI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(ADD, a, b, token)
{}

const Symbol AddI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::add(scope, evalA, evalB, &token, stack_trace);
}

const std::string AddI::compile() const
{
	return C_BINARY("AddI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SubI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SubI::SubI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(SUB, a, b, token)
{}

const Symbol SubI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::sub(scope, evalA, evalB, &token, stack_trace);
}

const std::string SubI::compile() const
{
	return C_BINARY("SubI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MulI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MulI::MulI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(MUL, a, b, token)
{}

const Symbol MulI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::mul(scope, evalA, evalB, &token, stack_trace);
}

const std::string MulI::compile() const
{
	return C_BINARY("MulI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DivI::DivI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(DIV, a, b, token)
{}

const Symbol DivI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::div(scope, evalA, evalB, &token, stack_trace);
}

const std::string DivI::compile() const
{
	return C_BINARY("DivI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ModI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ModI::ModI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(MOD, a, b, token)
{}

const Symbol ModI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::mod(scope, evalA, evalB, &token, stack_trace);
}

const std::string ModI::compile() const
{
	return C_BINARY("ModI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(POW_I, a, b, token)
{}

const Symbol PowI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::pow(scope, evalA, evalB, &token, stack_trace);
}

const std::string PowI::compile() const
{
	return C_BINARY("PowI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(LESS, a, b, token)
{}

const Symbol LessI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::less(scope, evalA, evalB, &token, stack_trace);
}

const std::string LessI::compile() const
{
	return C_BINARY("LessI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(MORE, a, b, token)
{}

const Symbol MoreI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::more(scope, evalA, evalB, &token, stack_trace);
}

const std::string MoreI::compile() const
{
	return C_BINARY("MoreI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(ELESS, a, b, token)
{}

const Symbol ELessI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::eless(scope, evalA, evalB, &token, stack_trace);
}

const std::string ELessI::compile() const
{
	return C_BINARY("ELessI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(EMORE, a, b, token)
{}

const Symbol EMoreI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::emore(scope, evalA, evalB, &token, stack_trace);
}

const std::string EMoreI::compile() const
{
	return C_BINARY("EMoreI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EqualsI::EqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(EQUALS, a, b, token)
{}

const Symbol EqualsI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Rossa::HASH_EQUALS))
		return scope->getVariable(Rossa::HASH_EQUALS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);

	return Symbol(evalA.equals(&evalB, &token, stack_trace));
}

const std::string EqualsI::compile() const
{
	return C_BINARY("EqualsI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

NEqualsI::NEqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(NEQUALS, a, b, token)
{}

const Symbol NEqualsI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Rossa::HASH_NEQUALS))
		return scope->getVariable(Rossa::HASH_NEQUALS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);

	return Symbol(evalA.nequals(&evalB, &token, stack_trace));
}

const std::string NEqualsI::compile() const
{
	return C_BINARY("NEqualsI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(AND, a, b, token)
{}

const Symbol AndI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	if (!evalA.getBool(&token, stack_trace))
		return Symbol(false);
	auto evalB = b->evaluate(scope, stack_trace);
	if (evalB.getBool(&token, stack_trace))
		return Symbol(true);
	return Symbol(false);
}

const std::string AndI::compile() const
{
	return C_BINARY("AndI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class OrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

OrI::OrI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(OR, a, b, token)
{}

const Symbol OrI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	if (evalA.getBool(&token, stack_trace))
		return Symbol(true);
	auto evalB = b->evaluate(scope, stack_trace);
	if (evalB.getBool(&token, stack_trace))
		return Symbol(true);
	return Symbol(false);
}

const std::string OrI::compile() const
{
	return C_BINARY("OrI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BOrI::BOrI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_OR, a, b, token)
{}

const Symbol BOrI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::bor(scope, evalA, evalB, &token, stack_trace);
}

const std::string BOrI::compile() const
{
	return C_BINARY("BOrI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BXOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BXOrI::BXOrI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_XOR, a, b, token)
{}

const Symbol BXOrI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::bxor(scope, evalA, evalB, &token, stack_trace);
}

const std::string BXOrI::compile() const
{
	return C_BINARY("BXOrI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BAndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BAndI::BAndI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_AND, a, b, token)
{}

const Symbol BAndI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::band(scope, evalA, evalB, &token, stack_trace);
}

const std::string BAndI::compile() const
{
	return C_BINARY("BAndI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftLeftI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftLeftI::BShiftLeftI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_SH_L, a, b, token)
{}

const Symbol BShiftLeftI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::bshl(scope, evalA, evalB, &token, stack_trace);
}

const std::string BShiftLeftI::compile() const
{
	return C_BINARY("BShiftLeftI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftRightI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftRightI::BShiftRightI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_SH_R, a, b, token)
{}

const Symbol BShiftRightI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	return ops::bshr(scope, evalA, evalB, &token, stack_trace);
}

const std::string BShiftRightI::compile() const
{
	return C_BINARY("BShiftRightI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const bool &isConst, const Token &token) : BinaryI(SET, a, b, token), isConst(isConst)
{}

const Symbol SetI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	if (evalA.getValueType() == OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Rossa::HASH_SET)) {
		try {
			return scope->getVariable(Rossa::HASH_SET, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
		} catch (const RTError &e) {
			evalA.set(&evalB, &token, isConst, stack_trace);
		}
	} else {
		evalA.set(&evalB, &token, isConst, stack_trace);
	}
	return evalA;
}

const std::string SetI::compile() const
{
	return C_TRINARY("SetI", a->compile(), b->compile(), isConst ? "true" : "false");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReturnI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ReturnI::ReturnI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(RETURN, a, token)
{}

const Symbol ReturnI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	evalA.setSymbolType(ID_RETURN);
	return evalA;
}

const std::string ReturnI::compile() const
{
	return C_UNARY("ReturnI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(const std::string &libname, const std::string &fname, const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(EXTERN, a, token), libname(libname), fname(fname)
{
	this->f = rossa::lib::loadFunction(libname, fname, &token);
}

const Symbol ExternI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	return f(a->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, Rossa::MAIN_HASH, stack_trace);
}

const std::string ExternI::compile() const
{
	return C_TRINARY("ExternI", ("\"" + libname + "\""), ("\"" + fname + "\""), a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(LENGTH, a, token)
{}

const Symbol LengthI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType()) {
		case STRING:
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
					return Symbol(RNumber::Long(evalA.getString(&token, stack_trace).size()));
			}
			return Symbol(RNumber::Long(q));
		}
		case DICTIONARY:
			return Symbol(RNumber::Long(evalA.dictionarySize(&token, stack_trace)));
		case ARRAY:
			return Symbol(RNumber::Long(evalA.vectorSize()));
		default:
			throw RTError(_FAILURE_LENGTH_, token, stack_trace);
	}
}

const std::string LengthI::compile() const
{
	return C_UNARY("LengthI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SizeI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SizeI::SizeI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(SIZE_I, a, token)
{}

const Symbol SizeI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType()) {
		case STRING:
			return Symbol(RNumber::Long(evalA.getString(&token, stack_trace).size()));
		case DICTIONARY:
			return Symbol(RNumber::Long(evalA.dictionarySize(&token, stack_trace)));
		case ARRAY:
			return Symbol(RNumber::Long(evalA.vectorSize()));
		default:
			throw RTError(_FAILURE_SIZE_, token, stack_trace);
	}
}

const std::string SizeI::compile() const
{
	return C_UNARY("SizeI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ClassI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ClassI::ClassI(const hash_ull &key, const ObjectType &type, const std::shared_ptr<Instruction> &body, const std::shared_ptr<Instruction> &extends, const Token &token) : Instruction(CLASS_I, token), key(key), type(type), body(body), extends(extends)
{}

const Symbol ClassI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	std::shared_ptr<Instruction> nbody = body;
	std::shared_ptr<Scope> o;
	Scope *ex = NULL;
	std::vector<type_sll> extensions;
	if (extends) {
		auto e = extends->evaluate(scope, stack_trace);
		if (e.getValueType() == TYPE_NAME)
			extensions.push_back(e.getTypeName(&token, stack_trace));
		else {
			ex = e.getObject(&token, stack_trace);
			if (ex->getType() == STATIC_O)
				throw RTError(_FAILURE_EXTEND_, token, stack_trace);
			auto eb = ex->getBody();
			std::vector<std::shared_ptr<Instruction>> temp;
			temp.push_back(body);
			temp.push_back(eb);
			nbody = std::make_shared<ScopeI>(temp, token);
		}
	}
	o = std::make_shared<Scope>(scope, type, nbody, key, ex, extensions);
	if (type == STATIC_O)
		body->evaluate(o.get(), stack_trace);
	return scope->createVariable(key, Symbol(o), &token);
}

const std::string ClassI::compile() const
{
	if (extends)
		return C_QUATERNARY("ClassI", std::to_string(key), ("static_cast<ObjectType>(" + std::to_string(type) + ")"), body->compile(), extends->compile());
	return C_QUATERNARY("ClassI", std::to_string(key), ("static_cast<ObjectType>(" + std::to_string(type) + ")"), body->compile(), "nullptr");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NewI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

NewI::NewI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(NEW_I, a, b, token)
{}

const Symbol NewI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace).getVector(&token, stack_trace);

	auto base = evalA.getObject(&token, stack_trace);
	return base->instantiate(evalB, &token, stack_trace);
}

const std::string NewI::compile() const
{
	return C_BINARY("NewI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastToI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

CastToI::CastToI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(CAST_TO_I, a, b, token)
{}

const Symbol CastToI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto convert = b->evaluate(scope, stack_trace).getTypeName(&token, stack_trace);

	switch (evalA.getValueType()) {
		case NUMBER:
			switch (convert) {
				case NUMBER:
					return evalA;
				case STRING:
					return Symbol(evalA.toString(&token, stack_trace));
				case BOOLEAN_D:
					return Symbol(evalA.getNumber(&token, stack_trace).getLong() != 0);
				default:
					break;
			}
			break;
		case STRING:
			switch (convert) {
				case NUMBER:
					try {
						auto s = evalA.getString(&token, stack_trace);
						if (s.length() > 2 && s[0] == '0' && isalpha(s[1])) {
							switch (s[1]) {
								case 'b':
								case 'B':
									return Symbol(RNumber::Long(std::stoll(s.substr(2), nullptr, 2)));
								default:
									return Symbol(RNumber::Long(std::stoll(s, nullptr, 0)));
							}
						}
						return Symbol(RNumber::Double(std::stold(s)));
					} catch (const std::invalid_argument &e) {
						throw RTError(format::format(_FAILURE_STR_TO_NUM_, { evalA.getString(&token, stack_trace) }), token, stack_trace);
					}
				case STRING:
					return evalA;
				case BOOLEAN_D:
					return Symbol(evalA.getString(&token, stack_trace) == KEYWORD_TRUE);
				case ARRAY:
				{
					std::string str = evalA.getString(&token, stack_trace);
					std::vector<Symbol> nv;
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
								nv.push_back(Symbol(std::string(1, str[i])));
							return Symbol(nv);
						}
						nv.push_back(Symbol(str.substr(last, s)));
						last = i + 1;
					}
					return Symbol(nv);
				}
				case TYPE_NAME:
				{
					auto s = evalA.getString(&token, stack_trace);
					if (s == KEYWORD_NUMBER)
						return Symbol(static_cast<type_sll>(NUMBER));
					if (s == KEYWORD_STRING)
						return Symbol(static_cast<type_sll>(STRING));
					if (s == KEYWORD_BOOLEAN)
						return Symbol(static_cast<type_sll>(BOOLEAN_D));
					if (s == KEYWORD_ARRAY)
						return Symbol(static_cast<type_sll>(ARRAY));
					if (s == KEYWORD_DICTIONARY)
						return Symbol(static_cast<type_sll>(DICTIONARY));
					if (s == KEYWORD_FUNCTION)
						return Symbol(static_cast<type_sll>(FUNCTION));
					if (s == KEYWORD_OBJECT)
						return Symbol(static_cast<type_sll>(OBJECT));
					if (s == KEYWORD_TYPE)
						return Symbol(static_cast<type_sll>(TYPE_NAME));
					if (s == KEYWORD_NIL_NAME)
						return Symbol(static_cast<type_sll>(NIL));
					if (s == KEYWORD_POINTER)
						return Symbol(static_cast<type_sll>(POINTER));
					return Symbol(static_cast<type_sll>(ROSSA_HASH(evalA.getString(&token, stack_trace))));
				}
				default:
					break;
			}
			break;
		case BOOLEAN_D:
			switch (convert) {
				case NUMBER:
					return Symbol(RNumber::Long(evalA.getBool(&token, stack_trace) ? 1 : 0));
				case STRING:
					return Symbol(evalA.getBool(&token, stack_trace) ? "true" : "false");
				case BOOLEAN_D:
					return evalA;
				default:
					break;
			}
			break;
		case ARRAY:
			switch (convert) {
				case STRING:
					return Symbol(evalA.toString(&token, stack_trace));
				case ARRAY:
					return evalA;
				case DICTIONARY:
				{
					auto v = evalA.getVector(&token, stack_trace);
					sym_map_t nd;
					for (size_t i = 0; i < v.size(); i++)
						nd[std::to_string(i)] = v[i];
					return Symbol(nd);
				}
				default:
					break;
			}
			break;
		case DICTIONARY:
			switch (convert) {
				case STRING:
					return Symbol(evalA.toString(&token, stack_trace));
				case ARRAY:
				{
					auto dict = evalA.getDictionary(&token, stack_trace);
					std::vector<Symbol> nv;
					for (auto &e : dict) {
						std::vector<Symbol> l = { Symbol(e.first), e.second };
						nv.push_back(Symbol(l));
					}
					return Symbol(nv);
				}
				case DICTIONARY:
					return evalA;
				default:
					break;
			}
			break;
		case OBJECT:
		{
			if (convert == evalA.getAugValueType())
				return evalA;
			auto fname = ROSSA_HASH("->" + sig::getTypeString(convert));
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(fname))
				return o->getVariable(fname, &token, stack_trace).call({ }, &token, stack_trace);
			break;
		}
		case TYPE_NAME:
			switch (convert) {
				case STRING:
					return Symbol(sig::getTypeString(evalA.getAugValueType()));
				case TYPE_NAME:
					return evalA;
				default:
					break;
			}
			break;
		default:
			break;
	}

	auto fname = ROSSA_HASH("->" + sig::getTypeString(convert));
	return scope->getVariable(fname, &token, stack_trace).call({ evalA }, &token, stack_trace);
}

const std::string CastToI::compile() const
{
	return C_BINARY("CastToI", a->compile(), b->compile());
}


/*-------------------------------------------------------------------------------------------------------*/
/*class AllocI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

AllocI::AllocI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(ALLOC_I, a, token)
{}

const Symbol AllocI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace).getNumber(&token, stack_trace).getLong();
	if (evalA < 0)
		throw RTError(_FAILURE_ALLOC_, token, stack_trace);
	return Symbol::allocate(evalA);
}

const std::string AllocI::compile() const
{
	return C_UNARY("AllocI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const std::shared_ptr<Instruction> &step, const bool &inclusive, const Token &token) : BinaryI(UNTIL_I, a, b, token), step(step), inclusive(inclusive)
{}

const Symbol UntilI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	if (step == nullptr)
		return ops::untilnostep(scope, inclusive, evalA, evalB, &token, stack_trace);
	else
		return ops::untilstep(scope, inclusive, evalA, evalB, step->evaluate(scope, stack_trace), &token, stack_trace);
}

const std::string UntilI::compile() const
{
	if (step)
		return C_QUATERNARY("UntilI", a->compile(), b->compile(), step->compile(), inclusive ? "true" : "false");
	return C_QUATERNARY("UntilI", a->compile(), b->compile(), "nullptr", inclusive ? "true" : "false");
}


/*-------------------------------------------------------------------------------------------------------*/
/*class ScopeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ScopeI::ScopeI(const std::vector<std::shared_ptr<Instruction>> &children, const Token &token) : Instruction(SCOPE_I, token), children(children)
{}

const Symbol ScopeI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	for (auto &e : children) {
		auto eval = e->evaluate(scope, stack_trace);
		if (eval.getSymbolType() != ID_CASUAL)
			return eval;
	}
	return Symbol();
}

const std::string ScopeI::compile() const
{
	std::string ca = "{";
	size_t i = 0;
	for (auto &e : children) {
		if (i++ > 0)
			ca += ", ";
		ca += e->compile();
	}
	ca += "}";
	return C_UNARY("ScopeI", ca);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MapI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

MapI::MapI(const std::map<std::string, std::shared_ptr<Instruction>> &children, const Token &token) : Instruction(MAP_I, token), children(children)
{}

const Symbol MapI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	sym_map_t evals;
	for (auto &e : children) {
		auto eval = e.second->evaluate(scope, stack_trace);
		if (eval.getValueType() == NIL)
			continue;
		evals[e.first] = eval;
	}
	return Symbol(evals);
}

const std::string MapI::compile() const
{
	std::string ca = "{";
	size_t i = 0;
	for (auto &e : children) {
		if (i++ > 0)
			ca += ", ";
		ca += "{\"" + e.first + "\", " + e.second->compile() + "}";
	}
	ca += "}";
	return C_UNARY("MapI", ca);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReferI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ReferI::ReferI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(REFER_I, a, token)
{}

const Symbol ReferI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	evalA.setSymbolType(ID_REFER);
	return evalA;
}

const std::string ReferI::compile() const
{
	return C_UNARY("ReferI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SwitchI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SwitchI::SwitchI(const std::shared_ptr<Instruction> &switchs, const std::map<Symbol, size_t> &cases_solved, const std::map<std::shared_ptr<Instruction>, size_t> &cases_unsolved, const std::vector<std::shared_ptr<Instruction>> &cases, const std::shared_ptr<Instruction> &elses, const Token &token) : Instruction(SWITCH_I, token), switchs(switchs), cases_solved(cases_solved), cases_unsolved(cases_unsolved), cases(cases), elses(elses)
{}

const Symbol SwitchI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	Scope newScope(scope, 0);
	auto eval = switchs->evaluate(&newScope, stack_trace);
	size_t index = 0;
	if (cases_solved.find(eval) != cases_solved.end()) {
		index = cases_solved.at(eval);
	} else if (!cases_unsolved.empty()) {
		for (auto &e : cases_unsolved) {
			auto evalE = e.first->evaluate(&newScope, stack_trace);
			if (evalE.equals(&eval, &token, stack_trace)) {
				index = e.second;
			}
		}
	}

	if (index > 0) {
		return cases[index - 1]->evaluate(&newScope, stack_trace);
	} else if (elses) {
		return elses->evaluate(&newScope, stack_trace);
	}

	return Symbol();
}

const std::string SwitchI::compile() const
{
	std::string cs = "{";
	size_t i = 0;
	for (auto &e : cases_solved) {
		if (i++ > 0)
			cs += ", ";
		cs += "{\"" + e.first.toCodeString() + "\", " + std::to_string(e.second) + "}";
	}
	cs += "}";

	std::string cu = "{";
	i = 0;
	for (auto &e : cases_unsolved) {
		if (i++ > 0)
			cu += ", ";
		cu += "{\"" + e.first->compile() + "\", " + std::to_string(e.second) + "}";
	}
	cu += "}";

	std::string ca = "{";
	i = 0;
	for (auto &e : cases) {
		if (i++ > 0)
			ca += ", ";
		ca += e->compile();
	}
	ca += "}";

	if (elses)
		return C_QUINARY("SwitchI", switchs->compile(), cs, cu, ca, elses->compile());
	return C_QUINARY("SwitchI", switchs->compile(), cs, cu, ca, "nullptr");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TryCatchI                                                                                        */
/*-------------------------------------------------------------------------------------------------------*/

TryCatchI::TryCatchI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const hash_ull &key, const Token &token) : BinaryI(TRY_CATCH_I, a, b, token), key(key)
{}

const Symbol TryCatchI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	try {
		Scope newScope(scope, 0);
		return a->evaluate(&newScope, stack_trace);
	} catch (const RTError &e) {
		Scope newScope(scope, 0);
		newScope.createVariable(key, Symbol(std::string(e.what())), &token);
		return b->evaluate(&newScope, stack_trace);
	}
}

const std::string TryCatchI::compile() const
{
	return C_TRINARY("TryCatchI", a->compile(), b->compile(), std::to_string(key));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ThrowI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ThrowI::ThrowI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(THROW_I, a, token)
{}

const Symbol ThrowI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	throw RTError(evalA.getString(&token, stack_trace), token, stack_trace);
	return Symbol();
}

const std::string ThrowI::compile() const
{
	return C_UNARY("ThrowI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureEqualsI::PureEqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(PURE_EQUALS, a, b, token)
{}

const Symbol PureEqualsI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);
	return Symbol(evalA.pureEquals(&evalB, &token, stack_trace));
}

const std::string PureEqualsI::compile() const
{
	return C_BINARY("PureEqualsI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureNEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureNEqualsI::PureNEqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(PURE_NEQUALS, a, b, token)
{}

const Symbol PureNEqualsI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);
	return Symbol(evalA.pureNEquals(&evalB, &token, stack_trace));
}

const std::string PureNEqualsI::compile() const
{
	return C_BINARY("PureNEqualsI", a->compile(), b->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharNI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharNI::CharNI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(CHARN_I, a, token)
{}

const Symbol CharNI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace).getString(&token, stack_trace);
	std::vector<Symbol> nv;
	for (const unsigned char &c : evalA)
		nv.push_back(Symbol(RNumber::Long(c)));
	return Symbol(nv);
}

const std::string CharNI::compile() const
{
	return C_UNARY("CharNI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharSI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharSI::CharSI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(CHARS_I, a, token)
{}

const Symbol CharSI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	switch (evalA.getValueType()) {
		case NUMBER:
			return Symbol(std::string(1, static_cast<char>(evalA.getNumber(&token, stack_trace).getLong())));
		case ARRAY:
		{
			std::string ret = "";
			auto v = evalA.getVector(&token, stack_trace);
			for (auto &e : v)
				ret.push_back(static_cast<char>(e.getNumber(&token, stack_trace).getLong()));
			return Symbol(ret);
		}
		default:
			throw RTError(_FAILURE_TO_STR_, token, stack_trace);
	}
}

const std::string CharSI::compile() const
{
	return C_UNARY("CharSI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareVarsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareVarsI::DeclareVarsI(const std::vector<hash_ull> &keys, const Token &token) : Instruction(DECLARE_VARS_I, token), keys(keys)
{}

const Symbol DeclareVarsI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	std::vector<Symbol> newvs;
	for (auto &k : keys)
		newvs.push_back(scope->createVariable(k, &token));
	return Symbol(newvs);
}

const std::string DeclareVarsI::compile() const
{
	std::string ca = "{";
	size_t i = 0;
	for (auto &e : keys) {
		if (i++ > 0)
			ca += ", ";
		ca += std::to_string(e);
	}
	ca += "}";
	return C_UNARY("DeclareVarsI", ca);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ParseI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ParseI::ParseI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(ALLOC_I, a, token)
{}

const Symbol ParseI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace).getString(&token, stack_trace);

	auto tokens = Rossa::lexString(evalA, std::filesystem::current_path() / KEYWORD_NIL);
	NodeParser np(tokens, std::filesystem::current_path() / KEYWORD_NIL);
	return np.parse()->fold()->genParser()->evaluate(scope, stack_trace);
}

const std::string ParseI::compile() const
{
	return C_UNARY("ParseI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TypeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

TypeI::TypeI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(TYPE_I, a, token)
{}

const Symbol TypeI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	return Symbol(evalA.getAugValueType());
}

const std::string TypeI::compile() const
{
	return C_UNARY("TypeI", a->compile());
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallOpI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CallOpI::CallOpI(const size_t &id, const std::vector<std::shared_ptr<Instruction>> &children, const Token &token) : Instruction(CALL_OP_I, token), id(id), children(children)
{}

const Symbol CallOpI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
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
		default:
			return Symbol();
	}
}

const std::string CallOpI::compile() const
{
	std::string ca = "{";
	size_t i = 0;
	for (auto &e : children) {
		if (i++ > 0)
			ca += ", ";
		ca += e->compile();
	}
	ca += "}";
	return C_BINARY("CallOpI", std::to_string(id), ca);
}