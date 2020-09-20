#include "Ruota.h"

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(I_TYPE type) : type(type) {}

const I_TYPE Instruction::getType()
{
	return type;
}

Instruction::~Instruction() {}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(I_TYPE type, Instruction *a) : Instruction(type), a(a) {}

Instruction *UnaryI::getA()
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

CastingI::CastingI(I_TYPE type, const std::string &key) : Instruction(type), key(key) {}

const std::string CastingI::getKey()
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(I_TYPE type, Instruction *a, Instruction *b) : UnaryI(type, a), b(b) {}

Instruction *BinaryI::getB()
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

Container::Container(const SYM &d) : Instruction(CONTAINER), d(d) {}

SYM Container::evaluate(Scope &) const
{
	return d;
}

const std::string Container::toString() const
{
	return manager::toString(d);
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(const std::string &key, std::vector<std::string> fargs, std::shared_ptr<Instruction> body) : Instruction(DEFINE), key(key), fargs(fargs), body(body) {}

SYM DefineI::evaluate(Scope &scope) const
{
	auto f = std::make_shared<Function>(scope, fargs, body);
	if (key != "")
	{
		auto d = manager::newValue(f);
		scope.createVariable(key, d);
		return d;
	}

	return manager::newValue(f);
}

const std::string DefineI::toString() const
{
	if (key != "")
		return "define(" + key + ", " + std::to_string(fargs.size()) + ", " + body->toString() + ")";
	else
		return "lambda(" + std::to_string(fargs.size()) + ", " + body->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Sequence                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Sequence::Sequence(bool scoped, std::vector<Instruction *> children) : scoped(scoped), Instruction(SEQUENCE), children(children) {}

SYM Sequence::evaluate(Scope &scope) const
{
	if (scoped)
	{
		Scope newScope(scope, "");
		for (auto &e : children)
		{
			auto temp = e->evaluate(newScope);
			if (temp.type == ID_RETURN)
				return temp;
		}
		return manager::newValue();
	}

	std::vector<SYM> evals;
	for (auto &e : children)
	{
		if (e->getType() == UNTIL_I)
		{
			auto eval = e->evaluate(scope);
			auto v = manager::getVector(eval);
			evals.insert(evals.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
		}
		else
		{
			auto eval = e->evaluate(scope);
			evals.push_back(eval);
		}
	}
	return manager::newValue(evals);
}

void Sequence::setScoped(bool scoped)
{
	this->scoped = scoped;
}

const std::string Sequence::toString() const
{
	std::string ret;
	if (scoped)
		ret = "scope(";
	else
		ret = "seq(";

	unsigned long i = 0;
	for (auto &e : children)
	{
		if (i > 0)
			ret += ", ";
		ret += e->toString();
		i++;
	}
	return ret + ")";
}

Sequence::~Sequence()
{
	for (auto &e : children)
		delete e;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IFElseI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IfElseI::IfElseI(Instruction *ifs, Instruction *body, Instruction *elses) : Instruction(IFELSE), ifs(ifs), body(body), elses(elses) {}

SYM IfElseI::evaluate(Scope &scope) const
{
	Scope newScope(scope, "");
	auto evalIf = ifs->evaluate(newScope);
	if (manager::getBool(evalIf))
	{
		auto temp = body->evaluate(newScope);
		if (temp.type == ID_RETURN || temp.type == ID_BREAK)
			return temp;
	}
	else if (elses)
	{
		auto temp = elses->evaluate(newScope);
		if (temp.type == ID_RETURN || temp.type == ID_BREAK)
			return temp;
	}
	return manager::newValue();
}

const std::string IfElseI::toString() const
{
	std::string ret = "ifelse(";
	ret += ifs->toString() + ", " + body->toString();
	if (elses)
		ret += ", " + elses->toString();
	return ret + ")";
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

WhileI::WhileI(Instruction *whiles, Instruction *body) : Instruction(WHILE), whiles(whiles), body(body) {}

SYM WhileI::evaluate(Scope &scope) const
{
	while (manager::getBool(whiles->evaluate(scope)))
	{
		Scope newScope(scope, "");
		auto temp = body->evaluate(newScope);
		if (temp.type == ID_RETURN)
			return temp;
		if (temp.type == ID_BREAK)
			break;
	}
	return manager::newValue();
}

const std::string WhileI::toString() const
{
	std::string ret = "while(";
	ret += whiles->toString() + ", " + body->toString();
	return ret + ")";
}

WhileI::~WhileI()
{
	delete whiles;
	delete body;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ForI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ForI::ForI(const std::string &id, Instruction *fors, Instruction *body) : Instruction(FOR), id(id), fors(fors), body(body) {}

SYM ForI::evaluate(Scope &scope) const
{
	auto evalFor = fors->evaluate(scope);
	for (size_t i = 0; i < manager::vectorSize(evalFor); i++)
	{
		Scope newScope(scope, "");
		newScope.createVariable(id, manager::indexVector(evalFor, i));
		auto temp = body->evaluate(newScope);
		if (temp.type == ID_RETURN)
			return temp;
		if (temp.type == ID_BREAK)
			break;
	}
	return manager::newValue();
}

const std::string ForI::toString() const
{
	std::string ret = "for(" + id;
	ret += fors->toString() + ", " + body->toString();
	return ret + ")";
}

ForI::~ForI()
{
	delete fors;
	delete body;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class VariableI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

VariableI::VariableI(const std::string &key) : CastingI(VARIABLE, key) {}

SYM VariableI::evaluate(Scope &scope) const
{
	auto d = scope.getVariable(key);
	return d;
}

const std::string VariableI::toString() const
{
	return "fetch(\"" + key + "\")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DeclareI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DeclareI::DeclareI(const std::string &key) : CastingI(DECLARE, key) {}

SYM DeclareI::evaluate(Scope &scope) const
{
	auto d = scope.createVariable(key);
	return d;
}

const std::string DeclareI::toString() const
{
	return "var(\"" + key + "\")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class IndexI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

IndexI::IndexI(Instruction *a, Instruction *b) : BinaryI(INDEX, a, b) {}

SYM IndexI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::indexVector(evalA, (unsigned long)manager::getNumber(evalB));
}

const std::string IndexI::toString() const
{
	return "index(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(Instruction *a, Instruction *b) : BinaryI(INNER, a, b) {}

SYM InnerI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (manager::getType(evalA))
	{
	case DICTIONARY:
		//TODO
		return manager::newValue();
	case OBJECT:
	{
		auto o = manager::getObject(evalA);
		if (o->getType() != STATIC_O)
			throw std::runtime_error("Cannot index a non-static, non-instantiated Object");
		return b->evaluate(*o->getScope());
	}
	default:
		throw std::runtime_error("Cannot enter value");
	}
}

const std::string InnerI::toString() const
{
	return "inner(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CallI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

CallI::CallI(Instruction *a, Instruction *b) : BinaryI(INDEX, a, b) {}

SYM CallI::evaluate(Scope &scope) const
{
	auto args = manager::getVector(b->evaluate(scope));
	switch (a->getType())
	{
	case INNER:
	{
		auto evalA = ((InnerI *)a)->getA()->evaluate(scope);
		if (manager::getType(evalA) == OBJECT)
		{
			auto bb = ((InnerI *)a)->getB();
			if (bb->getType() == VARIABLE && manager::getObject(evalA)->hasValue(((VariableI *)bb)->getKey()))
			{
				auto evalB = ((InnerI *)a)->getB()->evaluate(*manager::getObject(evalA)->getScope());
				return manager::call(evalB, args, evalA);
			}
		}

		auto evalB = ((InnerI *)a)->getB()->evaluate(scope);
		std::vector<SYM> params = {evalA};
		params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

		return manager::call(evalB, params);
	}
	default:
	{
		auto evalA = a->evaluate(scope);
		return manager::call(evalA, args);
	}
	}
}

const std::string CallI::toString() const
{
	return "call(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AddI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AddI::AddI(Instruction *a, Instruction *b) : BinaryI(ADD, a, b) {}

SYM AddI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::add(evalA, evalB);
}

const std::string AddI::toString() const
{
	return "add(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SubI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SubI::SubI(Instruction *a, Instruction *b) : BinaryI(SUB, a, b) {}

SYM SubI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::sub(evalA, evalB);
}

const std::string SubI::toString() const
{
	return "sub(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MulI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MulI::MulI(Instruction *a, Instruction *b) : BinaryI(MUL, a, b) {}

SYM MulI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::mul(evalA, evalB);
}

const std::string MulI::toString() const
{
	return "mul(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DivI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DivI::DivI(Instruction *a, Instruction *b) : BinaryI(DIV, a, b) {}

SYM DivI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::div(evalA, evalB);
}

const std::string DivI::toString() const
{
	return "div(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ModI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ModI::ModI(Instruction *a, Instruction *b) : BinaryI(MOD, a, b) {}

SYM ModI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::mod(evalA, evalB);
}

const std::string ModI::toString() const
{
	return "mod(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(Instruction *a, Instruction *b) : BinaryI(POW_I, a, b) {}

SYM PowI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::pow(evalA, evalB);
}

const std::string PowI::toString() const
{
	return "pow(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(Instruction *a, Instruction *b) : BinaryI(LESS, a, b) {}

SYM LessI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::less(evalA, evalB);
}

const std::string LessI::toString() const
{
	return "less(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(Instruction *a, Instruction *b) : BinaryI(MORE, a, b) {}

SYM MoreI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::more(evalA, evalB);
}

const std::string MoreI::toString() const
{
	return "more(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(Instruction *a, Instruction *b) : BinaryI(ELESS, a, b) {}

SYM ELessI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::eless(evalA, evalB);
}

const std::string ELessI::toString() const
{
	return "eless(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(Instruction *a, Instruction *b) : BinaryI(EMORE, a, b) {}

SYM EMoreI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::emore(evalA, evalB);
}

const std::string EMoreI::toString() const
{
	return "emore(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class Equals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Equals::Equals(Instruction *a, Instruction *b) : BinaryI(EQUALS, a, b) {}

SYM Equals::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::newValue(manager::equals(evalA, evalB));
}

const std::string Equals::toString() const
{
	return "equals(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NEquals                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

NEquals::NEquals(Instruction *a, Instruction *b) : BinaryI(NEQUALS, a, b) {}

SYM NEquals::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::newValue(manager::nequals(evalA, evalB));
}

const std::string NEquals::toString() const
{
	return "nequals(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(Instruction *a, Instruction *b) : BinaryI(AND, a, b) {}

SYM AndI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::newValue(manager::dand(evalA, evalB));
}

const std::string AndI::toString() const
{
	return "and(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class OrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

OrI::OrI(Instruction *a, Instruction *b) : BinaryI(OR, a, b) {}

SYM OrI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return manager::newValue(manager::dor(evalA, evalB));
}

const std::string OrI::toString() const
{
	return "or(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(Instruction *a, Instruction *b) : BinaryI(SET, a, b) {}

SYM SetI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	manager::set(evalA, evalB);
	return evalA;
}

const std::string SetI::toString() const
{
	return "set(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ReturnI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ReturnI::ReturnI(Instruction *a) : UnaryI(RETURN, a) {}

SYM ReturnI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	evalA.type = ID_RETURN;
	return evalA;
}

const std::string ReturnI::toString() const
{
	return "return(" + a->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(boost::function<SYM(std::vector<SYM>)> f, Instruction *a) : UnaryI(EXTERN, a), f(f) {}

SYM ExternI::evaluate(Scope &scope) const
{
	return f(manager::getVector(a->evaluate(scope)));
}

const std::string ExternI::toString() const
{
	return "extern_call()";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(Instruction *a) : UnaryI(LENGTH, a) {}

SYM LengthI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (manager::getType(evalA))
	{
	case STRING:
	{
		std::string str = manager::getString(evalA);
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
				return manager::newValue((long_double_t)manager::getString(evalA).size());
		}
		return manager::newValue((long_double_t)q);
	}
	case VECTOR:
		return manager::newValue((long_double_t)manager::getVector(evalA).size());
	default:
		throw std::runtime_error("Cannot get length of value");
	}
}

const std::string LengthI::toString() const
{
	return "length(" + a->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

SizeI::SizeI(Instruction *a) : UnaryI(SIZE_I, a) {}

SYM SizeI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (manager::getType(evalA))
	{
	case STRING:
		return manager::newValue((long_double_t)manager::getString(evalA).size());
	case VECTOR:
		return manager::newValue((long_double_t)manager::getVector(evalA).size());
	default:
		throw std::runtime_error("Cannot get length of value");
	}
}

const std::string SizeI::toString() const
{
	return "size(" + a->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ClassI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

ClassI::ClassI(const std::string &key, OBJECT_TYPE type, std::shared_ptr<Instruction> body) : Instruction(CLASS_I), key(key), type(type), body(body) {}

SYM ClassI::evaluate(Scope &scope) const
{
	std::shared_ptr<Object> o = std::make_shared<Object>(scope, type, body, key);
	if (type == STATIC_O)
		body->evaluate(*o->getScope());
	auto d = manager::newValue(o);
	scope.createVariable(key, d);
	return d;
}

const std::string ClassI::toString() const
{
	return "object(" + key + ", " + body->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class NewI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

NewI::NewI(Instruction *a, Instruction *b) : BinaryI(NEW_I, a, b) {}

SYM NewI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	auto base = manager::getObject(evalA);
	return base->instantiate(manager::getVector(evalB));
}

const std::string NewI::toString() const
{
	return "new(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TypeI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

TypeI::TypeI(Instruction *a) : UnaryI(TYPE_I, a) {}

SYM TypeI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	return manager::newValue(manager::getTypeString(evalA));
}

const std::string TypeI::toString() const
{
	return "type(" + a->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class CastToI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

CastToI::CastToI(Instruction *a, D_TYPE convert) : UnaryI(CAST_TO_I, a), convert(convert) {}

SYM CastToI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (convert)
	{
	case STRING:
		switch (manager::getType(evalA))
		{
		case STRING:
			return evalA;
		default:
			return manager::newValue(manager::toString(evalA));
		}
	case NUMBER:
		switch (manager::getType(evalA))
		{
		case NUMBER:
			return evalA;
		case NIL:
			return manager::newValue((long_double_t)0.0);
		case STRING:
			return manager::newValue(std::stold(manager::getString(evalA)));
		default:
			break;
		}
	case BOOLEAN_D:
		switch (manager::getType(evalA))
		{
		case BOOLEAN_D:
			return evalA;
		case NIL:
			return manager::newValue(false);
		case NUMBER:
			return manager::newValue(manager::getNumber(evalA) != 0);
		case STRING:
			return manager::newValue(manager::getString(evalA) == "true");
		default:
			break;
		}
	case VECTOR:
		switch (manager::getType(evalA))
		{
		case VECTOR:
			return evalA;
		case STRING:
		{
			std::string str = manager::getString(evalA);
			std::vector<SYM> nv;
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
						nv.push_back(manager::newValue(std::string(1, str[i])));
					return manager::newValue(nv);
				}
				nv.push_back(manager::newValue(str.substr(last, s)));
				last = i + 1;
			}
			return manager::newValue(nv);
		}
		default:
			break;
		}
	case NIL:
		return manager::newValue();
	default:
		break;
	}
	throw std::runtime_error("Cannot convert between given types");
}

const std::string CastToI::toString() const
{
	return "cast(" + a->toString() + ", " + std::to_string(convert) + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AllocI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

AllocI::AllocI(Instruction *a) : UnaryI(ALLOC_I, a) {}

SYM AllocI::evaluate(Scope &scope) const
{
	auto evalA = manager::getNumber(a->evaluate(scope));
	std::vector<SYM> v(evalA);
	return manager::newValue(v);
}

const std::string AllocI::toString() const
{
	return "alloc(" + a->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class UntilI                                                                                           */
/*-------------------------------------------------------------------------------------------------------*/

UntilI::UntilI(Instruction *a, Instruction *b) : BinaryI(UNTIL_I, a, b) {}

SYM UntilI::evaluate(Scope &scope) const
{
	auto evalA = manager::getNumber(a->evaluate(scope));
	auto evalB = manager::getNumber(b->evaluate(scope));
	std::vector<SYM> nv;
	for (auto i = evalA; i < evalB; i++)
		nv.push_back(manager::newValue(i));
	return manager::newValue(nv);
}

const std::string UntilI::toString() const
{
	return "until(" + a->toString() + ", " + b->toString() + ")";
}