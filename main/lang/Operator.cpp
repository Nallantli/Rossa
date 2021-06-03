#include "Rossa.h"

#define COMP(a, b) (a == Value::OBJECT ? Value::OBJECT : (((char)-a << 4) | (char)-b))

const sym_t ops::index(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::DICTIONARY, Value::STRING):
			return evalA.indexDict(evalB.getString(token, stack_trace));
		case COMP(Value::ARRAY, Value::NUMBER):
		{
			auto num = evalB.getNumber(token, stack_trace);
			if (num.type != number_t::LONG_NUM)
				throw rossa_error(format::format("Cannot index with non integral value: {0}", { num.toCodeString() }), *token, stack_trace);
			return evalA.indexVector(evalB.getNumber(token, stack_trace).getLong(), token, stack_trace);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_INDEX))
				return o->getVariable(Rossa::HASH_INDEX, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_INDEX, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "[]" }), *token, stack_trace);
}

const sym_t ops::call(const scope_t *scope, const i_ptr_t &a, const sym_vec_t &args, const token_t *token, trace_t &stack_trace)
{
	switch (a->getType()) {
		case Instruction::type_t::INNER:
		{
			const sym_t evalA = reinterpret_cast<const InnerI *>(a.get())->getA()->evaluate(scope, stack_trace);
			if (evalA.getValueType() == Value::type_t::OBJECT) {
				const sym_t evalB = reinterpret_cast<const InnerI *>(a.get())->getB()->evaluate(evalA.getObject(token, stack_trace), stack_trace);
				return evalB.call(args, token, stack_trace);
			} else {
				const sym_t evalB = reinterpret_cast<const InnerI *>(a.get())->getB()->evaluate(scope, stack_trace);
				sym_vec_t params;
				params.push_back(evalA);
				params.insert(params.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

				if (evalB.getValueType() == Value::type_t::OBJECT) {
					const auto &o = evalB.getObject(token, stack_trace);
					if (o->hasValue(Rossa::HASH_CALL))
						return o->getVariable(Rossa::HASH_CALL, token, stack_trace).call(args, token, stack_trace);
				}

				return evalB.call(params, token, stack_trace);
			}
		}
		default:
		{
			const sym_t evalA = a->evaluate(scope, stack_trace);

			if (evalA.getValueType() == Value::type_t::OBJECT) {
				const auto &o = evalA.getObject(token, stack_trace);
				if (o->hasValue(Rossa::HASH_CALL))
					return o->getVariable(Rossa::HASH_CALL, token, stack_trace).call(args, token, stack_trace);
			}

			return evalA.call(args, token, stack_trace);
		}
	}
}

const sym_t ops::untilstep(const scope_t *scope, const bool &inclusive, const sym_t &evalA, const sym_t &evalB, const sym_t &step, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
		{
			number_t numA = evalA.getNumber(token, stack_trace);
			const number_t &numB = evalB.getNumber(token, stack_trace);
			const number_t &numStep = step.getNumber(token, stack_trace);
			sym_vec_t nv;
			if (inclusive) {
				for (; numA <= numB; numA += numStep)
					nv.push_back(sym_t::Number(numA));
			} else {
				for (; numA < numB; numA += numStep)
					nv.push_back(sym_t::Number(numA));
			}
			return sym_t::Array(nv);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_RANGE))
				return o->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalB, step }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalA, evalB, step }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "<>" }), *token, stack_trace);
}

const sym_t ops::untilnostep(const scope_t *scope, const bool &inclusive, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
		{
			number_t numA = evalA.getNumber(token, stack_trace);
			const number_t &numB = evalB.getNumber(token, stack_trace);
			const number_t &numStep = number_t::Long(1);
			sym_vec_t nv;
			if (inclusive) {
				for (; numA <= numB; numA += numStep)
					nv.push_back(sym_t::Number(numA));
			} else {
				for (; numA < numB; numA += numStep)
					nv.push_back(sym_t::Number(numA));
			}
			return sym_t::Array(nv);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_RANGE))
				return o->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_RANGE, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { ".." }), *token, stack_trace);
}

const sym_t ops::add(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) + evalB.getNumber(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = add(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_ADD))
				return o->getVariable(Rossa::HASH_ADD, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_ADD, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "+" }), *token, stack_trace);
}

const sym_t ops::sub(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) - evalB.getNumber(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = sub(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_SUB)) {
				return o->getVariable(Rossa::HASH_SUB, token, stack_trace).call({ evalB }, token, stack_trace);
			}
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_SUB, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "-" }), *token, stack_trace);
}

const sym_t ops::mul(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) * evalB.getNumber(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = mul(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_MUL))
				return o->getVariable(Rossa::HASH_MUL, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_MUL, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "*" }), *token, stack_trace);
}

const sym_t ops::div(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) / evalB.getNumber(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = div(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_DIV))
				return o->getVariable(Rossa::HASH_DIV, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_DIV, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "/" }), *token, stack_trace);
}

const sym_t ops::mod(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) % evalB.getNumber(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = mod(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_MOD))
				return o->getVariable(Rossa::HASH_MOD, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_MOD, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "%" }), *token, stack_trace);
}

const sym_t ops::pow(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace).pow(evalB.getNumber(token, stack_trace)));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = pow(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_POW)) {
				return o->getVariable(Rossa::HASH_POW, token, stack_trace).call({ evalB }, token, stack_trace);
			}
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_POW, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "**" }), *token, stack_trace);
}

const sym_t ops::less(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Boolean(evalA.getNumber(token, stack_trace) < evalB.getNumber(token, stack_trace));
		case COMP(Value::STRING, Value::STRING):
			return sym_t::Boolean(evalA.getString(token, stack_trace) < evalB.getString(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = less(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_LESS))
				return o->getVariable(Rossa::HASH_LESS, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_LESS, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "<" }), *token, stack_trace);
}

const sym_t ops::more(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Boolean(evalA.getNumber(token, stack_trace) > evalB.getNumber(token, stack_trace));
		case COMP(Value::STRING, Value::STRING):
			return sym_t::Boolean(evalA.getString(token, stack_trace) > evalB.getString(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = more(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_MORE))
				return o->getVariable(Rossa::HASH_MORE, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_MORE, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { ">" }), *token, stack_trace);
}

const sym_t ops::eless(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Boolean(evalA.getNumber(token, stack_trace) <= evalB.getNumber(token, stack_trace));
		case COMP(Value::STRING, Value::STRING):
			return sym_t::Boolean(evalA.getString(token, stack_trace) <= evalB.getString(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = eless(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_ELESS))
				return o->getVariable(Rossa::HASH_ELESS, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_ELESS, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "<=" }), *token, stack_trace);
}

const sym_t ops::emore(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Boolean(evalA.getNumber(token, stack_trace) >= evalB.getNumber(token, stack_trace));
		case COMP(Value::STRING, Value::STRING):
			return sym_t::Boolean(evalA.getString(token, stack_trace) >= evalB.getString(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = emore(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_EMORE))
				return o->getVariable(Rossa::HASH_EMORE, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_EMORE, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { ">=" }), *token, stack_trace);
}

const sym_t ops::bor(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) | evalB.getNumber(token, stack_trace));
		case COMP(Value::BOOLEAN_D, Value::BOOLEAN_D):
			return sym_t::Boolean(evalA.getBool(token, stack_trace) | evalB.getBool(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = bor(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_OR))
				return o->getVariable(Rossa::HASH_B_OR, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_B_OR, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "|" }), *token, stack_trace);
}

const sym_t ops::bxor(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) ^ evalB.getNumber(token, stack_trace));
		case COMP(Value::BOOLEAN_D, Value::BOOLEAN_D):
			return sym_t::Boolean(evalA.getBool(token, stack_trace) ^ evalB.getBool(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = bxor(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_XOR))
				return o->getVariable(Rossa::HASH_B_XOR, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_B_XOR, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "^" }), *token, stack_trace);
}

const sym_t ops::band(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) & evalB.getNumber(token, stack_trace));
		case COMP(Value::BOOLEAN_D, Value::BOOLEAN_D):
			return sym_t::Boolean(evalA.getBool(token, stack_trace) & evalB.getBool(token, stack_trace));
		case COMP(Value::STRING, Value::ARRAY):
		{
			std::vector<std::string> elems;
			for (const sym_t &e : evalB.getVector(token, stack_trace))
				elems.push_back(e.toString(token, stack_trace));
			return sym_t::String(format::format(evalA.getString(token, stack_trace), elems));
		}
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = band(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_AND))
				return o->getVariable(Rossa::HASH_B_AND, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_B_AND, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "&" }), *token, stack_trace);
}

const sym_t ops::bshl(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) << evalB.getNumber(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = bshl(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_SH_L))
				return o->getVariable(Rossa::HASH_B_SH_L, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_B_SH_L, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "<<" }), *token, stack_trace);
}

const sym_t ops::bshr(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::NUMBER, Value::NUMBER):
			return sym_t::Number(evalA.getNumber(token, stack_trace) >> evalB.getNumber(token, stack_trace));
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			auto av = evalA.getVector(token, stack_trace);
			auto bv = evalB.getVector(token, stack_trace);
			if (av.size() != bv.size())
				throw rossa_error(_INCOMPATIBLE_VECTOR_SIZES_, *token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = bshr(scope, av[i], bv[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_SH_R))
				return o->getVariable(Rossa::HASH_B_SH_R, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_B_SH_R, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { ">>" }), *token, stack_trace);
}

const sym_t ops::bnot(const scope_t *scope, const sym_t &evalA, const token_t *token, trace_t &stack_trace)
{
	switch (evalA.getValueType()) {
		case Value::type_t::NUMBER:
			return sym_t::Number(~evalA.getNumber(token, stack_trace));
		case Value::type_t::ARRAY:
		{
			auto av = evalA.getVector(token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = bnot(scope, av[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_B_NOT))
				return o->getVariable(Rossa::HASH_B_NOT, token, stack_trace).call({ }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_B_NOT, token, stack_trace).call({ evalA }, token, stack_trace);

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "~" }), *token, stack_trace);
}

const sym_t ops::unadd(const scope_t *scope, const sym_t &evalA, const token_t *token, trace_t &stack_trace)
{
	switch (evalA.getValueType()) {
		case Value::type_t::NUMBER:
			return sym_t::Number(+evalA.getNumber(token, stack_trace));
		case Value::type_t::ARRAY:
		{
			auto av = evalA.getVector(token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = unadd(scope, av[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_ADD))
				return o->getVariable(Rossa::HASH_ADD, token, stack_trace).call({ }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_ADD, token, stack_trace).call({ evalA }, token, stack_trace);

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "+" }), *token, stack_trace);
}

const sym_t ops::neg(const scope_t *scope, const sym_t &evalA, const token_t *token, trace_t &stack_trace)
{
	switch (evalA.getValueType()) {
		case Value::type_t::NUMBER:
			return sym_t::Number(-evalA.getNumber(token, stack_trace));
		case Value::type_t::ARRAY:
		{
			auto av = evalA.getVector(token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = neg(scope, av[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_SUB))
				return o->getVariable(Rossa::HASH_SUB, token, stack_trace).call({ }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_SUB, token, stack_trace).call({ evalA }, token, stack_trace);

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "-" }), *token, stack_trace);
}

const sym_t ops::unot(const scope_t *scope, const sym_t &evalA, const token_t *token, trace_t &stack_trace)
{
	switch (evalA.getValueType()) {
		case Value::type_t::BOOLEAN_D:
			return sym_t::Boolean(!evalA.getBool(token, stack_trace));
		case Value::type_t::ARRAY:
		{
			auto av = evalA.getVector(token, stack_trace);
			sym_vec_t v(av.size());
			for (size_t i = 0; i < v.size(); i++)
				v[i] = unot(scope, av[i], token, stack_trace);
			return sym_t::Array(v);
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_NOT))
				return o->getVariable(Rossa::HASH_NOT, token, stack_trace).call({ }, token, stack_trace);
		}
		default:
			break;
	}

	if (scope != NULL)
		return scope->getVariable(Rossa::HASH_NOT, token, stack_trace).call({ evalA }, token, stack_trace);

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "!" }), *token, stack_trace);
}

const sym_t ops::cct(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::ARRAY, Value::ARRAY):
		{
			sym_vec_t valA = evalA.getVector(token, stack_trace);
			const sym_vec_t valB = evalB.getVector(token, stack_trace);
			valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
			return sym_t::Array(valA);
		}
		case COMP(Value::DICTIONARY, Value::DICTIONARY):
		{
			sym_map_t valA = evalA.getDictionary(token, stack_trace);
			sym_map_t valB = evalB.getDictionary(token, stack_trace);
			valA.merge(valB);
			return sym_t::Dictionary(valA);
		}
		case COMP(Value::STRING, Value::STRING):
			return sym_t::String(evalA.getString(token, stack_trace) + evalB.getString(token, stack_trace));
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_CCT))
				return o->getVariable(Rossa::HASH_CCT, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_CCT, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "++" }), *token, stack_trace);
}

const sym_t ops::del(const scope_t *scope, const sym_t &evalA, const sym_t &evalB, const token_t *token, trace_t &stack_trace)
{
	switch (COMP(evalA.getValueType(), evalB.getValueType())) {
		case COMP(Value::DICTIONARY, Value::STRING):
		{
			auto it = evalA.getDictionary(token, stack_trace).find(evalB.getString(token, stack_trace));
			evalA.getDictionary(token, stack_trace).erase(it);
			return evalA;
		}
		case Value::type_t::OBJECT:
		{
			const auto &o = evalA.getObject(token, stack_trace);
			if (o->hasValue(Rossa::HASH_DEL))
				return o->getVariable(Rossa::HASH_DEL, token, stack_trace).call({ evalB }, token, stack_trace);
		}
		default:
			if (scope != NULL)
				return scope->getVariable(Rossa::HASH_DEL, token, stack_trace).call({ evalA, evalB }, token, stack_trace);
	}

	throw rossa_error(format::format(_UNDECLARED_OPERATOR_ERROR_, { "delete" }), *token, stack_trace);
}

const sym_t ops::hash(const scope_t *scope, const sym_t &evalA, const token_t *token, trace_t &stack_trace)
{
	if (evalA.getValueType() == Value::OBJECT) {
		const auto &o = evalA.getObject(token, stack_trace);
		if (o->hasValue(Rossa::HASH_HASH))
			return o->getVariable(Rossa::HASH_HASH, token, stack_trace).call({ }, token, stack_trace);
		if (scope != NULL)
			return scope->getVariable(Rossa::HASH_HASH, token, stack_trace).call({ evalA }, token, stack_trace);
	}

	return sym_t::Number(number_t::Long(evalA.hash()));
}