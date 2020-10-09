#include "Parser.h"
#include "Library.h"

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(const InstructionType &type, const Token &token) : type(type), token(token) {}

InstructionType Instruction::getType() const
{
	return type;
}

Instruction::~Instruction() {}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(const InstructionType &type, const std::shared_ptr<Instruction> &a, const Token &token) : Instruction(type, token), a(a) {}

std::shared_ptr<Instruction> UnaryI::getA() const
{
	return a;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastingI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

CastingI::CastingI(const InstructionType &type, const hashcode_t &key, const Token &token) : Instruction(type, token), key(key) {}

const hashcode_t CastingI::getKey() const
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(const InstructionType &type, const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : UnaryI(type, a, token), b(b) {}

std::shared_ptr<Instruction> BinaryI::getB() const
{
	return b;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ContainerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ContainerI::ContainerI(const Symbol &d, const Token &token) : Instruction(CONTAINER, token), d(d) {}

const Symbol ContainerI::evaluate(Scope *scope) const
{
	return d;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(const hashcode_t &key, const Signature &ftype, std::vector<std::pair<LexerTokenType, hashcode_t>> params, const std::shared_ptr<Instruction> &body, const Token &token) : Instruction(DEFINE, token), key(key), ftype(ftype), params(params), body(body) {}

const Symbol DefineI::evaluate(Scope *scope) const
{
	auto f = std::make_shared<Function>(key, scope, params, body);
	if (key > 0)
	{
		auto d = Symbol(ftype, f);
		return scope->createVariable(key, d, &token);
	}
	return Symbol(ftype, f);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SequenceI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SequenceI::SequenceI(std::vector<std::shared_ptr<Instruction>> children, const Token &token) : Instruction(SEQUENCE, token), children(children) {}

const Symbol SequenceI::evaluate(Scope *scope) const
{
	std::vector<Symbol> evals;
	for (auto &e : children)
	{
		if (e->getType() == UNTIL_I)
		{
			auto eval = e->evaluate(scope);
			auto v = eval.getVector(&token);
			evals.insert(evals.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
		}
		else
		{
			auto eval = e->evaluate(scope);
			evals.push_back(eval);
		}
	}
	return Symbol(evals);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IFElseI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

IfElseI::IfElseI(const std::shared_ptr<Instruction> &ifs, const std::shared_ptr<Instruction> &body, const std::shared_ptr<Instruction> &elses, const Token &token) : Instruction(IFELSE, token), ifs(ifs), body(body), elses(elses) {}

const Symbol IfElseI::evaluate(Scope *scope) const
{
	Scope newScope(scope, 0);
	auto evalIf = ifs->evaluate(&newScope);
	if (evalIf.getBool(&token))
		return body->evaluate(&newScope);
	else if (elses)
		return elses->evaluate(&newScope);
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class WhileI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

WhileI::WhileI(const std::shared_ptr<Instruction> &whiles, const std::shared_ptr<Instruction> &body, const Token &token) : Instruction(WHILE, token), whiles(whiles), body(body) {}

const Symbol WhileI::evaluate(Scope *scope) const
{
	Scope newScope(scope, 0);
	while (whiles->evaluate(scope).getBool(&token))
	{
		auto temp = body->evaluate(&newScope);
		switch (temp.getSymbolType())
		{
		case ID_REFER:
		case ID_RETURN:
			return temp;
		case ID_BREAK:
			return Symbol();
		default:
			newScope.clear();
		}
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ForI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ForI::ForI(const hashcode_t &id, const std::shared_ptr<Instruction> &fors, const std::shared_ptr<Instruction> &body, const Token &token) : Instruction(FOR, token), id(id), fors(fors), body(body) {}

const Symbol ForI::evaluate(Scope *scope) const
{
	auto evalFor = fors->evaluate(scope).getVector(&token);
	Scope newScope(scope, 0);
	for (auto &e : evalFor)
	{
		newScope.createVariable(id, e, &token);
		auto temp = body->evaluate(&newScope);
		switch (temp.getSymbolType())
		{
		case ID_REFER:
		case ID_RETURN:
			return temp;
		case ID_BREAK:
			return Symbol();
		default:
			newScope.clear();
		}
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(const hashcode_t &key, const Token &token) : CastingI(VARIABLE, key, token) {}

const Symbol VariableI::evaluate(Scope *scope) const
{
	auto d = scope->getVariable(key, &token);
	return d;
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareI::DeclareI(const hashcode_t &key, const object_type_t &vtype, const std::shared_ptr<Instruction> &a, const bool &isConst, const Token &token) : CastingI(DECLARE, key, token), vtype(vtype), a(a), isConst(isConst) {}

const Symbol DeclareI::evaluate(Scope *scope) const
{
	auto v = scope->createVariable(key, &token);
	auto evalA = a->evaluate(scope);
	v.set(&evalA, &token, isConst);
	return v;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(INDEX, a, b, token) {}

const Symbol IndexI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case DICTIONARY:
		if (evalB.getValueType() == NUMBER)
			return evalA.indexDict(MAIN_HASH.hashString(evalB.toString(&token)));
		return evalA.indexDict(MAIN_HASH.hashString(evalB.getString(&token)));
	case VECTOR:
		return evalA.indexVector(evalB.getNumber(&token).getLong(), &token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_INDEX))
			return o->getScope()->getVariable(Ruota::HASH_INDEX, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "[]").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(INNER, a, b, token) {}

const Symbol InnerI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case DICTIONARY:
		if (b->getType() == VARIABLE)
			return evalA.indexDict(reinterpret_cast<VariableI *>(b.get())->getKey());
		throw RuotaError(_CANNOT_ENTER_DICTIONARY_, token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->getType() != STATIC_O && o->getType() != INSTANCE_O)
			throw RuotaError(_CANNOT_INDEX_OBJECT_, token);
		return b->evaluate(o->getScope());
	}
	default:
		throw RuotaError(_CANNOT_INDEX_VALUE_, token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

CallI::CallI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(INDEX, a, b, token) {}

const Symbol CallI::evaluate(Scope *scope) const
{
	auto args = b->evaluate(scope).getVector(&token);
	switch (a->getType())
	{
	case INNER:
	{
		auto evalA = reinterpret_cast<InnerI *>(a.get())->getA()->evaluate(scope);
		switch (evalA.getValueType())
		{
		case OBJECT:
		{
			auto bb = reinterpret_cast<InnerI *>(a.get())->getB();
			if (bb->getType() == VARIABLE && evalA.getObject(&token)->hasValue(reinterpret_cast<VariableI *>(bb.get())->getKey()))
			{
				auto evalB = reinterpret_cast<InnerI *>(a.get())->getB()->evaluate(evalA.getObject(&token)->getScope());

				if (evalB.getValueType() == OBJECT)
				{
					auto o = evalB.getObject(&token);
					if (o->hasValue(Ruota::HASH_CALL))
					{
						return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(args, &evalB, &token);
					}
				}

				return evalB.call(args, &evalA, &token);
			}
			break;
		}
		case DICTIONARY:
		{
			auto bb = reinterpret_cast<InnerI *>(a.get())->getB();
			if (bb->getType() == VARIABLE && evalA.hasDictionaryKey(reinterpret_cast<VariableI *>(bb.get())->getKey()))
			{
				auto evalB = a->evaluate(scope);

				if (evalB.getValueType() == OBJECT)
				{
					auto o = evalB.getObject(&token);
					if (o->hasValue(Ruota::HASH_CALL))
					{
						return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(args, &evalB, &token);
					}
				}

				return evalB.call(args, &evalA, &token);
			}
			break;
		}
		default:
			break;
		}

		auto evalB = reinterpret_cast<InnerI *>(a.get())->getB()->evaluate(scope);
		std::vector<Symbol> params;
		params.push_back(evalA);
		params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

		if (evalB.getValueType() == OBJECT)
		{
			auto o = evalB.getObject(&token);
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(args, &evalB, &token);
			}
		}

		return evalB.call(params, NULL, &token);
	}
	default:
	{
		auto evalA = a->evaluate(scope);

		if (evalA.getValueType() == OBJECT)
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(args, &evalA, &token);
			}
		}

		if (args.size() > 0)
			return evalA.call(args, NULL, &token);
		else
			return evalA.call(args, NULL, &token);
	}
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AddI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AddI::AddI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(ADD, a, b, token) {}

const Symbol AddI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) + evalB.getNumber(&token));
	case VECTOR:
	{
		auto valA = evalA.getVector(&token);
		auto valB = evalB.getVector(&token);
		valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
		return Symbol(valA);
	}
	case STRING:
		return Symbol(evalA.getString(&token) + evalB.getString(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_ADD))
			return o->getScope()->getVariable(Ruota::HASH_ADD, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "+").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SubI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SubI::SubI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(SUB, a, b, token) {}

const Symbol SubI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) - evalB.getNumber(&token));
	case VECTOR:
	{
		auto vA = evalA.getVector(&token);
		auto vB = evalB.getVector(&token);
		std::vector<Symbol> nv;
		for (auto &e : vA)
		{
			bool flag = true;
			for (auto &e2 : vB)
			{
				if (e.equals(&e2, &token))
				{
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
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_SUB))
		{
			return o->getScope()->getVariable(Ruota::HASH_SUB, &token).call({evalB}, &evalA, &token);
		}
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "-").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MulI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MulI::MulI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(MUL, a, b, token) {}

const Symbol MulI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) * evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_MUL))
			return o->getScope()->getVariable(Ruota::HASH_MUL, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "*").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DivI::DivI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(DIV, a, b, token) {}

const Symbol DivI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) / evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_DIV))
			return o->getScope()->getVariable(Ruota::HASH_DIV, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "/").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ModI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ModI::ModI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(MOD, a, b, token) {}

const Symbol ModI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) % evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_MOD))
			return o->getScope()->getVariable(Ruota::HASH_MOD, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "%").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(POW_I, a, b, token) {}

const Symbol PowI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token).pow(evalB.getNumber(&token)));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_POW))
		{
			return o->getScope()->getVariable(Ruota::HASH_POW, &token).call({evalB}, &evalA, &token);
		}
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "**").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(LESS, a, b, token) {}

const Symbol LessI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(&token) < evalB.getNumber(&token);
	case STRING:
		return evalA.getString(&token) < evalB.getString(&token);
	case TYPE_NAME:
		return evalA.getTypeName(&token) < evalB.getTypeName(&token);
	case BOOLEAN_D:
		return evalA.getBool(&token) < evalB.getBool(&token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_LESS))
			return o->getScope()->getVariable(Ruota::HASH_LESS, &token).call({evalB}, &evalA, &token).getBool(&token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "<").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(MORE, a, b, token) {}

const Symbol MoreI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(&token) > evalB.getNumber(&token);
	case STRING:
		return evalA.getString(&token) > evalB.getString(&token);
	case TYPE_NAME:
		return evalA.getTypeName(&token) > evalB.getTypeName(&token);
	case BOOLEAN_D:
		return evalA.getBool(&token) > evalB.getBool(&token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_MORE))
			return o->getScope()->getVariable(Ruota::HASH_MORE, &token).call({evalB}, &evalA, &token).getBool(&token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % ">").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(ELESS, a, b, token) {}

const Symbol ELessI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(&token) <= evalB.getNumber(&token);
	case STRING:
		return evalA.getString(&token) <= evalB.getString(&token);
	case TYPE_NAME:
		return evalA.getTypeName(&token) <= evalB.getTypeName(&token);
	case BOOLEAN_D:
		return evalA.getBool(&token) <= evalB.getBool(&token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_ELESS))
			return o->getScope()->getVariable(Ruota::HASH_ELESS, &token).call({evalB}, &evalA, &token).getBool(&token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "<=").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(EMORE, a, b, token) {}

const Symbol EMoreI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(&token) >= evalB.getNumber(&token);
	case STRING:
		return evalA.getString(&token) >= evalB.getString(&token);
	case TYPE_NAME:
		return evalA.getTypeName(&token) >= evalB.getTypeName(&token);
	case BOOLEAN_D:
		return evalA.getBool(&token) >= evalB.getBool(&token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_EMORE))
			return o->getScope()->getVariable(Ruota::HASH_EMORE, &token).call({evalB}, &evalA, &token).getBool(&token);
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % ">=").str(), token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % ">=").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EqualsI::EqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(EQUALS, a, b, token) {}

const Symbol EqualsI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.equals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

NEqualsI::NEqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(NEQUALS, a, b, token) {}

const Symbol NEqualsI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.nequals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(AND, a, b, token) {}

const Symbol AndI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	if (!evalA.getBool(&token))
		return Symbol(false);
	auto evalB = b->evaluate(scope);
	if (evalB.getBool(&token))
		return Symbol(true);
	return Symbol(false);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class OrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

OrI::OrI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(OR, a, b, token) {}

const Symbol OrI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	if (evalA.getBool(&token))
		return Symbol(true);
	auto evalB = b->evaluate(scope);
	if (evalB.getBool(&token))
		return Symbol(true);
	return Symbol(false);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BOrI::BOrI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_OR, a, b, token) {}

const Symbol BOrI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) | evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_OR))
			return o->getScope()->getVariable(Ruota::HASH_B_OR, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "|").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BXOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BXOrI::BXOrI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_XOR, a, b, token) {}

const Symbol BXOrI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) ^ evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_XOR))
			return o->getScope()->getVariable(Ruota::HASH_B_XOR, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "^").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BAndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BAndI::BAndI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_AND, a, b, token) {}

const Symbol BAndI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) & evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_AND))
			return o->getScope()->getVariable(Ruota::HASH_B_AND, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "&").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftLeftI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftLeftI::BShiftLeftI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_SH_L, a, b, token) {}

const Symbol BShiftLeftI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) << evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_SH_L))
			return o->getScope()->getVariable(Ruota::HASH_B_SH_L, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % "<<").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftRightI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftRightI::BShiftRightI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(B_SH_R, a, b, token) {}

const Symbol BShiftRightI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throw RuotaError(_OPERATOR_UNDEFINED_DIFFERENT_TYPE_, token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) >> evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_SH_R))
			return o->getScope()->getVariable(Ruota::HASH_B_SH_R, &token).call({evalB}, &evalA, &token);
	}
	default:
		throw RuotaError((boost::format(_OPERATOR_UNDECLARED_TYPE_) % ">>").str(), token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const bool &isConst, const Token &token) : BinaryI(SET, a, b, token), isConst(isConst) {}

const Symbol SetI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	evalA.set(&evalB, &token, isConst);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReturnI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ReturnI::ReturnI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(RETURN, a, token) {}

const Symbol ReturnI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	evalA.setSymbolType(ID_RETURN);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(const std::string &id, const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(EXTERN, a, token), id(id)
{
	if (rlib::loaded.find(id) != rlib::loaded.end())
		this->f = rlib::loaded[id];
	else
		throw RuotaError((boost::format(_EXTERN_NOT_DEFINED_) % id).str(), token);
}

const Symbol ExternI::evaluate(Scope *scope) const
{
	return f(a->evaluate(scope).getVector(&token), &token, MAIN_HASH);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(LENGTH, a, token) {}

const Symbol LengthI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case STRING:
	{
		std::string str = evalA.getString(&token);
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
				return Symbol(CNumber::Long(evalA.getString(&token).size()));
		}
		return Symbol(CNumber::Long(q));
	}
	case DICTIONARY:
		return Symbol(CNumber::Long(evalA.dictionarySize(&token)));
	case VECTOR:
		return Symbol(CNumber::Long(evalA.vectorSize()));
	default:
		throw RuotaError(_FAILURE_LENGTH_, token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SizeI::SizeI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(SIZE_I, a, token) {}

const Symbol SizeI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case STRING:
		return Symbol(CNumber::Long(evalA.getString(&token).size()));
	case DICTIONARY:
		return Symbol(CNumber::Long(evalA.dictionarySize(&token)));
	case VECTOR:
		return Symbol(CNumber::Long(evalA.vectorSize()));
	default:
		throw RuotaError(_FAILURE_SIZE_, token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ClassI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ClassI::ClassI(hashcode_t key, ObjectType type, const std::shared_ptr<Instruction> &body, const std::shared_ptr<Instruction> &extends, const Token &token) : Instruction(CLASS_I, token), key(key), type(type), body(body), extends(extends) {}

const Symbol ClassI::evaluate(Scope *scope) const
{
	std::shared_ptr<Instruction> nbody = body;
	Object *ex = NULL;
	if (extends)
	{
		auto e = extends->evaluate(scope);
		auto eo = e.getObject(&token);
		ex = eo.get();
		if (eo->getType() == STATIC_O)
			throw RuotaError(_FAILURE_EXTEND_, token);
		auto eb = eo->getBody();
		std::vector<std::shared_ptr<Instruction>> temp;
		temp.push_back(body);
		temp.push_back(eb);
		nbody = std::make_shared<ScopeI>(temp, token);
	}
	std::shared_ptr<Object> o = std::make_shared<Object>(scope, type, nbody, key, ex);
	if (type == STATIC_O)
		body->evaluate(o->getScope());
	auto d = Symbol(o);
	scope->createVariable(key, d, &token);
	return d;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NewI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

NewI::NewI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(NEW_I, a, b, token) {}

const Symbol NewI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope).getVector(&token);

	auto base = evalA.getObject(&token);
	return base->instantiate(evalB, &token);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastToI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

CastToI::CastToI(const std::shared_ptr<Instruction> &a, const ValueType &convert, const Token &token) : UnaryI(CAST_TO_I, a, token), convert(convert) {}

const Symbol CastToI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (convert)
	{
	case STRING:
		switch (evalA.getValueType())
		{
		case STRING:
			return evalA;
		default:
			return Symbol(evalA.toString(&token));
		}
	case NUMBER:
		switch (evalA.getValueType())
		{
		case NUMBER:
			return evalA;
		case NIL:
			return Symbol(CNumber::Long(0));
		case STRING:
			try
			{
				return Symbol(CNumber::Double(std::stold(evalA.getString(&token))));
			}
			catch (const std::invalid_argument &e)
			{
				throw RuotaError((boost::format(_FAILURE_STR_TO_NUM_) % evalA.getString(&token)).str(), token);
			}
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_NUMBER))
				return o->getScope()->getVariable(Ruota::HASH_TO_NUMBER, &token).call({}, &evalA, &token);
			break;
		}
		default:
			break;
		}
	case BOOLEAN_D:
		switch (evalA.getValueType())
		{
		case BOOLEAN_D:
			return evalA;
		case NIL:
			return Symbol(false);
		case NUMBER:
			return Symbol(evalA.getNumber(&token).getLong() != 0);
		case STRING:
			return Symbol(evalA.getString(&token) == "true");
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_BOOLEAN))
				return o->getScope()->getVariable(Ruota::HASH_TO_BOOLEAN, &token).call({}, &evalA, &token);
			break;
		}
		default:
			break;
		}
	case DICTIONARY:
		switch (evalA.getValueType())
		{
		case DICTIONARY:
			return evalA;
		case VECTOR:
		{
			auto v = evalA.getVector(&token);
			std::map<hashcode_t, Symbol> nd;
			for (size_t i = 0; i < v.size(); i++)
				nd[MAIN_HASH.hashString(std::to_string(i))] = v[i];
			return Symbol(nd);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_DICTIONARY))
				return o->getScope()->getVariable(Ruota::HASH_TO_DICTIONARY, &token).call({}, &evalA, &token);
			break;
		}
		default:
			break;
		}
	case VECTOR:
		switch (evalA.getValueType())
		{
		case VECTOR:
			return evalA;
		case DICTIONARY:
		{
			auto dict = evalA.getDictionary(&token);
			std::vector<Symbol> nv;
			for (auto &e : dict)
				nv.push_back(Symbol({{Ruota::HASH_KEY, Symbol(MAIN_HASH.deHash(e.first))}, {Ruota::HASH_VALUE, e.second}}));
			return Symbol(nv);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_VECTOR))
				return o->getScope()->getVariable(Ruota::HASH_TO_VECTOR, &token).call({}, &evalA, &token);
			break;
		}
		case STRING:
		{
			std::string str = evalA.getString(&token);
			std::vector<Symbol> nv;
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
			return Symbol(static_cast<object_type_t>(a));
		else
			return Symbol(static_cast<object_type_t>(evalA.getObject(&token)->getHashedKey()));
	}
	case NIL:
		return Symbol();
	default:
		break;
	}
	throw RuotaError(_FAILURE_CONVERT_, token);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AllocI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

AllocI::AllocI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(ALLOC_I, a, token) {}

const Symbol AllocI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope).getNumber(&token).getLong();
	if (evalA < 0)
		throw RuotaError(_FAILURE_ALLOC_, token);
	return Symbol::allocate(evalA);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const std::shared_ptr<Instruction> &step, const bool &inclusive, const Token &token) : BinaryI(UNTIL_I, a, b, token), step(step), inclusive(inclusive) {}

const Symbol UntilI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope).getNumber(&token);
	auto evalB = b->evaluate(scope).getNumber(&token);
	auto evalStep = step->evaluate(scope).getNumber(&token);
	std::vector<Symbol> nv;
	if (inclusive)
	{
		for (; evalA <= evalB; evalA += evalStep)
			nv.push_back(Symbol(evalA));
	}
	else
	{
		for (; evalA < evalB; evalA += evalStep)
			nv.push_back(Symbol(evalA));
	}
	return Symbol(nv);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ScopeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ScopeI::ScopeI(std::vector<std::shared_ptr<Instruction>> children, const Token &token) : Instruction(SCOPE_I, token), children(children) {}

const Symbol ScopeI::evaluate(Scope *scope) const
{
	for (auto &e : children)
	{
		auto eval = e->evaluate(scope);
		if (eval.getSymbolType() != ID_CASUAL)
			return eval;
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MapI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

MapI::MapI(std::map<hashcode_t, std::shared_ptr<Instruction>> children, const Token &token) : Instruction(MAP_I, token), children(children) {}

const Symbol MapI::evaluate(Scope *scope) const
{
	std::map<hashcode_t, Symbol> evals;
	for (auto &e : children)
	{
		auto eval = e.second->evaluate(scope);
		if (eval.getValueType() == NIL)
			continue;
		evals[e.first] = eval;
	}
	return Symbol(evals);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReferI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ReferI::ReferI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(REFER_I, a, token) {}

const Symbol ReferI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	evalA.setSymbolType(ID_REFER);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SwitchI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SwitchI::SwitchI(const std::shared_ptr<Instruction> &switchs, std::map<Symbol, std::shared_ptr<Instruction>> cases_solved, std::map<std::shared_ptr<Instruction>, std::shared_ptr<Instruction>> cases_unsolved, const std::shared_ptr<Instruction> &elses, const Token &token) : Instruction(SWITCH_I, token), switchs(switchs), cases_solved(cases_solved), cases_unsolved(cases_unsolved), elses(elses) {}

const Symbol SwitchI::evaluate(Scope *scope) const
{
	Scope newScope(scope, 0);
	auto eval = switchs->evaluate(&newScope);
	if (cases_solved.find(eval) != cases_solved.end())
	{
		return cases_solved.at(eval)->evaluate(&newScope);
	}
	else if (!cases_unsolved.empty())
	{
		for (auto &e : cases_unsolved)
		{
			auto evalE = e.first->evaluate(&newScope);
			if (evalE.equals(&eval, &token))
				return e.second->evaluate(&newScope);
		}
	}
	else if (elses)
	{
		return elses->evaluate(&newScope);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TryCatchI                                                                                        */
/*-------------------------------------------------------------------------------------------------------*/

TryCatchI::TryCatchI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const hashcode_t &key, const Token &token) : BinaryI(TRY_CATCH_I, a, b, token), key(key) {}

const Symbol TryCatchI::evaluate(Scope *scope) const
{
	try
	{
		Scope newScope(scope, 0);
		return a->evaluate(&newScope);
	}
	catch (const RuotaError &e)
	{
		Scope newScope(scope, 0);
		newScope.createVariable(key, Symbol(std::string(e.what())), &token);
		return b->evaluate(&newScope);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ThrowI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ThrowI::ThrowI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(THROW_I, a, token) {}

const Symbol ThrowI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	throw RuotaError(evalA.getString(&token), token);
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureEqualsI::PureEqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(PURE_EQUALS, a, b, token) {}

const Symbol PureEqualsI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.pureEquals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureNEqualsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureNEqualsI::PureNEqualsI(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b, const Token &token) : BinaryI(PURE_NEQUALS, a, b, token) {}

const Symbol PureNEqualsI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.pureNEquals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharNI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharNI::CharNI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(CHARN_I, a, token) {}

const Symbol CharNI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope).getString(&token);
	std::vector<Symbol> nv;
	for (const unsigned char &c : evalA)
		nv.push_back(Symbol(CNumber::Long(c)));
	return Symbol(nv);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharSI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharSI::CharSI(const std::shared_ptr<Instruction> &a, const Token &token) : UnaryI(CHARS_I, a, token) {}

const Symbol CharSI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(std::string(1, static_cast<char>(evalA.getNumber(&token).getLong())));
	case VECTOR:
	{
		std::string ret = "";
		auto v = evalA.getVector(&token);
		for (auto &e : v)
			ret.push_back(static_cast<char>(e.getNumber(&token).getLong()));
		return Symbol(ret);
	}
	default:
		throw RuotaError(_FAILURE_TO_STR_, token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareVarsI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareVarsI::DeclareVarsI(const std::vector<hashcode_t> &keys, const Token &token) : Instruction(DECLARE_VARS_I, token), keys(keys) {}

const Symbol DeclareVarsI::evaluate(Scope *scope) const
{
	std::vector<Symbol> newvs;
	for (auto &k : keys)
		newvs.push_back(scope->createVariable(k, &token));
	return Symbol(newvs);
}