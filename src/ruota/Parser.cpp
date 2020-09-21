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

Container::Container(const Symbol &d) : Instruction(CONTAINER), d(d) {}

Symbol Container::evaluate(Scope &) const
{
	return d;
}

const std::string Container::toString() const
{
	return d.toString();
}

/*-------------------------------------------------------------------------------------------------------*/
/*class DefineI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

DefineI::DefineI(const std::string &key, D_TYPE ftype, std::vector<std::pair<LEX_TOKEN_TYPE, std::string>> params, std::shared_ptr<Instruction> body) : Instruction(DEFINE), key(key), ftype(ftype), params(params), body(body) {}

Symbol DefineI::evaluate(Scope &scope) const
{
	auto f = std::make_shared<Function>(scope, params, body);
	if (key != "")
	{
		auto d = Symbol(ftype, f);
		scope.createVariable(key, d);
		return d;
	}

	return Symbol(NIL, f);
}

const std::string DefineI::toString() const
{
	if (key != "")
		return "define(" + key + ", " + std::to_string(params.size()) + ", " + body->toString() + ")";
	else
		return "lambda(" + std::to_string(params.size()) + ", " + body->toString() + ")";
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

const std::string Sequence::toString() const
{
	std::string ret = "seq(";

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

Symbol IfElseI::evaluate(Scope &scope) const
{
	Scope newScope(scope, "");
	auto evalIf = ifs->evaluate(newScope);
	if (evalIf.getBool())
	{
		auto temp = body->evaluate(newScope);
		if (temp.getSymbolType() == ID_RETURN || temp.getSymbolType() == ID_BREAK)
			return temp;
	}
	else if (elses)
	{
		auto temp = elses->evaluate(newScope);
		if (temp.getSymbolType() == ID_RETURN || temp.getSymbolType() == ID_BREAK)
			return temp;
	}
	return Symbol();
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

Symbol WhileI::evaluate(Scope &scope) const
{
	while (whiles->evaluate(scope).getBool())
	{
		Scope newScope(scope, "");
		auto temp = body->evaluate(newScope);
		if (temp.getSymbolType() == ID_RETURN)
			return temp;
		if (temp.getSymbolType() == ID_BREAK)
			break;
	}
	return Symbol();
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

Symbol ForI::evaluate(Scope &scope) const
{
	auto evalFor = fors->evaluate(scope);
	for (size_t i = 0; i < evalFor.vectorSize(); i++)
	{
		Scope newScope(scope, "");
		newScope.createVariable(id, evalFor.indexVector(i));
		auto temp = body->evaluate(newScope);
		if (temp.getSymbolType() == ID_RETURN)
			return temp;
		if (temp.getSymbolType() == ID_BREAK)
			break;
	}
	return Symbol();
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

Symbol VariableI::evaluate(Scope &scope) const
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

Symbol DeclareI::evaluate(Scope &scope) const
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

Symbol IndexI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	switch (evalA.getType())
	{
	case VECTOR:
		return evalA.indexVector(evalB.getNumber().getLong());
		break;
	case DICTIONARY:
		return evalA.indexDictionary(evalB.getString());
		break;
	default:
		throw std::runtime_error("Cannot index value");
	}
}

const std::string IndexI::toString() const
{
	return "index(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class InnerI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

InnerI::InnerI(Instruction *a, Instruction *b) : BinaryI(INNER, a, b) {}

Symbol InnerI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getType())
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

const std::string InnerI::toString() const
{
	return "inner(" + a->toString() + ", " + b->toString() + ")";
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
		switch (evalA.getType())
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

		return evalB.call(evalA.getType(), params);
	}
	default:
	{
		auto evalA = a->evaluate(scope);
		if (args.size() > 0)
			return evalA.call(args[0].getType(), args);
		else
			return evalA.call(NIL, args);
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

Symbol AddI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA + evalB;
}

const std::string AddI::toString() const
{
	return "add(" + a->toString() + ", " + b->toString() + ")";
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

const std::string SubI::toString() const
{
	return "sub(" + a->toString() + ", " + b->toString() + ")";
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

const std::string MulI::toString() const
{
	return "mul(" + a->toString() + ", " + b->toString() + ")";
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

const std::string DivI::toString() const
{
	return "div(" + a->toString() + ", " + b->toString() + ")";
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

const std::string ModI::toString() const
{
	return "mod(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class PowI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

PowI::PowI(Instruction *a, Instruction *b) : BinaryI(POW_I, a, b) {}

Symbol PowI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA.pow(evalB);
}

const std::string PowI::toString() const
{
	return "pow(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

LessI::LessI(Instruction *a, Instruction *b) : BinaryI(LESS, a, b) {}

Symbol LessI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA < evalB;
}

const std::string LessI::toString() const
{
	return "less(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

MoreI::MoreI(Instruction *a, Instruction *b) : BinaryI(MORE, a, b) {}

Symbol MoreI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA > evalB;
}

const std::string MoreI::toString() const
{
	return "more(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ELessI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

ELessI::ELessI(Instruction *a, Instruction *b) : BinaryI(ELESS, a, b) {}

Symbol ELessI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA <= evalB;
}

const std::string ELessI::toString() const
{
	return "eless(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class EMoreI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

EMoreI::EMoreI(Instruction *a, Instruction *b) : BinaryI(EMORE, a, b) {}

Symbol EMoreI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return evalA >= evalB;
}

const std::string EMoreI::toString() const
{
	return "emore(" + a->toString() + ", " + b->toString() + ")";
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

const std::string Equals::toString() const
{
	return "equals(" + a->toString() + ", " + b->toString() + ")";
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

const std::string NEquals::toString() const
{
	return "nequals(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class AndI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

AndI::AndI(Instruction *a, Instruction *b) : BinaryI(AND, a, b) {}

Symbol AndI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA && evalB);
}

const std::string AndI::toString() const
{
	return "and(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class OrI                                                                                      */
/*-------------------------------------------------------------------------------------------------------*/

OrI::OrI(Instruction *a, Instruction *b) : BinaryI(OR, a, b) {}

Symbol OrI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	return Symbol(evalA || evalB);
}

const std::string OrI::toString() const
{
	return "or(" + a->toString() + ", " + b->toString() + ")";
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

const std::string SetI::toString() const
{
	return "set(" + a->toString() + ", " + b->toString() + ")";
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

const std::string ReturnI::toString() const
{
	return "return(" + a->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class ExternI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

ExternI::ExternI(boost::function<Symbol(std::vector<Symbol>)> f, Instruction *a) : UnaryI(EXTERN, a), f(f) {}

Symbol ExternI::evaluate(Scope &scope) const
{
	return f(a->evaluate(scope).getVector());
}

const std::string ExternI::toString() const
{
	return "extern_call()";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class LengthI                                                                                          */
/*-------------------------------------------------------------------------------------------------------*/

LengthI::LengthI(Instruction *a) : UnaryI(LENGTH, a) {}

Symbol LengthI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getType())
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
				return Symbol((long_double_t)evalA.getString().size());
		}
		return Symbol((long_double_t)q);
	}
	case DICTIONARY:
		return Symbol((long_double_t)evalA.dictionarySize());
	case VECTOR:
		return Symbol((long_double_t)evalA.vectorSize());
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

Symbol SizeI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	switch (evalA.getType())
	{
	case STRING:
		return Symbol((long_double_t)evalA.getString().size());
	case DICTIONARY:
		return Symbol((long_double_t)evalA.dictionarySize());
	case VECTOR:
		return Symbol((long_double_t)evalA.vectorSize());
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

Symbol ClassI::evaluate(Scope &scope) const
{
	std::shared_ptr<Object> o = std::make_shared<Object>(scope, type, body, key);
	if (type == STATIC_O)
		body->evaluate(*o->getScope());
	auto d = Symbol(o);
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

Symbol NewI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	auto evalB = b->evaluate(scope);
	auto base = evalA.getObject();
	return base->instantiate(evalB.getVector());
}

const std::string NewI::toString() const
{
	return "new(" + a->toString() + ", " + b->toString() + ")";
}

/*-------------------------------------------------------------------------------------------------------*/
/*class TypeI                                                                                            */
/*-------------------------------------------------------------------------------------------------------*/

TypeI::TypeI(Instruction *a) : UnaryI(TYPE_I, a) {}

Symbol TypeI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope);
	return Symbol(evalA.getTypeString());
}

const std::string TypeI::toString() const
{
	return "type(" + a->toString() + ")";
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
		switch (evalA.getType())
		{
		case STRING:
			return evalA;
		default:
			return Symbol(evalA.toString());
		}
	case NUMBER:
		switch (evalA.getType())
		{
		case NUMBER:
			return evalA;
		case NIL:
			return Symbol((long_double_t)0.0);
		case STRING:
			try
			{
				return Symbol(std::stold(evalA.getString()));
			}
			catch (const std::invalid_argument &e)
			{
				throw std::runtime_error("String `" + evalA.getString() + "` cannot be converted to Number");
			}
		default:
			break;
		}
	case BOOLEAN_D:
		switch (evalA.getType())
		{
		case BOOLEAN_D:
			return evalA;
		case NIL:
			return Symbol(false);
		case NUMBER:
			return Symbol(evalA.getNumber().getLong() != 0);
		case STRING:
			return Symbol(evalA.getString() == "true");
		default:
			break;
		}
	case DICTIONARY:
		switch (evalA.getType())
		{
		case DICTIONARY:
			return evalA;
		case VECTOR:
		{
			auto v = evalA.getVector();
			std::map<std::string, Symbol> nd;
			for (size_t i = 0; i < v.size(); i++)
			{
				nd[std::to_string(i)] = v[i];
			}
			return Symbol(nd);
		}
		default:
			break;
		}
	case VECTOR:
		switch (evalA.getType())
		{
		case VECTOR:
			return evalA;
		case DICTIONARY:
		{
			auto dict = evalA.getDictionary();
			std::vector<Symbol> nv;
			for (auto &e : dict)
			{
				nv.push_back(Symbol({{"key", Symbol(e.first)}, {"value", e.second}}));
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
	case NIL:
		return Symbol();
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

Symbol AllocI::evaluate(Scope &scope) const
{
	auto evalA = a->evaluate(scope).getNumber();
	std::vector<Symbol> v(evalA.getLong());
	return Symbol(v);
}

const std::string AllocI::toString() const
{
	return "alloc(" + a->toString() + ")";
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

const std::string UntilI::toString() const
{
	return "until(" + a->toString() + ", " + b->toString() + ")";
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
		if (eval.getSymbolType() == ID_RETURN || eval.getSymbolType() == ID_BREAK)
			return eval;
	}
	return Symbol();
}

const std::string ScopeI::toString() const
{
	std::string ret = "scope(";

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

ScopeI::~ScopeI()
{
	for (auto &e : children)
		delete e;
}

/*-------------------------------------------------------------------------------------------------------*/
/*class MapI                                                                                             */
/*-------------------------------------------------------------------------------------------------------*/

MapI::MapI(std::map<std::string, Instruction *> children) : Instruction(MAP_I), children(children) {}

Symbol MapI::evaluate(Scope &scope) const
{
	std::map<std::string, Symbol> evals;
	for (auto &e : children)
	{
		auto eval = e.second->evaluate(scope);
		if (eval.getType() == NIL)
			continue;
		evals[e.first] = eval;
	}
	return Symbol(evals);
}

const std::string MapI::toString() const
{
	std::string ret = "dict(";

	unsigned long i = 0;
	for (auto &e : children)
	{
		if (i > 0)
			ret += ", ";
		ret += e.first + " : " + e.second->toString();
		i++;
	}
	return ret + ")";
}

MapI::~MapI()
{
	for (auto &e : children)
		delete e.second;
}