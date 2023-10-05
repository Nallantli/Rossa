#include "operation.h"

#include "../symbol/symbol.h"
#include "../rossa_error/rossa_error.h"
#include "../object/object.h"
#include "../instruction/instruction.h"
#include "../parser/parser.h"
#include "../global/global.h"

#define COMP(a, b) (a == value_type_enum::OBJECT ? value_type_enum::OBJECT : (((char)-a << 4) | (char)-b))

const symbol_t operation::index(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::DICTIONARY, value_type_enum::STRING):
		return evalA.indexDict(evalB.getString(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::NUMBER):
	{
		auto num = evalB.getNumber(token, stack_trace);
		if (num.type != number_t::LONG_NUM)
			throw rossa_error_t(global::format("Cannot index with non integral value: {0}", {num.toCodeString()}), *token, stack_trace);
		return evalA.indexVector(evalB.getNumber(token, stack_trace).getLong(), token, stack_trace);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_INDEX))
			return o->getVariable(parser_t::HASH_INDEX, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_INDEX, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"[]"}), *token, stack_trace);
}

const symbol_t operation::call(const object_t *scope, const ptr_instruction_t &a, const std::vector<symbol_t> &args, const token_t *token, trace_t &stack_trace)
{
	const symbol_t evalA = a->evaluate(scope, stack_trace);

	if (evalA.getValueType() == value_type_enum::OBJECT)
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_CALL))
			return o->getVariable(parser_t::HASH_CALL, token, stack_trace).call(args, token, stack_trace);
	}

	return evalA.call(args, token, stack_trace);
}

const symbol_t operation::callWithInner(const object_t *scope, const ptr_instruction_t &a, const std::vector<symbol_t> &args, const token_t *token, trace_t &stack_trace)
{
	const symbol_t evalA = reinterpret_cast<const InnerI *>(a.get())->getA()->evaluate(scope, stack_trace);
	if (evalA.getValueType() == value_type_enum::OBJECT)
	{
		const symbol_t evalB = reinterpret_cast<const InnerI *>(a.get())->getB()->evaluate(evalA.getObject(token, stack_trace), stack_trace);
		return evalB.call(args, token, stack_trace);
	}
	else
	{
		const symbol_t evalB = reinterpret_cast<const InnerI *>(a.get())->getB()->evaluate(scope, stack_trace);
		std::vector<symbol_t> params;
		params.push_back(evalA);
		params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

		if (evalB.getValueType() == value_type_enum::OBJECT)
		{
			const auto &o = evalB.getObject(token, stack_trace);
			if (o->hasValue(parser_t::HASH_CALL))
				return o->getVariable(parser_t::HASH_CALL, token, stack_trace).call(args, token, stack_trace);
		}

		return evalB.call(params, token, stack_trace);
	}
}

const symbol_t operation::untilstep_inclusive(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const symbol_t &step, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
	{
		number_t numA = evalA.getNumber(token, stack_trace);
		const number_t &numB = evalB.getNumber(token, stack_trace);
		const number_t &numStep = step.getNumber(token, stack_trace);
		std::vector<symbol_t> nv;
		for (; numA <= numB; numA += numStep)
		{
			nv.push_back(symbol_t::Number(numA));
		}
		return symbol_t::Array(nv);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_RANGE_INC))
			return o->getVariable(parser_t::HASH_RANGE_INC, token, stack_trace).call({evalB, step}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_RANGE_INC, token, stack_trace).call({evalA, evalB, step}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"<>"}), *token, stack_trace);
}

const symbol_t operation::untilnostep_inclusive(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
	{
		number_t numA = evalA.getNumber(token, stack_trace);
		const number_t &numB = evalB.getNumber(token, stack_trace);
		const number_t &numStep = number_t::Long(1);
		std::vector<symbol_t> nv;
		for (; numA <= numB; numA += numStep)
		{
			nv.push_back(symbol_t::Number(numA));
		}
		return symbol_t::Array(nv);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_RANGE_INC))
			return o->getVariable(parser_t::HASH_RANGE_INC, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_RANGE_INC, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"<>"}), *token, stack_trace);
}

const symbol_t operation::untilstep_exclusive(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const symbol_t &step, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
	{
		number_t numA = evalA.getNumber(token, stack_trace);
		const number_t &numB = evalB.getNumber(token, stack_trace);
		const number_t &numStep = step.getNumber(token, stack_trace);
		std::vector<symbol_t> nv;
		for (; numA < numB; numA += numStep)
		{
			nv.push_back(symbol_t::Number(numA));
		}
		return symbol_t::Array(nv);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_RANGE_EXC))
			return o->getVariable(parser_t::HASH_RANGE_EXC, token, stack_trace).call({evalB, step}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_RANGE_EXC, token, stack_trace).call({evalA, evalB, step}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {".."}), *token, stack_trace);
}

const symbol_t operation::untilnostep_exclusive(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
	{
		number_t numA = evalA.getNumber(token, stack_trace);
		const number_t &numB = evalB.getNumber(token, stack_trace);
		const number_t &numStep = number_t::Long(1);
		std::vector<symbol_t> nv;
		for (; numA < numB; numA += numStep)
		{
			nv.push_back(symbol_t::Number(numA));
		}
		return symbol_t::Array(nv);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_RANGE_EXC))
			return o->getVariable(parser_t::HASH_RANGE_EXC, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_RANGE_EXC, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {".."}), *token, stack_trace);
}

const symbol_t operation::add(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) + evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = add(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_ADD))
			return o->getVariable(parser_t::HASH_ADD, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_ADD, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"+"}), *token, stack_trace);
}

const symbol_t operation::sub(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) - evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = sub(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_SUB))
		{
			return o->getVariable(parser_t::HASH_SUB, token, stack_trace).call({evalB}, token, stack_trace);
		}
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_SUB, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"-"}), *token, stack_trace);
}

const symbol_t operation::mul(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) * evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = mul(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_MUL))
			return o->getVariable(parser_t::HASH_MUL, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_MUL, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"*"}), *token, stack_trace);
}

const symbol_t operation::div(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) / evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = div(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_DIV))
			return o->getVariable(parser_t::HASH_DIV, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_DIV, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"/"}), *token, stack_trace);
}

const symbol_t operation::fdiv(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(number_t::Long((evalA.getNumber(token, stack_trace) / evalB.getNumber(token, stack_trace)).getLong()));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = div(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_FDIV))
			return o->getVariable(parser_t::HASH_FDIV, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_FDIV, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"//"}), *token, stack_trace);
}

const symbol_t operation::mod(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) % evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = mod(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_MOD))
			return o->getVariable(parser_t::HASH_MOD, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_MOD, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"%"}), *token, stack_trace);
}

const symbol_t operation::pow(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace).pow(evalB.getNumber(token, stack_trace)));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = pow(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_POW))
		{
			return o->getVariable(parser_t::HASH_POW, token, stack_trace).call({evalB}, token, stack_trace);
		}
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_POW, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"**"}), *token, stack_trace);
}

const symbol_t operation::less(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Boolean(evalA.getNumber(token, stack_trace) < evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::STRING, value_type_enum::STRING):
		return symbol_t::Boolean(evalA.getString(token, stack_trace) < evalB.getString(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = less(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_LESS))
			return o->getVariable(parser_t::HASH_LESS, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_LESS, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"<"}), *token, stack_trace);
}

const symbol_t operation::more(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Boolean(evalA.getNumber(token, stack_trace) > evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::STRING, value_type_enum::STRING):
		return symbol_t::Boolean(evalA.getString(token, stack_trace) > evalB.getString(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = more(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_MORE))
			return o->getVariable(parser_t::HASH_MORE, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_MORE, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {">"}), *token, stack_trace);
}

const symbol_t operation::eless(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Boolean(evalA.getNumber(token, stack_trace) <= evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::STRING, value_type_enum::STRING):
		return symbol_t::Boolean(evalA.getString(token, stack_trace) <= evalB.getString(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = eless(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_ELESS))
			return o->getVariable(parser_t::HASH_ELESS, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_ELESS, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"<="}), *token, stack_trace);
}

const symbol_t operation::emore(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Boolean(evalA.getNumber(token, stack_trace) >= evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::STRING, value_type_enum::STRING):
		return symbol_t::Boolean(evalA.getString(token, stack_trace) >= evalB.getString(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = emore(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_EMORE))
			return o->getVariable(parser_t::HASH_EMORE, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_EMORE, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {">="}), *token, stack_trace);
}

const symbol_t operation::bor(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) | evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::BOOLEAN_D, value_type_enum::BOOLEAN_D):
		return symbol_t::Boolean(evalA.getBool(token, stack_trace) | evalB.getBool(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = bor(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_B_OR))
			return o->getVariable(parser_t::HASH_B_OR, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_B_OR, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"|"}), *token, stack_trace);
}

const symbol_t operation::bxor(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) ^ evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::BOOLEAN_D, value_type_enum::BOOLEAN_D):
		return symbol_t::Boolean(evalA.getBool(token, stack_trace) ^ evalB.getBool(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = bxor(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_B_XOR))
			return o->getVariable(parser_t::HASH_B_XOR, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_B_XOR, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"^"}), *token, stack_trace);
}

const symbol_t operation::band(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) & evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::BOOLEAN_D, value_type_enum::BOOLEAN_D):
		return symbol_t::Boolean(evalA.getBool(token, stack_trace) & evalB.getBool(token, stack_trace));
	case COMP(value_type_enum::STRING, value_type_enum::ARRAY):
	{
		std::vector<std::string> elems;
		for (const symbol_t &e : evalB.getVector(token, stack_trace))
			elems.push_back(e.toString(token, stack_trace));
		return symbol_t::String(global::format(evalA.getString(token, stack_trace), elems));
	}
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = band(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_B_AND))
			return o->getVariable(parser_t::HASH_B_AND, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_B_AND, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"&"}), *token, stack_trace);
}

const symbol_t operation::bshl(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) << evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = bshl(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_B_SH_L))
			return o->getVariable(parser_t::HASH_B_SH_L, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_B_SH_L, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"<<"}), *token, stack_trace);
}

const symbol_t operation::bshr(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::NUMBER, value_type_enum::NUMBER):
		return symbol_t::Number(evalA.getNumber(token, stack_trace) >> evalB.getNumber(token, stack_trace));
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		auto av = evalA.getVector(token, stack_trace);
		auto bv = evalB.getVector(token, stack_trace);
		if (av.size() != bv.size())
			throw rossa_error_t(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = bshr(scope, av[i], bv[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_B_SH_R))
			return o->getVariable(parser_t::HASH_B_SH_R, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_B_SH_R, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {">>"}), *token, stack_trace);
}

const symbol_t operation::bnot(const object_t *scope, const symbol_t &evalA, const token_t *token, trace_t &stack_trace)
{
	switch (evalA.getValueType())
	{
	case value_type_enum::NUMBER:
		return symbol_t::Number(~evalA.getNumber(token, stack_trace));
	case value_type_enum::ARRAY:
	{
		auto av = evalA.getVector(token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = bnot(scope, av[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_B_NOT))
			return o->getVariable(parser_t::HASH_B_NOT, token, stack_trace).call({}, token, stack_trace);
	}
	default:
		break;
	}

	if (scope != NULL)
		return scope->getVariable(parser_t::HASH_B_NOT, token, stack_trace).call({evalA}, token, stack_trace);

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"~"}), *token, stack_trace);
}

const symbol_t operation::unadd(const object_t *scope, const symbol_t &evalA, const token_t *token, trace_t &stack_trace)
{
	if (evalA.getValueType() == value_type_enum::OBJECT)
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_ADD))
			return o->getVariable(parser_t::HASH_ADD, token, stack_trace).call({}, token, stack_trace);

		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_ADD, token, stack_trace).call({evalA}, token, stack_trace);

		throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"+"}), *token, stack_trace);
	}
	symbol_t temp;
	temp.set(&evalA, token, stack_trace);
	return temp;
}

const symbol_t operation::neg(const object_t *scope, const symbol_t &evalA, const token_t *token, trace_t &stack_trace)
{
	switch (evalA.getValueType())
	{
	case value_type_enum::NUMBER:
		return symbol_t::Number(-evalA.getNumber(token, stack_trace));
	case value_type_enum::ARRAY:
	{
		auto av = evalA.getVector(token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = neg(scope, av[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_SUB))
			return o->getVariable(parser_t::HASH_SUB, token, stack_trace).call({}, token, stack_trace);
	}
	default:
		break;
	}

	if (scope != NULL)
		return scope->getVariable(parser_t::HASH_SUB, token, stack_trace).call({evalA}, token, stack_trace);

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"-"}), *token, stack_trace);
}

const symbol_t operation::unot(const object_t *scope, const symbol_t &evalA, const token_t *token, trace_t &stack_trace)
{
	switch (evalA.getValueType())
	{
	case value_type_enum::BOOLEAN_D:
		return symbol_t::Boolean(!evalA.getBool(token, stack_trace));
	case value_type_enum::ARRAY:
	{
		auto av = evalA.getVector(token, stack_trace);
		std::vector<symbol_t> v(av.size());
		for (size_t i = 0; i < v.size(); i++)
			v[i] = unot(scope, av[i], token, stack_trace);
		return symbol_t::Array(v);
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_NOT))
			return o->getVariable(parser_t::HASH_NOT, token, stack_trace).call({}, token, stack_trace);
	}
	default:
		break;
	}

	if (scope != NULL)
		return scope->getVariable(parser_t::HASH_NOT, token, stack_trace).call({evalA}, token, stack_trace);

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"!"}), *token, stack_trace);
}

const symbol_t operation::cct(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::ARRAY, value_type_enum::ARRAY):
	{
		std::vector<symbol_t> valA = evalA.getVector(token, stack_trace);
		const std::vector<symbol_t> &valB = evalB.getVector(token, stack_trace);
		valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
		return symbol_t::Array(valA);
	}
	case COMP(value_type_enum::DICTIONARY, value_type_enum::DICTIONARY):
	{
		std::map<const std::string, const symbol_t> valA = evalA.getDictionary(token, stack_trace);
		std::map<const std::string, const symbol_t> valB = evalB.getDictionary(token, stack_trace);
		valA.merge(valB);
		return symbol_t::Dictionary(valA);
	}
	case COMP(value_type_enum::STRING, value_type_enum::STRING):
		return symbol_t::String(evalA.getString(token, stack_trace) + evalB.getString(token, stack_trace));
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_CCT))
			return o->getVariable(parser_t::HASH_CCT, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_CCT, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"++"}), *token, stack_trace);
}

const symbol_t operation::del(const object_t *scope, const symbol_t &evalA, const symbol_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType()))
	{
	case COMP(value_type_enum::DICTIONARY, value_type_enum::STRING):
	{
		auto it = evalA.getDictionary(token, stack_trace).find(evalB.getString(token, stack_trace));
		evalA.getDictionary(token, stack_trace).erase(it);
		return evalA;
	}
	case value_type_enum::OBJECT:
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_DEL))
			return o->getVariable(parser_t::HASH_DEL, token, stack_trace).call({evalB}, token, stack_trace);
	}
	default:
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_DEL, token, stack_trace).call({evalA, evalB}, token, stack_trace);
	}

	throw rossa_error_t(global::format(_UNDECLARED_OPERATOR_ERROR_, {"delete"}), *token, stack_trace);
}

const symbol_t operation::hash(const object_t *scope, const symbol_t &evalA, const token_t *token, trace_t &stack_trace)
{
	if (evalA.getValueType() == value_type_enum::OBJECT)
	{
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(parser_t::HASH_HASH))
			return o->getVariable(parser_t::HASH_HASH, token, stack_trace).call({}, token, stack_trace);
		if (scope != NULL)
			return scope->getVariable(parser_t::HASH_HASH, token, stack_trace).call({evalA}, token, stack_trace);
	}

	return symbol_t::Number(number_t::Long(evalA.hash()));
}