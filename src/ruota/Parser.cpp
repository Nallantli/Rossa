#include "Ruota.hpp"

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
		Scope newScope(scope);
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
		evals.push_back(e->evaluate(scope));
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
	Scope newScope(scope);
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
		Scope newScope(scope);
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
	for (int i = 0; i < manager::vectorSize(evalFor); i++)
	{
		Scope newScope(scope);
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
	return manager::indexVector(evalA, manager::getNumber(evalB));
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
		//TODO
		return manager::newValue();
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
	return manager::mul(evalA, evalB);
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