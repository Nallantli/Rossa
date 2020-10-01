#include "Parser.h"
#include "Library.h"

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(I_TYPE type, const Token token) : type(type), token(token) {}

I_TYPE Instruction::getType() const
{
	return type;
}

Instruction::~Instruction() {}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(I_TYPE type, std::shared_ptr<Instruction> a, const Token token) : Instruction(type, token), a(a) {}

std::shared_ptr<Instruction> UnaryI::getA() const
{
	return a;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastingI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

CastingI::CastingI(I_TYPE type, hashcode_t key, const Token token) : Instruction(type, token), key(key) {}

hashcode_t CastingI::getKey() const
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(I_TYPE type, std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : UnaryI(type, a, token), b(b) {}

std::shared_ptr<Instruction> BinaryI::getB() const
{
	return b;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Container                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Container::Container(const Symbol &d, const Token token) : Instruction(CONTAINER, token), d(d) {}

const Symbol Container::evaluate(Scope *scope) const
{
	return d;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(hashcode_t key, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params, std::shared_ptr<Instruction> body, const Token token) : Instruction(DEFINE, token), key(key), ftype(ftype), params(params), body(body) {}

const Symbol DefineI::evaluate(Scope *scope) const
{
	auto f = std::make_shared<Function>(key, scope, params, body);
	if (key > 0)
	{
		auto d = Symbol(ftype, f);
		scope->createVariable(key, d, &token);
		return d;
	}

	return Symbol(NIL, f);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Sequence                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Sequence::Sequence(std::vector<std::shared_ptr<Instruction>> children, const Token token) : Instruction(SEQUENCE, token), children(children) {}

const Symbol Sequence::evaluate(Scope *scope) const
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

IfElseI::IfElseI(std::shared_ptr<Instruction> ifs, std::shared_ptr<Instruction> body, std::shared_ptr<Instruction> elses, const Token token) : Instruction(IFELSE, token), ifs(ifs), body(body), elses(elses) {}

const Symbol IfElseI::evaluate(Scope *scope) const
{
	Scope newScope(scope, "");
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

WhileI::WhileI(std::shared_ptr<Instruction> whiles, std::shared_ptr<Instruction> body, const Token token) : Instruction(WHILE, token), whiles(whiles), body(body) {}

const Symbol WhileI::evaluate(Scope *scope) const
{
	while (whiles->evaluate(scope).getBool(&token))
	{
		Scope newScope(scope, "");
		auto temp = body->evaluate(&newScope);
		if (temp.getSymbolType() == ID_RETURN || temp.getSymbolType() == ID_REFER)
			return temp;
		if (temp.getSymbolType() == ID_BREAK)
			break;
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ForI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ForI::ForI(hashcode_t id, std::shared_ptr<Instruction> fors, std::shared_ptr<Instruction> body, const Token token) : Instruction(FOR, token), id(id), fors(fors), body(body) {}

const Symbol ForI::evaluate(Scope *scope) const
{
	auto evalFor = fors->evaluate(scope).getVector(&token);
	for (size_t i = 0; i < evalFor.size(); i++)
	{
		Scope newScope(scope, "");
		newScope.createVariable(id, evalFor[i], &token);
		auto temp = body->evaluate(&newScope);
		if (temp.getSymbolType() == ID_RETURN || temp.getSymbolType() == ID_REFER)
			return temp;
		if (temp.getSymbolType() == ID_BREAK)
			break;
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(hashcode_t key, const Token token) : CastingI(VARIABLE, key, token) {}

const Symbol VariableI::evaluate(Scope *scope) const
{
	auto d = scope->getVariable(key, &token);
	return d;
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareI::DeclareI(hashcode_t key, const Token token) : CastingI(DECLARE, key, token) {}

const Symbol DeclareI::evaluate(Scope *scope) const
{
	auto d = scope->createVariable(key, &token);
	return d;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(INDEX, a, b, token) {}

const Symbol IndexI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case DICTIONARY:
		return evalA.indexDict(hash.hashString(evalB.getString(&token)));
	case VECTOR:
		return evalA.indexVector(NUMBER_GET_LONG(evalB.getNumber(&token)), &token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_INDEX))
			return o->getScope()->getVariable(Ruota::HASH_INDEX, &token).call(NIL, {evalB}, &evalA, &token);
		throwError("Operator `[]` is undefined for Object type", &token);
	}
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(INNER, a, b, token) {}

const Symbol InnerI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case DICTIONARY:
		if (b->getType() == VARIABLE)
			evalA.indexDict(hash.hashString(b->evaluate(scope).getString(&token)));
		throwError("Cannot enter Dictionary with given value", &token);
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->getType() != STATIC_O && o->getType() != INSTANCE_O)
			throwError("Cannot index a non-static, non-instantiated Object", &token);
		return b->evaluate(o->getScope());
	}
	default:
		throwError("Cannot enter value", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

CallI::CallI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(INDEX, a, b, token) {}

const Symbol CallI::evaluate(Scope *scope) const
{
	auto args = b->evaluate(scope).getVector(&token);
	switch (a->getType())
	{
	case INNER:
	{
		auto evalA = ((InnerI *)a.get())->getA()->evaluate(scope);
		switch (evalA.getValueType())
		{
		case OBJECT:
		{
			auto bb = ((InnerI *)a.get())->getB();
			if (bb->getType() == VARIABLE && evalA.getObject(&token)->hasValue(((VariableI *)bb.get())->getKey()))
			{
				auto evalB = ((InnerI *)a.get())->getB()->evaluate(evalA.getObject(&token)->getScope());

				if (evalB.getValueType() == OBJECT)
				{
					auto o = evalB.getObject(&token);
					if (o->hasValue(Ruota::HASH_CALL))
					{
						return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(NIL, args, &evalB, &token);
					}
				}

				return evalB.call(NIL, args, &evalA, &token);
			}
			break;
		}
		case DICTIONARY:
		{
			auto bb = ((InnerI *)a.get())->getB();
			if (bb->getType() == VARIABLE && evalA.hasDictionaryKey(((VariableI *)bb.get())->getKey()))
			{
				auto evalB = a->evaluate(scope);

				if (evalB.getValueType() == OBJECT)
				{
					auto o = evalB.getObject(&token);
					if (o->hasValue(Ruota::HASH_CALL))
					{
						return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(NIL, args, &evalB, &token);
					}
				}

				return evalB.call(NIL, args, &evalA, &token);
			}
			break;
		}
		}

		auto evalB = ((InnerI *)a.get())->getB()->evaluate(scope);
		std::vector<Symbol> params;
		params.push_back(evalA);
		params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

		if (evalB.getValueType() == OBJECT)
		{
			auto o = evalB.getObject(&token);
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(NIL, args, &evalB, &token);
			}
		}

		return evalB.call(evalA.getValueType(), params, NULL, &token);
	}
	default:
	{
		auto evalA = a->evaluate(scope);

		if (evalA.getValueType() == OBJECT)
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL, &token).call(NIL, args, &evalA, &token);
			}
		}

		if (args.size() > 0)
			return evalA.call(args[0].getValueType(), args, NULL, &token);
		else
			return evalA.call(NIL, args, NULL, &token);
	}
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AddI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AddI::AddI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(ADD, a, b, token) {}

const Symbol AddI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
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
		{
			return o->getScope()->getVariable(Ruota::HASH_ADD, &token).call(NIL, {evalB}, &evalA, &token);
		}
	}
	default:
		throwError("Operator `+` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SubI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SubI::SubI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(SUB, a, b, token) {}

const Symbol SubI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) - evalB.getNumber(&token));
	case VECTOR:
	{
		auto vA = evalA.getVector(&token);
		auto vB = evalB.getVector(&token);
		std::vector<Symbol> nv;
		for (auto &e : vA) {
			bool flag = true;
			for (auto &e2 : vB) {
				if (e.equals(&e2, &token)){
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
			return o->getScope()->getVariable(Ruota::HASH_SUB, &token).call(NIL, {evalB}, &evalA, &token);
		}
	}
	default:
		throwError("Operator `-` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MulI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MulI::MulI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(MUL, a, b, token) {}

const Symbol MulI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) * evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_MUL))
			return o->getScope()->getVariable(Ruota::HASH_MUL, &token).call(NIL, {evalB}, &evalA, &token);
	}
	default:
		throwError("Operator `*` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DivI::DivI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(DIV, a, b, token) {}

const Symbol DivI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) / evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_DIV))
			return o->getScope()->getVariable(Ruota::HASH_DIV, &token).call(NIL, {evalB}, &evalA, &token);
	}
	default:
		throwError("Operator `/` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ModI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ModI::ModI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(MOD, a, b, token) {}

const Symbol ModI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) % evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_MOD))
			return o->getScope()->getVariable(Ruota::HASH_MOD, &token).call(NIL, {evalB}, &evalA, &token);
	}
	default:
		throwError("Operator `%` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(POW_I, a, b, token) {}

const Symbol PowI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(NUMBER_POW(evalA.getNumber(&token), evalB.getNumber(&token)));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_POW))
		{
			return o->getScope()->getVariable(Ruota::HASH_POW, &token).call(NIL, {evalB}, &evalA, &token);
		}
	}
	default:
		throwError("Operator `**` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(LESS, a, b, token) {}

const Symbol LessI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
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
			return o->getScope()->getVariable(Ruota::HASH_LESS, &token).call(NIL, {evalB}, &evalA, &token).getBool(&token);
		throwError("Operator `<` not defined for Object type", &token);
	}
	default:
		return evalA.toString(&token) < evalB.toString(&token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(MORE, a, b, token) {}

const Symbol MoreI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
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
			return o->getScope()->getVariable(Ruota::HASH_MORE, &token).call(NIL, {evalB}, &evalA, &token).getBool(&token);
		throwError("Operator `>` not defined for Object type", &token);
	}
	default:
		return evalA.toString(&token) > evalB.toString(&token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(ELESS, a, b, token) {}

const Symbol ELessI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
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
			return o->getScope()->getVariable(Ruota::HASH_ELESS, &token).call(NIL, {evalB}, &evalA, &token).getBool(&token);
		throwError("Operator `<=` not defined for Object type", &token);
	}
	default:
		return evalA.toString(&token) <= evalB.toString(&token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(EMORE, a, b, token) {}

const Symbol EMoreI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
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
			return o->getScope()->getVariable(Ruota::HASH_EMORE, &token).call(NIL, {evalB}, &evalA, &token).getBool(&token);
		throwError("Operator `>=` not defined for Object type", &token);
	}
	default:
		return evalA.toString(&token) >= evalB.toString(&token);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Equals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Equals::Equals(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(EQUALS, a, b, token) {}

const Symbol Equals::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.equals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NEquals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

NEquals::NEquals(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(NEQUALS, a, b, token) {}

const Symbol NEquals::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.nequals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(AND, a, b, token) {}

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

OrI::OrI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(OR, a, b, token) {}

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

BOrI::BOrI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(B_OR, a, b, token) {}

const Symbol BOrI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) | evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_OR))
			return o->getScope()->getVariable(Ruota::HASH_B_OR, &token).call(NIL, {evalB}, &evalA, &token);
	}
	default:
		throwError("Operator `|` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BXOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BXOrI::BXOrI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(B_XOR, a, b, token) {}

const Symbol BXOrI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) ^ evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_XOR))
			return o->getScope()->getVariable(Ruota::HASH_B_XOR, &token).call(NIL, {evalB}, &evalA, &token);
	}
	default:
		throwError("Operator `^` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BAndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BAndI::BAndI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(B_AND, a, b, token) {}

const Symbol BAndI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) & evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_AND))
		{
			return o->getScope()->getVariable(Ruota::HASH_B_AND, &token).call(NIL, {evalB}, &evalA, &token);
		}
	}
	default:
		throwError("Operator `&` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftLeft                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftLeft::BShiftLeft(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(B_SH_L, a, b, token) {}

const Symbol BShiftLeft::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) << evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_SH_L))
			return o->getScope()->getVariable(Ruota::HASH_B_SH_L, &token).call(NIL, {evalB}, &evalA, &token);
	}
	default:
		throwError("Operator `<<` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftRight                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftRight::BShiftRight(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(B_SH_R, a, b, token) {}

const Symbol BShiftRight::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	if (evalA.getValueType() != OBJECT && evalA.getValueType() != evalB.getValueType())
		throwError("Operator is undefined for base values of different types", &token);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(&token) >> evalB.getNumber(&token));
	case OBJECT:
	{
		auto o = evalA.getObject(&token);
		if (o->hasValue(Ruota::HASH_B_SH_R))
			return o->getScope()->getVariable(Ruota::HASH_B_SH_R, &token).call(NIL, {evalB}, &evalA, &token);
	}
	default:
		throwError("Operator `>>` is undefined for value type", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(SET, a, b, token) {}

const Symbol SetI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	evalA.set(&evalB, &token);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReturnI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ReturnI::ReturnI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(RETURN, a, token) {}

const Symbol ReturnI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	evalA.setSymbolType(ID_RETURN);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(const std::string &id, std::shared_ptr<Instruction> a, const Token token) : UnaryI(EXTERN, a, token), id(id)
{
	if (rlib::loaded.find(id) != rlib::loaded.end())
		this->f = rlib::loaded[id];
	else
		throwError("External function `" + id + "` is not defined", &token);
}

const Symbol ExternI::evaluate(Scope *scope) const
{
	return f(a->evaluate(scope).getVector(&token), &token);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(LENGTH, a, token) {}

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
			c = (unsigned char)str[i];
			if (c >= 0 && c <= 127)
				i += 0;
			else if ((c & 0xE0) == 0xC0)
				i += 1;
			else if ((c & 0xF0) == 0xE0)
				i += 2;
			else if ((c & 0xF8) == 0xF0)
				i += 3;
			else
				return Symbol(NUMBER_NEW_LONG(evalA.getString(&token).size()));
		}
		return Symbol(NUMBER_NEW_LONG(q));
	}
	case DICTIONARY:
		return Symbol(NUMBER_NEW_LONG(evalA.dictionarySize(&token)));
	case VECTOR:
		return Symbol(NUMBER_NEW_LONG(evalA.vectorSize()));
	default:
		throwError("Cannot get length of value", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SizeI::SizeI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(SIZE_I, a, token) {}

const Symbol SizeI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case STRING:
		return Symbol(NUMBER_NEW_LONG(evalA.getString(&token).size()));
	case DICTIONARY:
		return Symbol(NUMBER_NEW_LONG(evalA.dictionarySize(&token)));
	case VECTOR:
		return Symbol(NUMBER_NEW_LONG(evalA.vectorSize()));
	default:
		throwError("Cannot get size of value", &token);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ClassI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ClassI::ClassI(hashcode_t key, OBJECT_TYPE type, std::shared_ptr<Instruction> body, std::shared_ptr<Instruction> extends, const Token token) : Instruction(CLASS_I, token), key(key), type(type), body(body), extends(extends) {}

const Symbol ClassI::evaluate(Scope *scope) const
{
	std::shared_ptr<Instruction> nbody = body;
	if (extends)
	{
		auto e = extends->evaluate(scope);
		auto eo = e.getObject(&token);
		if (eo->getType() == STATIC_O)
			throwError("Cannot extend a statically declared Object", &token);
		auto eb = eo->getBody();
		std::vector<std::shared_ptr<Instruction>> temp;
		temp.push_back(body);
		temp.push_back(eb);
		nbody = std::make_shared<ScopeI>(temp, token);
	}
	std::shared_ptr<Object> o = std::make_shared<Object>(scope, type, nbody, hash.deHash(key));
	if (type == STATIC_O)
		body->evaluate(o->getScope());
	auto d = Symbol(o);
	scope->createVariable(key, d, &token);
	return d;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NewI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

NewI::NewI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(NEW_I, a, b, token) {}

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

CastToI::CastToI(std::shared_ptr<Instruction> a, D_TYPE convert, const Token token) : UnaryI(CAST_TO_I, a, token), convert(convert) {}

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
			return Symbol(NUMBER_NEW_LONG(0));
		case STRING:
			try
			{
				return Symbol(NUMBER_NEW_DOUBLE(std::stold(evalA.getString(&token))));
			}
			catch (const std::invalid_argument &e)
			{
				throwError("String `" + evalA.getString(&token) + "` cannot be converted to Number", &token);
			}
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_NUMBER))
				return o->getScope()->getVariable(Ruota::HASH_TO_NUMBER, &token).call(NIL, {}, &evalA, &token);
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
			return Symbol(NUMBER_GET_LONG(evalA.getNumber(&token)) != 0);
		case STRING:
			return Symbol(evalA.getString(&token) == "true");
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_BOOLEAN))
				return o->getScope()->getVariable(Ruota::HASH_TO_BOOLEAN, &token).call(NIL, {}, &evalA, &token);
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
				nd[hash.hashString(std::to_string(i))] = v[i];
			return Symbol(nd);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_DICTIONARY))
				return o->getScope()->getVariable(Ruota::HASH_TO_DICTIONARY, &token).call(NIL, {}, &evalA, &token);
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
				nv.push_back(Symbol({{Ruota::HASH_KEY, Symbol(hash.deHash(e.first))}, {Ruota::HASH_VALUE, e.second}}));
			return Symbol(nv);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(&token);
			if (o->hasValue(Ruota::HASH_TO_VECTOR))
				return o->getScope()->getVariable(Ruota::HASH_TO_VECTOR, &token).call(NIL, {}, &evalA, &token);
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
				c = (unsigned char)str[i];
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
			return Symbol(static_cast<signed long long>(a));
		else
			return Symbol(static_cast<signed long long>(hash.hashString(evalA.getObject(&token)->getName())));
	}
	case NIL:
		return Symbol();
	default:
		break;
	}
	throwError("Cannot convert to given type", &token);
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AllocI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

AllocI::AllocI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(ALLOC_I, a, token) {}

const Symbol AllocI::evaluate(Scope *scope) const
{
	auto evalA = NUMBER_GET_LONG(a->evaluate(scope).getNumber(&token));
	if (evalA < 0)
		throwError("Cannot initialize a Vector with size < 0", &token);
	std::vector<Symbol> v(evalA);
	return Symbol(v);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(UNTIL_I, a, b, token) {}

const Symbol UntilI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope).getNumber(&token);
	auto evalB = b->evaluate(scope).getNumber(&token);
	std::vector<Symbol> nv;
	for (auto i = evalA; i < evalB; i += 1)
		nv.push_back(Symbol(i));
	return Symbol(nv);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ScopeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ScopeI::ScopeI(std::vector<std::shared_ptr<Instruction>> children, const Token token) : Instruction(SCOPE_I, token), children(children) {}

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

MapI::MapI(std::map<hashcode_t, std::shared_ptr<Instruction>> children, const Token token) : Instruction(MAP_I, token), children(children) {}

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

ReferI::ReferI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(REFER_I, a, token) {}

const Symbol ReferI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	evalA.setSymbolType(ID_REFER);
	return evalA;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SwitchI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SwitchI::SwitchI(std::shared_ptr<Instruction> switchs, std::map<Symbol, std::shared_ptr<Instruction>> cases, std::shared_ptr<Instruction> elses, const Token token) : Instruction(SWITCH_I, token), switchs(switchs), cases(cases), elses(elses) {}

const Symbol SwitchI::evaluate(Scope *scope) const
{
	auto eval = switchs->evaluate(scope);
	if (cases.find(eval) != cases.end())
	{
		Scope newScope(scope, "");
		return cases.at(eval)->evaluate(&newScope);
	}
	else if (elses)
	{
		Scope newScope(scope, "");
		return elses->evaluate(&newScope);
	}
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TryCatchI                                                                                        */
/*-------------------------------------------------------------------------------------------------------*/

TryCatchI::TryCatchI(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, hashcode_t key, const Token token) : BinaryI(TRY_CATCH_I, a, b, token), key(key) {}

const Symbol TryCatchI::evaluate(Scope *scope) const
{
	try
	{
		Scope newScope(scope, "");
		return a->evaluate(&newScope);
	}
	catch (const std::runtime_error &e)
	{
		Scope newScope(scope, "");
		newScope.createVariable(key, Symbol(std::string(e.what())), &token);
		return b->evaluate(&newScope);
	}
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ThrowI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ThrowI::ThrowI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(THROW_I, a, token) {}

const Symbol ThrowI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	throwError(evalA.getString(&token), &token);
	return Symbol();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureEquals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureEquals::PureEquals(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(PURE_EQUALS, a, b, token) {}

const Symbol PureEquals::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.pureEquals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureNEquals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureNEquals::PureNEquals(std::shared_ptr<Instruction> a, std::shared_ptr<Instruction> b, const Token token) : BinaryI(PURE_NEQUALS, a, b, token) {}

const Symbol PureNEquals::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.pureNEquals(&evalB, &token));
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharNI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharNI::CharNI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(CHARN_I, a, token) {}

const Symbol CharNI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope).getString(&token);
	std::vector<Symbol> nv;
	for (const unsigned char &c : evalA)
		nv.push_back(Symbol(NUMBER_NEW_LONG(c)));
	return Symbol(nv);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharSI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharSI::CharSI(std::shared_ptr<Instruction> a, const Token token) : UnaryI(CHARS_I, a, token) {}

const Symbol CharSI::evaluate(Scope *scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(std::string(1, static_cast<char>(NUMBER_GET_LONG(evalA.getNumber(&token)))));
	case VECTOR:
	{
		std::string ret = "";
		auto v = evalA.getVector(&token);
		for (auto &e : v)
			ret.push_back(static_cast<char>(NUMBER_GET_LONG(e.getNumber(&token))));
		return Symbol(ret);
	}
	default:
		throwError("Cannot convert value(s) into String", &token);
	}
	return Symbol();
}