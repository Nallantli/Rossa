#include "Ruota.h"

/*-------------------------------------------------------------------------------------------------------*/
/*class Instruction                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

Instruction::Instruction(I_TYPE type) : type(type) {}

I_TYPE Instruction::getType() const
{
	return type;
}

Instruction::~Instruction() {}

/*-------------------------------------------------------------------------------------------------------*/
/*class UnaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

UnaryI::UnaryI(I_TYPE type, Instruction *a) : Instruction(type), a(a) {}

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

CastingI::CastingI(I_TYPE type, hashcode_t key) : Instruction(type), key(key) {}

hashcode_t CastingI::getKey() const
{
	return key;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class BinaryI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

BinaryI::BinaryI(I_TYPE type, Instruction *a, Instruction *b) : UnaryI(type, a), b(b) {}

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

Container::Container(Symbol d) : Instruction(CONTAINER), d(d) {}

Symbol Container::evaluate(Scope &) const
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

DefineI::DefineI(hashcode_t key, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> params, std::shared_ptr<Instruction> body) : Instruction(DEFINE), key(key), ftype(ftype), params(params), body(body) {}

Symbol DefineI::evaluate(Scope &scope) const
{
	auto f = std::make_shared<Function>(scope, params, body);
	if (key > 0)
	{
		auto d = Symbol(ftype, f);
		scope.createVariable(key, d);
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

Sequence::Sequence(std::vector<Instruction *> children) : Instruction(SEQUENCE), children(children) {}

Symbol Sequence::evaluate(Scope &scope) const
{
	std::vector<Symbol> evals;
	for (auto &e : children)
	{
		if (e->getType() == UNTIL_I)
		{
			auto eval = e->evaluate(scope);
			auto v = eval.getVector();
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

IfElseI::IfElseI(Instruction *ifs, Instruction *body, Instruction *elses) : Instruction(IFELSE), ifs(ifs), body(body), elses(elses) {}

Symbol IfElseI::evaluate(Scope &scope) const
{
	Scope newScope(scope, "");
	auto evalIf = ifs->evaluate(newScope);
	if (evalIf.getBool())
	{
		return body->evaluate(newScope);
	}
	else if (elses)
	{
		return elses->evaluate(newScope);
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

WhileI::WhileI(Instruction *whiles, Instruction *body) : Instruction(WHILE), whiles(whiles), body(body) {}

Symbol WhileI::evaluate(Scope &scope) const
{
	while (whiles->evaluate(scope).getBool())
	{
		Scope newScope(scope, "");
		auto temp = body->evaluate(newScope);
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

ForI::ForI(hashcode_t id, Instruction *fors, Instruction *body) : Instruction(FOR), id(id), fors(fors), body(body) {}

Symbol ForI::evaluate(Scope &scope) const
{
	auto evalFor = fors->evaluate(scope);
	for (size_t i = 0; i < evalFor.vectorSize(); i++)
	{
		Scope newScope(scope, "");
		newScope.createVariable(id, evalFor.indexVector(i));
		auto temp = body->evaluate(newScope);
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

VariableI::VariableI(hashcode_t key) : CastingI(VARIABLE, key) {}

Symbol VariableI::evaluate(Scope &scope) const
{
	auto d = scope.getVariable(key);
	return d;
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

DeclareI::DeclareI(hashcode_t key) : CastingI(DECLARE, key) {}

Symbol DeclareI::evaluate(Scope &scope) const
{
	auto d = scope.createVariable(key);
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

IndexI::IndexI(Instruction *a, Instruction *b) : BinaryI(INDEX, a, b) {}

Symbol IndexI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getValueType())
	{
	case VECTOR:
		return evalA.indexVector(NUMBER_GET_LONG(evalB.getNumber()));
		break;
	case DICTIONARY:
		return evalA.indexDictionary(hash.hashString(evalB.getString()));
		break;
	default:
		throw std::runtime_error("Cannot index value");
	}
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

InnerI::InnerI(Instruction *a, Instruction *b) : BinaryI(INNER, a, b) {}

Symbol InnerI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case DICTIONARY:
		if (b->getType() == VARIABLE)
			return evalA.indexDictionary(((VariableI *)b)->getKey());
		throw std::runtime_error("Cannot enter Dictionary with given value");
	case OBJECT:
	{
		auto o = evalA.getObject();
		if (o->getType() != STATIC_O)
			throw std::runtime_error("Cannot index a non-static, non-instantiated Object");
		return b->evaluate(*o->getScope());
	}
	default:
		throw std::runtime_error("Cannot enter value");
	}
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

CallI::CallI(Instruction *a, Instruction *b) : BinaryI(INDEX, a, b) {}

Symbol CallI::evaluate(Scope &scope) const
{
	auto args = b->evaluate(scope).getVector();
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
			if (bb->getType() == VARIABLE && evalA.getObject()->hasValue(((VariableI *)bb)->getKey()))
			{
				auto evalB = ((InnerI *)a)->getB()->evaluate(*evalA.getObject()->getScope());
				return evalB.call(NIL, args, evalA);
			}
		}
		case DICTIONARY:
		{
			auto bb = ((InnerI *)a)->getB();
			if (bb->getType() == VARIABLE && evalA.hasDictionaryKey(((VariableI *)bb)->getKey()))
			{
				auto evalB = a->evaluate(scope);
				return evalB.call(NIL, args, evalA);
			}
		}
		}

		auto evalB = ((InnerI *)a)->getB()->evaluate(scope);
		std::vector<Symbol> params;
		params.push_back(evalA);
		params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

		return evalB.call(evalA.getValueType(), params);
	}
	default:
	{
		auto evalA = a->evaluate(scope);
		if (args.size() > 0)
			return evalA.call(args[0].getValueType(), args);
		else
			return evalA.call(NIL, args);
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

AddI::AddI(Instruction *a, Instruction *b) : BinaryI(ADD, a, b) {}

Symbol AddI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA + evalB;
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

SubI::SubI(Instruction *a, Instruction *b) : BinaryI(SUB, a, b) {}

Symbol SubI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA - evalB;
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

MulI::MulI(Instruction *a, Instruction *b) : BinaryI(MUL, a, b) {}

Symbol MulI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA * evalB;
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

DivI::DivI(Instruction *a, Instruction *b) : BinaryI(DIV, a, b) {}

Symbol DivI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA / evalB;
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

ModI::ModI(Instruction *a, Instruction *b) : BinaryI(MOD, a, b) {}

Symbol ModI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA % evalB;
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

PowI::PowI(Instruction *a, Instruction *b) : BinaryI(POW_I, a, b) {}

Symbol PowI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA ^ evalB;
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

LessI::LessI(Instruction *a, Instruction *b) : BinaryI(LESS, a, b) {}

Symbol LessI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA < evalB);
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

MoreI::MoreI(Instruction *a, Instruction *b) : BinaryI(MORE, a, b) {}

Symbol MoreI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA > evalB);
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

ELessI::ELessI(Instruction *a, Instruction *b) : BinaryI(ELESS, a, b) {}

Symbol ELessI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA <= evalB);
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

EMoreI::EMoreI(Instruction *a, Instruction *b) : BinaryI(EMORE, a, b) {}

Symbol EMoreI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA >= evalB);
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

Equals::Equals(Instruction *a, Instruction *b) : BinaryI(EQUALS, a, b) {}

Symbol Equals::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA == evalB);
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

NEquals::NEquals(Instruction *a, Instruction *b) : BinaryI(NEQUALS, a, b) {}

Symbol NEquals::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA != evalB);
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

AndI::AndI(Instruction *a, Instruction *b) : BinaryI(AND, a, b) {}

Symbol AndI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	if (!evalA.getBool())
		return Symbol(false);
	auto evalB = b->evaluate(scope);
	if (evalB.getBool())
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

OrI::OrI(Instruction *a, Instruction *b) : BinaryI(OR, a, b) {}

Symbol OrI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	if (evalA.getBool())
		return Symbol(true);
	auto evalB = b->evaluate(scope);
	if (evalB.getBool())
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
/*class SetI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

SetI::SetI(Instruction *a, Instruction *b) : BinaryI(SET, a, b) {}

Symbol SetI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	evalA.set(evalB);
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

ReturnI::ReturnI(Instruction *a) : UnaryI(RETURN, a) {}

Symbol ReturnI::evaluate(Scope &scope) const
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

ExternI::ExternI(const std::string &id, Instruction *a) : UnaryI(EXTERN, a), id(id)
{
	if (rlib::loaded.find(id) != rlib::loaded.end())
		this->f = rlib::loaded.at(id);
	else
		throw std::runtime_error("External function `" + id + "` is not defined");
}

Symbol ExternI::evaluate(Scope &scope) const
{
	return f(a->evaluate(scope).getVector());
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

LengthI::LengthI(Instruction *a) : UnaryI(LENGTH, a) {}

Symbol LengthI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case STRING:
	{
		std::string str = evalA.getString();
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
				return Symbol(NUMBER_NEW_LONG(evalA.getString().size()));
		}
		return Symbol(NUMBER_NEW_LONG(q));
	}
	case DICTIONARY:
		return Symbol(NUMBER_NEW_LONG(evalA.dictionarySize()));
	case VECTOR:
		return Symbol(NUMBER_NEW_LONG(evalA.vectorSize()));
	default:
		throw std::runtime_error("Cannot get length of value");
	}
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

SizeI::SizeI(Instruction *a) : UnaryI(SIZE_I, a) {}

Symbol SizeI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getValueType())
	{
	case STRING:
		return Symbol(NUMBER_NEW_LONG(evalA.getString().size()));
	case DICTIONARY:
		return Symbol(NUMBER_NEW_LONG(evalA.dictionarySize()));
	case VECTOR:
		return Symbol(NUMBER_NEW_LONG(evalA.vectorSize()));
	default:
		throw std::runtime_error("Cannot get length of value");
	}
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

ClassI::ClassI(hashcode_t key, OBJECT_TYPE type, std::shared_ptr<Instruction> body, Instruction *extends) : Instruction(CLASS_I), key(key), type(type), body(body), extends(extends) {}

Symbol ClassI::evaluate(Scope &scope) const
{
	std::shared_ptr<Instruction> nbody = body;
	if (extends != NULL)
	{
		auto e = extends->evaluate(scope);
		auto eo = e.getObject();
		if (eo->getType() == STATIC_O)
			throw std::runtime_error("Cannot extend a statically declared Object");
		auto eb = eo->getBody().get();
		std::vector<Instruction *> temp = {body.get(), eb};
		nbody = std::make_shared<ScopeI>(temp);
	}
	std::shared_ptr<Object> o = std::make_shared<Object>(scope, type, nbody, hash.deHash(key));
	if (type == STATIC_O)
		body->evaluate(*o->getScope());
	auto d = Symbol(o);
	scope.createVariable(key, d);
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

NewI::NewI(Instruction *a, Instruction *b) : BinaryI(NEW_I, a, b) {}

Symbol NewI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	auto base = evalA.getObject();
	return base->instantiate(evalB.getVector());
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

CastToI::CastToI(Instruction *a, D_TYPE convert) : UnaryI(CAST_TO_I, a), convert(convert) {}

Symbol CastToI::evaluate(Scope &scope) const
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
			return Symbol(evalA.toString());
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
				return Symbol(NUMBER_NEW_DOUBLE(std::stold(evalA.getString())));
			}
			catch (const std::invalid_argument &e)
			{
				throw std::runtime_error("String `" + evalA.getString() + "` cannot be converted to Number");
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
			return Symbol(NUMBER_GET_LONG(evalA.getNumber()) != 0);
		case STRING:
			return Symbol(evalA.getString() == "true");
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
			auto v = evalA.getVector();
			std::map<hashcode_t, Symbol> nd;
			for (size_t i = 0; i < v.size(); i++)
			{
				nd[hash.hashString(std::to_string(i))] = v[i];
			}
			return Symbol(nd);
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
			auto dict = evalA.getDictionary();
			std::vector<Symbol> nv;
			for (auto &e : dict)
			{
				nv.push_back(Symbol({{Ruota::HASH_KEY, Symbol(hash.deHash(e.first))}, {Ruota::HASH_VALUE, e.second}}));
			}
			return Symbol(nv);
		}
		case STRING:
		{
			std::string str = evalA.getString();
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
			return Symbol(static_cast<signed long long>(hash.hashString(evalA.getObject()->getName())));
	}
	case NIL:
		return Symbol();
	default:
		break;
	}
	throw std::runtime_error("Cannot convert between given types");
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

AllocI::AllocI(Instruction *a) : UnaryI(ALLOC_I, a) {}

Symbol AllocI::evaluate(Scope &scope) const
{
	auto evalA = NUMBER_GET_LONG(a->evaluate(scope).getNumber());
	if (evalA < 0)
		throw std::runtime_error("Cannot initialize a Vector with size 0");
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

UntilI::UntilI(Instruction *a, Instruction *b) : BinaryI(UNTIL_I, a, b) {}

Symbol UntilI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope).getNumber();
	auto evalB = b->evaluate(scope).getNumber();
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

ScopeI::ScopeI(std::vector<Instruction *> children) : Instruction(SCOPE_I), children(children) {}

Symbol ScopeI::evaluate(Scope &scope) const
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

MapI::MapI(std::map<hashcode_t, Instruction *> children) : Instruction(MAP_I), children(children) {}

Symbol MapI::evaluate(Scope &scope) const
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

ReferI::ReferI(Instruction *a) : UnaryI(REFER_I, a) {}

Symbol ReferI::evaluate(Scope &scope) const
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

SwitchI::SwitchI(Instruction *switchs, std::map<Symbol, Instruction *> cases, Instruction *elses) : Instruction(SWITCH_I), switchs(switchs), cases(cases), elses(elses) {}

Symbol SwitchI::evaluate(Scope &scope) const
{
	auto eval = switchs->evaluate(scope);
	if (cases.find(eval) != cases.end())
	{
		Scope newScope(scope, "");
		return cases.at(eval)->evaluate(newScope);
	}
	else if (elses != NULL)
	{
		Scope newScope(scope, "");
		return elses->evaluate(newScope);
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