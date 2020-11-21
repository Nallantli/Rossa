#include "Parser.h"

using namespace ruota;

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

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(const hash_ull &key, const sig_t &ftype, std::vector<std::pair<LexerTokenType, hash_ull>> params, const std::shared_ptr<Instruction> &body, const Token &token) : Instruction(DEFINE, token), key(key), ftype(ftype), params(params), body(body)
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

/*-------------------------------------------------------------------------------------------------------*/
/*class SequenceI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SequenceI::SequenceI(std::vector<std::shared_ptr<Instruction>> children, const Token &token) : Instruction(SEQUENCE, token), children(children)
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

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(const hash_ull &key, const Token &token) : CastingI(VARIABLE, key, token)
{}

const Symbol VariableI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	if (key == Ruota::HASH_THIS)
		return scope->getThis(&token, stack_trace);
	return scope->getVariable(key, &token, stack_trace);
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

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(INDEX, a, b, token)
{}

const Symbol IndexI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	switch (evalA.getValueType()) {
		case DICTIONARY:
			if (evalB.getValueType() == NUMBER)
				return evalA.indexDict(evalB.toString(&token, stack_trace));
			return evalA.indexDict(evalB.getString(&token, stack_trace));
		case ARRAY:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.indexVector(evalB.getNumber(&token, stack_trace).getLong(), &token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_INDEX))
				return o->getVariable(Ruota::HASH_INDEX, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_INDEX, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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
				return evalA.indexDict(RUOTA_DEHASH(reinterpret_cast<VariableI *>(b.get())->getKey()));
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
						if (o->hasValue(Ruota::HASH_CALL))
							return o->getVariable(Ruota::HASH_CALL, &token, stack_trace).call(args, &token, stack_trace);
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
				if (o->hasValue(Ruota::HASH_CALL))
					return o->getVariable(Ruota::HASH_CALL, &token, stack_trace).call(args, &token, stack_trace);
			}

			return evalA.call(args, &token, stack_trace);
		}
	}
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) + evalB.getNumber(&token, stack_trace));
		case ARRAY:
		{
			if (evalB.getValueType() != ARRAY)
				break;
			auto valA = evalA.getVector(&token, stack_trace);
			auto valB = evalB.getVector(&token, stack_trace);
			valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
			return Symbol(valA);
		}
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return Symbol(evalA.getString(&token, stack_trace) + evalB.getString(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_ADD))
				return o->getVariable(Ruota::HASH_ADD, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}

	return scope->getVariable(Ruota::HASH_ADD, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) - evalB.getNumber(&token, stack_trace));
		case ARRAY:
		{
			if (evalB.getValueType() != ARRAY)
				break;
			auto vA = evalA.getVector(&token, stack_trace);
			auto vB = evalB.getVector(&token, stack_trace);
			std::vector<Symbol> nv;
			for (auto &e : vA) {
				bool flag = true;
				for (auto &e2 : vB) {
					if (e.equals(&e2, &token, stack_trace)) {
						flag = false;
						break;
					}
				}
				if (flag)
					nv.push_back(e);
			}
			return Symbol(nv);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_SUB)) {
				return o->getVariable(Ruota::HASH_SUB, &token, stack_trace).call({ evalB }, &token, stack_trace);
			}
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_SUB, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) * evalB.getNumber(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_MUL))
				return o->getVariable(Ruota::HASH_MUL, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_MUL, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) / evalB.getNumber(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_DIV))
				return o->getVariable(Ruota::HASH_DIV, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_DIV, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) % evalB.getNumber(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_MOD))
				return o->getVariable(Ruota::HASH_MOD, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_MOD, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace).pow(evalB.getNumber(&token, stack_trace)));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_POW)) {
				return o->getVariable(Ruota::HASH_POW, &token, stack_trace).call({ evalB }, &token, stack_trace);
			}
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_POW, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(&token, stack_trace) < evalB.getNumber(&token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(&token, stack_trace) < evalB.getString(&token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_LESS))
				return o->getVariable(Ruota::HASH_LESS, &token, stack_trace).call({ evalB }, &token, stack_trace).getBool(&token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_LESS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(&token, stack_trace) > evalB.getNumber(&token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(&token, stack_trace) > evalB.getString(&token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_MORE))
				return o->getVariable(Ruota::HASH_MORE, &token, stack_trace).call({ evalB }, &token, stack_trace).getBool(&token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_SUB, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(&token, stack_trace) <= evalB.getNumber(&token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(&token, stack_trace) <= evalB.getString(&token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_ELESS))
				return o->getVariable(Ruota::HASH_ELESS, &token, stack_trace).call({ evalB }, &token, stack_trace).getBool(&token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_ELESS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(&token, stack_trace) >= evalB.getNumber(&token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(&token, stack_trace) >= evalB.getString(&token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_EMORE))
				return o->getVariable(Ruota::HASH_EMORE, &token, stack_trace).call({ evalB }, &token, stack_trace).getBool(&token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_EMORE, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	if (evalA.getValueType() == OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Ruota::HASH_EQUALS))
		return scope->getVariable(Ruota::HASH_EQUALS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);

	return Symbol(evalA.equals(&evalB, &token, stack_trace));
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

	if (evalA.getValueType() == OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Ruota::HASH_NEQUALS))
		return scope->getVariable(Ruota::HASH_NEQUALS, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);

	return Symbol(evalA.nequals(&evalB, &token, stack_trace));
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

/*-------------------------------------------------------------------------------------------------------*/
/*class BOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BOrI::BOrI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_OR, a, b, token)
{}

const Symbol BOrI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) | evalB.getNumber(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_B_OR))
				return o->getVariable(Ruota::HASH_B_OR, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_B_OR, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) ^ evalB.getNumber(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_B_XOR))
				return o->getVariable(Ruota::HASH_B_XOR, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_B_XOR, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) & evalB.getNumber(&token, stack_trace));
		case STRING:
		{
			if (evalB.getValueType() != ARRAY)
				break;
			boost::format ret(evalA.getString(&token, stack_trace));
			for (auto &e : evalB.getVector(&token, stack_trace)) {
				std::string s;
				if (e.getValueType() != STRING)
					s = e.toString(&token, stack_trace);
				else
					s = e.getString(&token, stack_trace);
				ret = ret % s;
			}
			if (ret.remaining_args() > 0)
				throw RTError(_FAILURE_STRING_FORMAT_, token, stack_trace);
			return Symbol(ret.str());
		}
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_B_AND))
				return o->getVariable(Ruota::HASH_B_AND, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_B_AND, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) << evalB.getNumber(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_B_SH_L))
				return o->getVariable(Ruota::HASH_B_SH_L, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_B_SH_L, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(&token, stack_trace) >> evalB.getNumber(&token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_B_SH_R))
				return o->getVariable(Ruota::HASH_B_SH_R, &token, stack_trace).call({ evalB }, &token, stack_trace);
		}
		default:
			break;
	}
	return scope->getVariable(Ruota::HASH_B_SH_R, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
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

	if (evalA.getValueType() == OBJECT && !evalA.getObject(&token, stack_trace)->hasValue(Ruota::HASH_SET)) {
		try {
			return scope->getVariable(Ruota::HASH_SET, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
		} catch (const RTError &e) {
			evalA.set(&evalB, &token, isConst, stack_trace);
		}
	} else {
		evalA.set(&evalB, &token, isConst, stack_trace);
	}
	return evalA;
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

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(const std::string &id, const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(EXTERN, a, token), id(id)
{
	if (ruota::lib::loaded.find(id) != ruota::lib::loaded.end())
		this->f = ruota::lib::loaded[id];
	else {
		std::vector<Function> stack_trace;
		throw RTError((boost::format(_EXTERN_NOT_DEFINED_) % id).str(), token, stack_trace);
	}
}

const Symbol ExternI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	return f(a->evaluate(scope, stack_trace).getVector(&token, stack_trace), &token, Ruota::MAIN_HASH);
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

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
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

/*-------------------------------------------------------------------------------------------------------*/
/*class CastToI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

CastToI::CastToI(const std::shared_ptr<Instruction> &a, const ValueType &convert, const Token &token) : UnaryI(CAST_TO_I, a, token), convert(convert)
{}

const Symbol CastToI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	switch (convert) {
		case STRING:
			switch (evalA.getValueType()) {
				case STRING:
					return evalA;
				default:
					return Symbol(evalA.toString(&token, stack_trace));
			}
		case NUMBER:
			switch (evalA.getValueType()) {
				case NUMBER:
					return evalA;
				case NIL:
					return Symbol(RNumber::Long(0));
				case STRING:
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
						throw RTError((boost::format(_FAILURE_STR_TO_NUM_) % evalA.getString(&token, stack_trace)).str(), token, stack_trace);
					}
				case OBJECT:
				{
					auto o = evalA.getObject(&token, stack_trace);
					if (o->hasValue(Ruota::HASH_TO_NUMBER))
						return o->getVariable(Ruota::HASH_TO_NUMBER, &token, stack_trace).call({}, &token, stack_trace);
					break;
				}
				default:
					break;
			}
		case BOOLEAN_D:
			switch (evalA.getValueType()) {
				case BOOLEAN_D:
					return evalA;
				case NIL:
					return Symbol(false);
				case NUMBER:
					return Symbol(evalA.getNumber(&token, stack_trace).getLong() != 0);
				case STRING:
					return Symbol(evalA.getString(&token, stack_trace) == KEYWORD_TRUE);
				case OBJECT:
				{
					auto o = evalA.getObject(&token, stack_trace);
					if (o->hasValue(Ruota::HASH_TO_BOOLEAN))
						return o->getVariable(Ruota::HASH_TO_BOOLEAN, &token, stack_trace).call({}, &token, stack_trace);
					break;
				}
				default:
					break;
			}
		case DICTIONARY:
			switch (evalA.getValueType()) {
				case DICTIONARY:
					return evalA;
				case ARRAY:
				{
					auto v = evalA.getVector(&token, stack_trace);
					sym_map_t nd;
					for (size_t i = 0; i < v.size(); i++)
						nd[std::to_string(i)] = v[i];
					return Symbol(nd);
				}
				case OBJECT:
				{
					auto o = evalA.getObject(&token, stack_trace);
					if (o->hasValue(Ruota::HASH_TO_DICTIONARY))
						return o->getVariable(Ruota::HASH_TO_DICTIONARY, &token, stack_trace).call({}, &token, stack_trace);
					break;
				}
				default:
					break;
			}
		case ARRAY:
			switch (evalA.getValueType()) {
				case ARRAY:
					return evalA;
				case DICTIONARY:
				{
					auto dict = evalA.getDictionary(&token, stack_trace);
					std::vector<Symbol> nv;
					for (auto &e : dict)
						nv.push_back(Symbol({ {"key", Symbol(e.first)}, {"value", e.second} }));
					return Symbol(nv);
				}
				case OBJECT:
				{
					auto o = evalA.getObject(&token, stack_trace);
					if (o->hasValue(Ruota::HASH_TO_VECTOR))
						return o->getVariable(Ruota::HASH_TO_VECTOR, &token, stack_trace).call({}, &token, stack_trace);
					break;
				}
				case STRING:
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
				default:
					break;
			}
		case TYPE_NAME:
		{
			auto a = evalA.getValueType();
			if (a != OBJECT)
				return Symbol(static_cast<type_sll>(a));
			else
				return Symbol(static_cast<type_sll>(evalA.getObject(&token, stack_trace)->getHashedKey()));
		}
		case NIL:
			return Symbol();
		default:
			break;
	}
	throw RTError(_FAILURE_CONVERT_, token, stack_trace);
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

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const std::shared_ptr<Instruction> &step, const bool &inclusive, const Token &token) : BinaryI(UNTIL_I, a, b, token), step(step), inclusive(inclusive)
{}

const Symbol UntilI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace);
	auto evalB = b->evaluate(scope, stack_trace);

	switch (evalA.getValueType()) {
		case NUMBER:
		{
			if (evalB.getValueType() != NUMBER)
				break;
			auto numA = evalA.getNumber(&token, stack_trace);
			auto numB = evalB.getNumber(&token, stack_trace);
			auto numStep = (step == nullptr ? RNumber::Long(1) : step->evaluate(scope, stack_trace).getNumber(&token, stack_trace));
			std::vector<Symbol> nv;
			if (inclusive) {
				for (; numA <= numB; numA += numStep)
					nv.push_back(Symbol(numA));
			} else {
				for (; numA < numB; numA += numStep)
					nv.push_back(Symbol(numA));
			}
			return Symbol(nv);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(&token, stack_trace);
			if (o->hasValue(Ruota::HASH_RANGE)) {
				if (step == nullptr)
					return o->getVariable(Ruota::HASH_RANGE, &token, stack_trace).call({ evalB }, &token, stack_trace);
				else
					return o->getVariable(Ruota::HASH_RANGE, &token, stack_trace).call({ evalB, step->evaluate(scope, stack_trace) }, &token, stack_trace);
			}
		}
		default:
			break;
	}
	if (step == nullptr)
		return scope->getVariable(Ruota::HASH_RANGE, &token, stack_trace).call({ evalA, evalB }, &token, stack_trace);
	else
		return scope->getVariable(Ruota::HASH_RANGE, &token, stack_trace).call({ evalA, evalB, step->evaluate(scope, stack_trace) }, &token, stack_trace);
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

/*-------------------------------------------------------------------------------------------------------*/
/*class ParseI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ParseI::ParseI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(ALLOC_I, a, token)
{}

const Symbol ParseI::evaluate(Scope *scope, std::vector<Function> &stack_trace) const
{
	auto evalA = a->evaluate(scope, stack_trace).getString(&token, stack_trace);

	auto tokens = Ruota::lexString(evalA, boost::filesystem::current_path() / KEYWORD_NIL);
	NodeParser np(tokens, boost::filesystem::current_path() / KEYWORD_NIL);
	return np.parse()->fold()->genParser()->evaluate(scope, stack_trace);
}