#include "Parser.h"
#include "Library.h"

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(I_TYPE type, Token *token) : type(type), token(token) {}

I_TYPE Instruction::getType() const
{
	return type;
}

Instruction::~Instruction()
{
	//delete token;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(I_TYPE type, Instruction *a, Token *token) : Instruction(type, token), a(a) {}

Instruction *UnaryI::getA() const
{
	return a;
}

UnaryI::~UnaryI()
{
	delete a;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastingI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

CastingI::CastingI(I_TYPE type, hashcode_t key, Token *token) : Instruction(type, token), key(key) {}

hashcode_t CastingI::getKey() const
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(I_TYPE type, Instruction *a, Instruction *b, Token *token) : UnaryI(type, a, token), b(b) {}

Instruction *BinaryI::getB() const
{
	return b;
}

BinaryI::~BinaryI()
{
	delete b;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Container                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Container::Container(const Symbol &d, Token *token) : Instruction(CONTAINER, token), d(d) {}

const Symbol Container::evaluate(Scope * scope) const
{
	return d;
}

const std::string Container::toString(bool shared) const
{
	if (shared)
		return "std::shared_ptr<Container>(new Container(" + d.toCodeString() + "))";
	return "new Container(" + d.toCodeString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(hashcode_t key, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params, std::shared_ptr<Instruction> body, Token *token) : Instruction(DEFINE, token), key(key), ftype(ftype), params(params), body(body) {}

const Symbol DefineI::evaluate(Scope * scope) const
{
	auto f = std::make_shared<Function>(key, scope, params, body);
	if (key > 0)
	{
		auto d = Symbol(ftype, f);
		scope->createVariable(key, d, token);
		return d;
	}

	return Symbol(NIL, f);
}

const std::string DefineI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<DefineI>(new DefineI(";
	else
		ret = "new DefineI(";
	ret += std::to_string(key) + ", static_cast<D_TYPE>(" + std::to_string(ftype) + "), {";
	unsigned long i = 0;
	for (auto &e : params)
	{
		if (i > 0)
			ret += ", ";
		ret += "{static_cast<LEX_TOKEN_TYPE>(" + std::to_string(e.first) + "), " + std::to_string(e.second) + "}";
	}
	ret += "}, ";
	ret += body->toString(true);
	return ret + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Sequence                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Sequence::Sequence(std::vector<Instruction *> children, Token *token) : Instruction(SEQUENCE, token), children(children) {}

const Symbol Sequence::evaluate(Scope * scope) const
{
	std::vector<Symbol> evals;
	for (auto &e : children)
	{
		if (e->getType() == UNTIL_I)
		{
			auto eval = e->evaluate(scope);
			auto v = eval.getVector(token);
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

const std::string Sequence::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<Sequence>(new Sequence({";
	else
		ret = "new Sequence({";

	unsigned long i = 0;
	for (auto &e : children)
	{
		if (i > 0)
			ret += ", ";
		ret += e->toString(false);
		i++;
	}
	return ret + "})" + (shared ? ")" : "");
}

Sequence::~Sequence()
{
	for (auto &e : children)
		delete e;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IFElseI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

IfElseI::IfElseI(Instruction *ifs, Instruction *body, Instruction *elses, Token *token) : Instruction(IFELSE, token), ifs(ifs), body(body), elses(elses) {}

const Symbol IfElseI::evaluate(Scope * scope) const
{
	Scope newScope(scope, "");
	auto evalIf = ifs->evaluate(&newScope);
	if (evalIf.getBool(token))
	{
		return body->evaluate(&newScope);
	}
	else if (elses)
	{
		return elses->evaluate(&newScope);
	}
	return Symbol();
}

const std::string IfElseI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<IfElseI>(new IfElseI(";
	else
		ret = "new IfElseI(";
	ret += ifs->toString(false) + ", " + body->toString(false);
	if (elses)
		ret += ", " + elses->toString(false);
	return ret + ")" + (shared ? ")" : "");
}

IfElseI::~IfElseI()
{
	delete ifs;
	delete body;
	if (elses)
		delete elses;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class WhileI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

WhileI::WhileI(Instruction *whiles, Instruction *body, Token *token) : Instruction(WHILE, token), whiles(whiles), body(body) {}

const Symbol WhileI::evaluate(Scope * scope) const
{
	while (whiles->evaluate(scope).getBool(token))
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

const std::string WhileI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<WhileI>(new WhileI(";
	else
		ret = "new WhileI(";
	ret += whiles->toString(false) + ", " + body->toString(false);
	return ret + ")" + (shared ? ")" : "");
}

WhileI::~WhileI()
{
	delete whiles;
	delete body;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ForI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ForI::ForI(hashcode_t id, Instruction *fors, Instruction *body, Token *token) : Instruction(FOR, token), id(id), fors(fors), body(body) {}

const Symbol ForI::evaluate(Scope * scope) const
{
	auto evalFor = fors->evaluate(scope).getVector(token);
	for (size_t i = 0; i < evalFor.size(); i++)
	{
		Scope newScope(scope, "");
		newScope.createVariable(id, evalFor[i], token);
		auto temp = body->evaluate(&newScope);
		if (temp.getSymbolType() == ID_RETURN || temp.getSymbolType() == ID_REFER)
			return temp;
		if (temp.getSymbolType() == ID_BREAK)
			break;
	}
	return Symbol();
}

const std::string ForI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<ForI>(new ForI(";
	else
		ret = "new ForI(";
	ret += std::to_string(id) + ", ";
	ret += fors->toString(false) + ", " + body->toString(false);
	return ret + ")" + (shared ? ")" : "");
}

ForI::~ForI()
{
	delete fors;
	delete body;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(hashcode_t key, Token *token) : CastingI(VARIABLE, key, token) {}

const Symbol VariableI::evaluate(Scope * scope) const
{
	auto d = scope->getVariable(key, token);
	return d;
	return Symbol();
}

const std::string VariableI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<VariableI>(new VariableI(";
	else
		ret = "new VariableI(";
	return ret + std::to_string(key) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareI::DeclareI(hashcode_t key, Token *token) : CastingI(DECLARE, key, token) {}

const Symbol DeclareI::evaluate(Scope * scope) const
{
	auto d = scope->createVariable(key, token);
	return d;
}

const std::string DeclareI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<DeclareI>(new DeclareI(";
	else
		ret = "new DeclareI(";
	return ret + std::to_string(key) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(Instruction *a, Instruction *b, Token *token) : BinaryI(INDEX, a, b, token) {}

const Symbol IndexI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case DICTIONARY:
		return evalA.indexDict(hash.hashString(evalB.getString(token)));
	case VECTOR:
	{
		auto i = NUMBER_GET_LONG(evalB.getNumber(token));
		auto v = evalA.getVector(token);
		if (i >= v.size())
			throwError("Array index out of bounds: size = " + std::to_string(v.size()) + ", got " + std::to_string(i), token);
		return v[i];
	}
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_INDEX))
		{
			return o->getScope()->getVariable(Ruota::HASH_INDEX, token).call(NIL, {evalB}, &evalA, token);
		}
		throwError("Operator `[]` is undefined for Object type", token);
	}
	}
	return Symbol();
}

const std::string IndexI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<IndexI>(new IndexI(";
	else
		ret = "new IndexI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(Instruction *a, Instruction *b, Token *token) : BinaryI(INNER, a, b, token) {}

const Symbol InnerI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case DICTIONARY:
		if (b->getType() == VARIABLE)
			evalA.indexDict(hash.hashString(b->evaluate(scope).getString(token)));
		throwError("Cannot enter Dictionary with given value", token);
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->getType() != STATIC_O && o->getType() != INSTANCE_O)
			throwError("Cannot index a non-static, non-instantiated Object", token);
		return b->evaluate(o->getScope());
	}
	default:
		throwError("Cannot enter value", token);
	}
	return Symbol();
}

const std::string InnerI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<InnerI>(new InnerI(";
	else
		ret = "new InnerI(";

	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

CallI::CallI(Instruction *a, Instruction *b, Token *token) : BinaryI(INDEX, a, b, token) {}

const Symbol CallI::evaluate(Scope * scope) const
{
	auto args = b->evaluate(scope).getVector(token);
	switch (a->getType())
	{
	case INNER:
	{
		auto evalA = ((InnerI *)a)->getA()->evaluate(scope);
		switch (evalA.getValueType())
		{
		case OBJECT:
		{
			auto bb = ((InnerI *)a)->getB();
			if (bb->getType() == VARIABLE && evalA.getObject(token)->hasValue(((VariableI *)bb)->getKey()))
			{
				auto evalB = ((InnerI *)a)->getB()->evaluate(evalA.getObject(token)->getScope());

				if (evalB.getValueType() == OBJECT)
				{
					auto o = evalB.getObject(token);
					if (o->hasValue(Ruota::HASH_CALL))
					{
						return o->getScope()->getVariable(Ruota::HASH_CALL, token).call(NIL, args, &evalB, token);
					}
				}

				return evalB.call(NIL, args, &evalA, token);
			}
			break;
		}
		case DICTIONARY:
		{
			auto bb = ((InnerI *)a)->getB();
			if (bb->getType() == VARIABLE && evalA.hasDictionaryKey(((VariableI *)bb)->getKey()))
			{
				auto evalB = a->evaluate(scope);

				if (evalB.getValueType() == OBJECT)
				{
					auto o = evalB.getObject(token);
					if (o->hasValue(Ruota::HASH_CALL))
					{
						return o->getScope()->getVariable(Ruota::HASH_CALL, token).call(NIL, args, &evalB, token);
					}
				}

				return evalB.call(NIL, args, &evalA, token);
			}
			break;
		}
		}

		auto evalB = ((InnerI *)a)->getB()->evaluate(scope);
		std::vector<Symbol> params;
		params.push_back(evalA);
		params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

		if (evalB.getValueType() == OBJECT)
		{
			auto o = evalB.getObject(token);
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL, token).call(NIL, args, &evalB, token);
			}
		}

		return evalB.call(evalA.getValueType(), params, token);
	}
	default:
	{
		auto evalA = a->evaluate(scope);

		if (evalA.getValueType() == OBJECT)
		{
			auto o = evalA.getObject(token);
			if (o->hasValue(Ruota::HASH_CALL))
			{
				return o->getScope()->getVariable(Ruota::HASH_CALL, token).call(NIL, args, &evalA, token);
			}
		}

		if (args.size() > 0)
			return evalA.call(args[0].getValueType(), args, token);
		else
			return evalA.call(NIL, args, token);
	}
	}
}

const std::string CallI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<CallI>(new CallI(";
	else
		ret = "new CallI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AddI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AddI::AddI(Instruction *a, Instruction *b, Token *token) : BinaryI(ADD, a, b, token) {}

const Symbol AddI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) + evalB.getNumber(token));
	case VECTOR:
	{
		auto valA = evalA.getVector(token);
		auto valB = evalB.getVector(token);
		valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
		return Symbol(valA);
	}
	case STRING:
		return Symbol(evalA.getString(token) + evalB.getString(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_ADD))
		{
			return o->getScope()->getVariable(Ruota::HASH_ADD, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `+` is undefined for value type", token);
	}
	return Symbol();
}

const std::string AddI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<AddI>(new AddI(";
	else
		ret = "new AddI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SubI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SubI::SubI(Instruction *a, Instruction *b, Token *token) : BinaryI(SUB, a, b, token) {}

const Symbol SubI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) - evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_SUB))
		{
			return o->getScope()->getVariable(Ruota::HASH_SUB, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `-` is undefined for value type", token);
	}
	return Symbol();
}

const std::string SubI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<SubI>(new SubI(";
	else
		ret = "new SubI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MulI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MulI::MulI(Instruction *a, Instruction *b, Token *token) : BinaryI(MUL, a, b, token) {}

const Symbol MulI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) * evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_MUL))
		{
			return o->getScope()->getVariable(Ruota::HASH_MUL, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `*` is undefined for value type", token);
	}
	return Symbol();
}

const std::string MulI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<MulI>(new MulI(";
	else
		ret = "new MulI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DivI::DivI(Instruction *a, Instruction *b, Token *token) : BinaryI(DIV, a, b, token) {}

const Symbol DivI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) / evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_DIV))
		{
			return o->getScope()->getVariable(Ruota::HASH_DIV, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `/` is undefined for value type", token);
	}
	return Symbol();
}

const std::string DivI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<DivI>(new DivI(";
	else
		ret = "new DivI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ModI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ModI::ModI(Instruction *a, Instruction *b, Token *token) : BinaryI(MOD, a, b, token) {}

const Symbol ModI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) % evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_MOD))
		{
			return o->getScope()->getVariable(Ruota::HASH_MOD, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `%` is undefined for value type", token);
	}
	return Symbol();
}

const std::string ModI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<ModI>(new ModI(";
	else
		ret = "new ModI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(Instruction *a, Instruction *b, Token *token) : BinaryI(POW_I, a, b, token) {}

const Symbol PowI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(NUMBER_POW(evalA.getNumber(token), evalB.getNumber(token)));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_POW))
		{
			return o->getScope()->getVariable(Ruota::HASH_POW, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `**` is undefined for value type", token);
	}
	return Symbol();
}

const std::string PowI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<PowI>(new PowI(";
	else
		ret = "new PowI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(Instruction *a, Instruction *b, Token *token) : BinaryI(LESS, a, b, token) {}

const Symbol LessI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(token) < evalB.getNumber(token);
	case STRING:
		return evalA.getString(token) < evalB.getString(token);
	case TYPE_NAME:
		return evalA.getTypeName(token) < evalB.getTypeName(token);
	case BOOLEAN_D:
		return evalA.getBool(token) < evalB.getBool(token);
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_LESS))
		{
			return o->getScope()->getVariable(Ruota::HASH_LESS, token).call(NIL, {evalB}, &evalA, token).getBool(token);
		}
		throwError("Operator `<` not defined for Object type", token);
	}
	default:
		return evalA.toString(token) < evalB.toString(token);
	}
}

const std::string LessI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<LessI>(new LessI(";
	else
		ret = "new LessI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(Instruction *a, Instruction *b, Token *token) : BinaryI(MORE, a, b, token) {}

const Symbol MoreI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(token) > evalB.getNumber(token);
	case STRING:
		return evalA.getString(token) > evalB.getString(token);
	case TYPE_NAME:
		return evalA.getTypeName(token) > evalB.getTypeName(token);
	case BOOLEAN_D:
		return evalA.getBool(token) > evalB.getBool(token);
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_MORE))
		{
			return o->getScope()->getVariable(Ruota::HASH_MORE, token).call(NIL, {evalB}, &evalA, token).getBool(token);
		}
		throwError("Operator `>` not defined for Object type", token);
	}
	default:
		return evalA.toString(token) > evalB.toString(token);
	}
}

const std::string MoreI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<MoreI>(new MoreI(";
	else
		ret = "new MoreI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(Instruction *a, Instruction *b, Token *token) : BinaryI(ELESS, a, b, token) {}

const Symbol ELessI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(token) <= evalB.getNumber(token);
	case STRING:
		return evalA.getString(token) <= evalB.getString(token);
	case TYPE_NAME:
		return evalA.getTypeName(token) <= evalB.getTypeName(token);
	case BOOLEAN_D:
		return evalA.getBool(token) <= evalB.getBool(token);
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_ELESS))
		{
			return o->getScope()->getVariable(Ruota::HASH_ELESS, token).call(NIL, {evalB}, &evalA, token).getBool(token);
		}
		throwError("Operator `<=` not defined for Object type", token);
	}
	default:
		return evalA.toString(token) <= evalB.toString(token);
	}
}

const std::string ELessI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<ELessI>(new ELessI(";
	else
		ret = "new ELessI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(Instruction *a, Instruction *b, Token *token) : BinaryI(EMORE, a, b, token) {}

const Symbol EMoreI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return evalA.getNumber(token) >= evalB.getNumber(token);
	case STRING:
		return evalA.getString(token) >= evalB.getString(token);
	case TYPE_NAME:
		return evalA.getTypeName(token) >= evalB.getTypeName(token);
	case BOOLEAN_D:
		return evalA.getBool(token) >= evalB.getBool(token);
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_EMORE))
		{
			return o->getScope()->getVariable(Ruota::HASH_EMORE, token).call(NIL, {evalB}, &evalA, token).getBool(token);
		}
		throwError("Operator `>=` not defined for Object type", token);
	}
	default:
		return evalA.toString(token) >= evalB.toString(token);
	}
}

const std::string EMoreI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<EMoreI>(new EMoreI(";
	else
		ret = "new EMoreI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Equals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Equals::Equals(Instruction *a, Instruction *b, Token *token) : BinaryI(EQUALS, a, b, token) {}

const Symbol Equals::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.equals(evalB, token));
}

const std::string Equals::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<Equals>(new Equals(";
	else
		ret = "new Equals(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NEquals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

NEquals::NEquals(Instruction *a, Instruction *b, Token *token) : BinaryI(NEQUALS, a, b, token) {}

const Symbol NEquals::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.nequals(evalB, token));
}

const std::string NEquals::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<NEquals>(new NEquals(";
	else
		ret = "new NEquals(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(Instruction *a, Instruction *b, Token *token) : BinaryI(AND, a, b, token) {}

const Symbol AndI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	if (!evalA.getBool(token))
		return Symbol(false);
	auto evalB = b->evaluate(scope);
	if (evalB.getBool(token))
		return Symbol(true);
	return Symbol(false);
}

const std::string AndI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<AndI>(new AndI(";
	else
		ret = "new AndI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class OrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

OrI::OrI(Instruction *a, Instruction *b, Token *token) : BinaryI(OR, a, b, token) {}

const Symbol OrI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	if (evalA.getBool(token))
		return Symbol(true);
	auto evalB = b->evaluate(scope);
	if (evalB.getBool(token))
		return Symbol(true);
	return Symbol(false);
}

const std::string OrI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<OrI>(new OrI(";
	else
		ret = "new OrI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BOrI::BOrI(Instruction *a, Instruction *b, Token *token) : BinaryI(B_OR, a, b, token) {}

const Symbol BOrI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) | evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_B_OR))
		{
			return o->getScope()->getVariable(Ruota::HASH_B_OR, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `|` is undefined for value type", token);
	}
	return Symbol();
}

const std::string BOrI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<BOrI>(new BOrI(";
	else
		ret = "new BOrI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BXOrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BXOrI::BXOrI(Instruction *a, Instruction *b, Token *token) : BinaryI(B_XOR, a, b, token) {}

const Symbol BXOrI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) ^ evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_B_XOR))
		{
			return o->getScope()->getVariable(Ruota::HASH_B_XOR, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `^` is undefined for value type", token);
	}
	return Symbol();
}

const std::string BXOrI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<BXOrI>(new BXOrI(";
	else
		ret = "new BXOrI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BAndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BAndI::BAndI(Instruction *a, Instruction *b, Token *token) : BinaryI(B_AND, a, b, token) {}

const Symbol BAndI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) & evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_B_AND))
		{
			return o->getScope()->getVariable(Ruota::HASH_B_AND, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `&` is undefined for value type", token);
	}
	return Symbol();
}

const std::string BAndI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<BAndI>(new BAndI(";
	else
		ret = "new BAndI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftLeft                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftLeft::BShiftLeft(Instruction *a, Instruction *b, Token *token) : BinaryI(B_SH_L, a, b, token) {}

const Symbol BShiftLeft::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) << evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_B_SH_L))
		{
			return o->getScope()->getVariable(Ruota::HASH_B_SH_L, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `<<` is undefined for value type", token);
	}
	return Symbol();
}

const std::string BShiftLeft::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<BShiftLeft>(new BShiftLeft(";
	else
		ret = "new BShiftLeft(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BShiftRight                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BShiftRight::BShiftRight(Instruction *a, Instruction *b, Token *token) : BinaryI(B_SH_R, a, b, token) {}

const Symbol BShiftRight::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(evalA.getNumber(token) >> evalB.getNumber(token));
	case OBJECT:
	{
		auto o = evalA.getObject(token);
		if (o->hasValue(Ruota::HASH_B_SH_R))
		{
			return o->getScope()->getVariable(Ruota::HASH_B_SH_R, token).call(NIL, {evalB}, &evalA, token);
		}
	}
	default:
		throwError("Operator `>>` is undefined for value type", token);
	}
	return Symbol();
}

const std::string BShiftRight::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<BShiftRight>(new BShiftRight(";
	else
		ret = "new BShiftRight(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(Instruction *a, Instruction *b, Token *token) : BinaryI(SET, a, b, token) {}

const Symbol SetI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	evalA.set(evalB, token);
	return evalA;
}

const std::string SetI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<SetI>(new SetI(";
	else
		ret = "new SetI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReturnI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ReturnI::ReturnI(Instruction *a, Token *token) : UnaryI(RETURN, a, token) {}

const Symbol ReturnI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	evalA.setSymbolType(ID_RETURN);
	return evalA;
}

const std::string ReturnI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<Returni>(new Returni(";
	else
		ret = "new ReturnI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(const std::string &id, Instruction *a, Token *token) : UnaryI(EXTERN, a, token), id(id)
{
	if (rlib::loaded.find(id) != rlib::loaded.end())
		this->f = rlib::loaded.at(id);
	else
		throwError("External function `" + id + "` is not defined", token);
}

const Symbol ExternI::evaluate(Scope * scope) const
{
	return f(a->evaluate(scope).getVector(token), token);
}

const std::string ExternI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<ExternI>(new ExternI(";
	else
		ret = "new ExternI(";
	ret += "\"" + id + "\", " + a->toString(false);

	return ret + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(Instruction *a, Token *token) : UnaryI(LENGTH, a, token) {}

const Symbol LengthI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case STRING:
	{
		std::string str = evalA.getString(token);
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
				return Symbol(NUMBER_NEW_LONG(evalA.getString(token).size()));
		}
		return Symbol(NUMBER_NEW_LONG(q));
	}
	case DICTIONARY:
		return Symbol(NUMBER_NEW_LONG(evalA.dictionarySize(token)));
	case VECTOR:
		return Symbol(NUMBER_NEW_LONG(evalA.vectorSize()));
	default:
		throwError("Cannot get length of value", token);
	}
	return Symbol();
}

const std::string LengthI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<LengthI>(new LengthI(";
	else
		ret = "new LengthI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SizeI::SizeI(Instruction *a, Token *token) : UnaryI(SIZE_I, a, token) {}

const Symbol SizeI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case STRING:
		return Symbol(NUMBER_NEW_LONG(evalA.getString(token).size()));
	case DICTIONARY:
		return Symbol(NUMBER_NEW_LONG(evalA.dictionarySize(token)));
	case VECTOR:
		return Symbol(NUMBER_NEW_LONG(evalA.vectorSize()));
	default:
		throwError("Cannot get length of value", token);
	}
	return Symbol();
}

const std::string SizeI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<SizeI>(new SizeI(";
	else
		ret = "new SizeI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ClassI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ClassI::ClassI(hashcode_t key, OBJECT_TYPE type, std::shared_ptr<Instruction> body, Instruction *extends, Token *token) : Instruction(CLASS_I, token), key(key), type(type), body(body), extends(extends) {}

const Symbol ClassI::evaluate(Scope * scope) const
{
	std::shared_ptr<Instruction> nbody = body;
	if (extends != NULL)
	{
		auto e = extends->evaluate(scope);
		auto eo = e.getObject(token);
		if (eo->getType() == STATIC_O)
			throwError("Cannot extend a statically declared Object", token);
		auto eb = eo->getBody().get();
		std::vector<Instruction *> temp = {body.get(), eb};
		nbody = std::make_shared<ScopeI>(temp, token);
	}
	std::shared_ptr<Object> o = std::make_shared<Object>(scope, type, nbody, hash.deHash(key));
	if (type == STATIC_O)
		body->evaluate(o->getScope());
	auto d = Symbol(o);
	scope->createVariable(key, d, token);
	return d;
}

const std::string ClassI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<ClassI>(new ClassI(";
	else
		ret = "new ClassI(";
	ret += std::to_string(key) + ", static_cast<OBJECT_TYPE>(" + std::to_string(type) + "), " + body->toString(true) + ", " + extends->toString(false);
	return ret + ")" + (shared ? ")" : "");
}

ClassI::~ClassI()
{
	if (extends != NULL)
		delete extends;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NewI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

NewI::NewI(Instruction *a, Instruction *b, Token *token) : BinaryI(NEW_I, a, b, token) {}

const Symbol NewI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	auto base = evalA.getObject(token);
	return base->instantiate(evalB.getVector(token), token);
}

const std::string NewI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<NewI>(new NewI(";
	else
		ret = "new NewI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastToI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

CastToI::CastToI(Instruction *a, D_TYPE convert, Token *token) : UnaryI(CAST_TO_I, a, token), convert(convert) {}

const Symbol CastToI::evaluate(Scope * scope) const
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
			return Symbol(evalA.toString(token));
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
				return Symbol(NUMBER_NEW_DOUBLE(std::stold(evalA.getString(token))));
			}
			catch (const std::invalid_argument &e)
			{
				throwError("String `" + evalA.getString(token) + "` cannot be converted to Number", token);
			}
		case OBJECT:
		{
			auto o = evalA.getObject(token);
			if (o->hasValue(Ruota::HASH_TO_NUMBER))
			{
				return o->getScope()->getVariable(Ruota::HASH_TO_NUMBER, token).call(NIL, {}, &evalA, token);
			}
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
			return Symbol(NUMBER_GET_LONG(evalA.getNumber(token)) != 0);
		case STRING:
			return Symbol(evalA.getString(token) == "true");
		case OBJECT:
		{
			auto o = evalA.getObject(token);
			if (o->hasValue(Ruota::HASH_TO_BOOLEAN))
			{
				return o->getScope()->getVariable(Ruota::HASH_TO_BOOLEAN, token).call(NIL, {}, &evalA, token);
			}
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
			auto v = evalA.getVector(token);
			std::map<hashcode_t, Symbol> nd;
			for (size_t i = 0; i < v.size(); i++)
			{
				nd[hash.hashString(std::to_string(i))] = v[i];
			}
			return Symbol(nd);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(token);
			if (o->hasValue(Ruota::HASH_TO_DICTIONARY))
			{
				return o->getScope()->getVariable(Ruota::HASH_TO_DICTIONARY, token).call(NIL, {}, &evalA, token);
			}
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
			auto dict = evalA.getDictionary(token);
			std::vector<Symbol> nv;
			for (auto &e : dict)
			{
				nv.push_back(Symbol({{Ruota::HASH_KEY, Symbol(hash.deHash(e.first))}, {Ruota::HASH_VALUE, e.second}}));
			}
			return Symbol(nv);
		}
		case OBJECT:
		{
			auto o = evalA.getObject(token);
			if (o->hasValue(Ruota::HASH_TO_VECTOR))
			{
				return o->getScope()->getVariable(Ruota::HASH_TO_VECTOR, token).call(NIL, {}, &evalA, token);
			}
			break;
		}
		case STRING:
		{
			std::string str = evalA.getString(token);
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
			return Symbol(static_cast<signed long long>(hash.hashString(evalA.getObject(token)->getName())));
	}
	case NIL:
		return Symbol();
	default:
		break;
	}
	throwError("Cannot convert between given types", token);
	return Symbol();
}

const std::string CastToI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<CastToI>(new CastToI(";
	else
		ret = "new CastToI(";
	return ret + a->toString(false) + ", static_cast<D_TYPE>(" + std::to_string(convert) + "))" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AllocI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

AllocI::AllocI(Instruction *a, Token *token) : UnaryI(ALLOC_I, a, token) {}

const Symbol AllocI::evaluate(Scope * scope) const
{
	auto evalA = NUMBER_GET_LONG(a->evaluate(scope).getNumber(token));
	if (evalA < 0)
		throwError("Cannot initialize a Vector with size < 0", token);
	std::vector<Symbol> v(evalA);
	return Symbol(v);
}

const std::string AllocI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<AllocI>(new AllocI(";
	else
		ret = "new AllocI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(Instruction *a, Instruction *b, Token *token) : BinaryI(UNTIL_I, a, b, token) {}

const Symbol UntilI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope).getNumber(token);
	auto evalB = b->evaluate(scope).getNumber(token);
	std::vector<Symbol> nv;
	for (auto i = evalA; i < evalB; i += 1)
		nv.push_back(Symbol(i));
	return Symbol(nv);
}

const std::string UntilI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<UntilI>(new UntilI(";
	else
		ret = "new UntilI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ScopeI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ScopeI::ScopeI(std::vector<Instruction *> children, Token *token) : Instruction(SCOPE_I, token), children(children) {}

const Symbol ScopeI::evaluate(Scope * scope) const
{
	for (auto &e : children)
	{
		auto eval = e->evaluate(scope);
		if (eval.getSymbolType() != ID_CASUAL)
			return eval;
	}
	return Symbol();
}

const std::string ScopeI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<ScopeI>(new ScopeI({";
	else
		ret = "new ScopeI({";

	unsigned long i = 0;
	for (auto &e : children)
	{
		if (i > 0)
			ret += ", ";
		ret += e->toString(false);
		i++;
	}
	return ret + "})" + (shared ? ")" : "");
}

ScopeI::~ScopeI()
{
	for (auto &e : children)
		delete e;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MapI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

MapI::MapI(std::map<hashcode_t, Instruction *> children, Token *token) : Instruction(MAP_I, token), children(children) {}

const Symbol MapI::evaluate(Scope * scope) const
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

const std::string MapI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<MapI>(new MapI({";
	else
		ret = "new MapI({";

	unsigned long i = 0;
	for (auto &e : children)
	{
		if (i > 0)
			ret += ", ";
		ret += "{" + std::to_string(e.first) + ", " + e.second->toString(false) + "}";
		i++;
	}
	return ret + "})" + (shared ? ")" : "");
}

MapI::~MapI()
{
	for (auto &e : children)
		delete e.second;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReferI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ReferI::ReferI(Instruction *a, Token *token) : UnaryI(REFER_I, a, token) {}

const Symbol ReferI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	evalA.setSymbolType(ID_REFER);
	return evalA;
}

const std::string ReferI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<ReferI>(new ReferI(";
	else
		ret = "new ReferI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SwitchI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SwitchI::SwitchI(Instruction *switchs, std::map<Symbol, Instruction *> cases, Instruction *elses, Token *token) : Instruction(SWITCH_I, token), switchs(switchs), cases(cases), elses(elses) {}

const Symbol SwitchI::evaluate(Scope * scope) const
{
	auto eval = switchs->evaluate(scope);
	if (cases.find(eval) != cases.end())
	{
		Scope newScope(scope, "");
		return cases.at(eval)->evaluate(&newScope);
	}
	else if (elses != NULL)
	{
		Scope newScope(scope, "");
		return elses->evaluate(&newScope);
	}
	return Symbol();
}

const std::string SwitchI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<SwitchI>(new SwitchI({";
	else
		ret = "new SwitchI({";

	unsigned long i = 0;
	for (auto &e : cases)
	{
		if (i > 0)
			ret += ", ";
		ret += "{" + e.first.toCodeString() + ", " + e.second->toString(false) + "}";
		i++;
	}
	return ret + "})" + (shared ? ")" : "");
}

SwitchI::~SwitchI()
{
	for (auto &e : cases)
		delete e.second;
	cases.clear();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TryCatchI                                                                                        */
/*-------------------------------------------------------------------------------------------------------*/

TryCatchI::TryCatchI(Instruction *a, Instruction *b, hashcode_t key, Token *token) : BinaryI(TRY_CATCH_I, a, b, token), key(key) {}

const Symbol TryCatchI::evaluate(Scope * scope) const
{
	try
	{
		Scope newScope(scope, "");
		return a->evaluate(&newScope);
	}
	catch (const std::runtime_error &e)
	{
		Scope newScope(scope, "");
		newScope.createVariable(key, Symbol(std::string(e.what())), token);
		return b->evaluate(&newScope);
	}
}

const std::string TryCatchI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<TryCatchI>(new TryCatchI(";
	else
		ret = "new TryCatchI(";
	return ret + a->toString(false) + ", " + b->toString(false) + ", " + std::to_string(key) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ThrowI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ThrowI::ThrowI(Instruction *a, Token *token) : UnaryI(THROW_I, a, token) {}

const Symbol ThrowI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	throwError(evalA.getString(token), token);
	return Symbol();
}

const std::string ThrowI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<TryCatchI>(new ThrowI(";
	else
		ret = "new ThrowI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureEquals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureEquals::PureEquals(Instruction *a, Instruction *b, Token *token) : BinaryI(PURE_EQUALS, a, b, token) {}

const Symbol PureEquals::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.pureEquals(evalB, token));
}

const std::string PureEquals::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<Equals>(new PureEquals(";
	else
		ret = "new PureEquals(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PureNEquals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

PureNEquals::PureNEquals(Instruction *a, Instruction *b, Token *token) : BinaryI(PURE_NEQUALS, a, b, token) {}

const Symbol PureNEquals::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA.pureNEquals(evalB, token));
}

const std::string PureNEquals::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<NEquals>(new PureNEquals(";
	else
		ret = "new PureNEquals(";
	return ret + a->toString(false) + ", " + b->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharNI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharNI::CharNI(Instruction *a, Token *token) : UnaryI(CHARN_I, a, token) {}

const Symbol CharNI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope).getString(token);
	std::vector<Symbol> nv;
	for (const unsigned char &c : evalA)
		nv.push_back(Symbol(NUMBER_NEW_LONG(c)));
	return Symbol(nv);
}

const std::string CharNI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<CharNI>(new CharNI(";
	else
		ret = "new CharNI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CharSI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

CharSI::CharSI(Instruction *a, Token *token) : UnaryI(CHARS_I, a, token) {}

const Symbol CharSI::evaluate(Scope * scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case NUMBER:
		return Symbol(std::string(1, static_cast<char>(NUMBER_GET_LONG(evalA.getNumber(token)))));
	case VECTOR:
	{
		std::string ret = "";
		auto v = evalA.getVector(token);
		for (auto &e : v)
			ret.push_back(static_cast<char>(NUMBER_GET_LONG(e.getNumber(token))));
		return Symbol(ret);
	}
	default:
		throwError("Cannot convert value(s) into String", token);
	}
	return Symbol();
}

const std::string CharSI::toString(bool shared) const
{
	std::string ret;
	if (shared)
		ret = "std::shared_ptr<CharSI>(new CharSI(";
	else
		ret = "new CharSI(";
	return ret + a->toString(false) + ")" + (shared ? ")" : "");
}