#include "../../bin/include/Rossa.h"

using namespace rossa;

const Symbol ops::index(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case DICTIONARY:
			if (evalB.getValueType() == NUMBER)
				return evalA.indexDict(evalB.toString(token, stack_trace));
			return evalA.indexDict(evalB.getString(token, stack_trace));
		case ARRAY:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.indexVector(evalB.getNumber(token, stack_trace).getLong(), token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_INDEX))
				return o->getVariable(Rossa::HASH_INDEX, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_INDEX, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "[]" }), *token, stack_trace);
}

const Symbol ops::untilstep(const std::shared_ptr<Scope> &scope, const bool &inclusive, const Symbol &evalA, const Symbol &evalB, const Symbol &step, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
		{
			if (evalB.getValueType() != NUMBER)
				break;
			auto numA = evalA.getNumber(token, stack_trace);
			auto numB = evalB.getNumber(token, stack_trace);
			auto numStep = step.getNumber(token, stack_trace);
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
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_RANGE))
				return o->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalB, step }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalA, evalB, step }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { ".." }), *token, stack_trace);
}

const Symbol ops::untilnostep(const std::shared_ptr<Scope> &scope, const bool &inclusive, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
		{
			if (evalB.getValueType() != NUMBER)
				break;
			auto numA = evalA.getNumber(token, stack_trace);
			auto numB = evalB.getNumber(token, stack_trace);
			auto numStep = RNumber::Long(1);
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
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_RANGE))
				return o->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { ".." }), *token, stack_trace);
}

const Symbol ops::add(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) + evalB.getNumber(token, stack_trace));
		case ARRAY:
		{
			if (evalB.getValueType() != ARRAY)
				break;
			auto valA = evalA.getVector(token, stack_trace);
			auto valB = evalB.getVector(token, stack_trace);
			valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
			return Symbol(valA);
		}
		case DICTIONARY:
		{
			if (evalB.getValueType() != DICTIONARY)
				break;
			auto valA = evalA.getDictionary(token, stack_trace);
			auto valB = evalB.getDictionary(token, stack_trace);
			valA.merge(valB);
			return Symbol(valA);
		}
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return Symbol(evalA.getString(token, stack_trace) + evalB.getString(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_ADD))
				return o->getVariable(Rossa::HASH_ADD, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_ADD, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "+" }), *token, stack_trace);
}

const Symbol ops::sub(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) - evalB.getNumber(token, stack_trace));
		case ARRAY:
		{
			if (evalB.getValueType() != ARRAY)
				break;
			auto vA = evalA.getVector(token, stack_trace);
			auto vB = evalB.getVector(token, stack_trace);
			std::vector<Symbol> nv;
			for (auto &e : vA) {
				bool flag = true;
				for (auto &e2 : vB) {
					if (e.equals(&e2, token, stack_trace)) {
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
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_SUB)) {
				return o->getVariable(Rossa::HASH_SUB, token, stack_trace).call({ evalB }, token, stack_trace);
			}
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_SUB, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "-" }), *token, stack_trace);
}

const Symbol ops::mul(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) * evalB.getNumber(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_MUL))
				return o->getVariable(Rossa::HASH_MUL, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_MUL, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "*" }), *token, stack_trace);
}

const Symbol ops::div(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) / evalB.getNumber(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_DIV))
				return o->getVariable(Rossa::HASH_DIV, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_DIV, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "/" }), *token, stack_trace);
}

const Symbol ops::mod(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) % evalB.getNumber(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_MOD))
				return o->getVariable(Rossa::HASH_MOD, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_MOD, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "%" }), *token, stack_trace);
}

const Symbol ops::pow(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace).pow(evalB.getNumber(token, stack_trace)));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_POW)) {
				return o->getVariable(Rossa::HASH_POW, token, stack_trace).call({ evalB }, token, stack_trace);
			}
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_POW, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "**" }), *token, stack_trace);
}

const Symbol ops::less(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(token, stack_trace) < evalB.getNumber(token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(token, stack_trace) < evalB.getString(token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_LESS))
				return o->getVariable(Rossa::HASH_LESS, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_LESS, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "<" }), *token, stack_trace);
}

const Symbol ops::more(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(token, stack_trace) > evalB.getNumber(token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(token, stack_trace) > evalB.getString(token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_MORE))
				return o->getVariable(Rossa::HASH_MORE, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_MORE, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { ">" }), *token, stack_trace);
}

const Symbol ops::eless(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(token, stack_trace) <= evalB.getNumber(token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(token, stack_trace) <= evalB.getString(token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_ELESS))
				return o->getVariable(Rossa::HASH_ELESS, token, stack_trace).call({ evalB }, token, stack_trace).getBool(token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_ELESS, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "<=" }), *token, stack_trace);
}

const Symbol ops::emore(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return evalA.getNumber(token, stack_trace) >= evalB.getNumber(token, stack_trace);
		case STRING:
			if (evalB.getValueType() != STRING)
				break;
			return evalA.getString(token, stack_trace) >= evalB.getString(token, stack_trace);
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_EMORE))
				return o->getVariable(Rossa::HASH_EMORE, token, stack_trace).call({ evalB }, token, stack_trace).getBool(token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_EMORE, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { ">=" }), *token, stack_trace);
}

const Symbol ops::bor(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) | evalB.getNumber(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_OR))
				return o->getVariable(Rossa::HASH_B_OR, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_B_OR, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "|" }), *token, stack_trace);
}

const Symbol ops::bxor(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) ^ evalB.getNumber(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_XOR))
				return o->getVariable(Rossa::HASH_B_XOR, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_B_XOR, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "^" }), *token, stack_trace);
}

const Symbol ops::band(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) & evalB.getNumber(token, stack_trace));
		case STRING:
		{
			if (evalB.getValueType() != ARRAY)
				break;
			std::vector<std::string> elems;
			for (auto &e : evalB.getVector(token, stack_trace))
				elems.push_back(e.getString(token, stack_trace));
			return Symbol(format::format(evalA.getString(token, stack_trace), elems));
		}
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_AND))
				return o->getVariable(Rossa::HASH_B_AND, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_B_AND, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "&" }), *token, stack_trace);
}

const Symbol ops::bshl(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) << evalB.getNumber(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_SH_L))
				return o->getVariable(Rossa::HASH_B_SH_L, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_B_SH_L, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { "<<" }), *token, stack_trace);
}

const Symbol ops::bshr(const std::shared_ptr<Scope> &scope, const Symbol &evalA, const Symbol &evalB, const Token *token, std::vector<Function> &stack_trace)
{
	switch (evalA.getValueType()) {
		case NUMBER:
			if (evalB.getValueType() != NUMBER)
				break;
			return Symbol(evalA.getNumber(token, stack_trace) >> evalB.getNumber(token, stack_trace));
		case OBJECT:
		{
			auto o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_SH_R))
				return o->getVariable(Rossa::HASH_B_SH_R, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_B_SH_R, token, stack_trace).call({ evalA, evalB }, token, stack_trace);

	throw RTError(format::format(_UNDECLARED_OPERATOR_ERROR_, { ">>" }), *token, stack_trace);
}