#include "../main/rossa/rossa.h"
#include "../main/rossa/symbol/symbol.h"
#include "../main/rossa/function/function.h"

#include <gmpxx.h>

ROSSA_EXT_SIG(_arbitrary_init, args, token, hash, stack_trace)
{
	std::shared_ptr<mpz_class> x = std::make_shared<mpz_class>(0);
	return symbol_t::Pointer(x);
}

ROSSA_EXT_SIG(_arbitrary_set_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	*a = (long)args[1].getNumber(token, stack_trace).getLong();
	return symbol_t();
}

ROSSA_EXT_SIG(_arbitrary_set_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	*a = *b;
	return symbol_t();
}

ROSSA_EXT_SIG(_arbitrary_to_string, args, token, hash, stack_trace)
{
	auto x = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::String(x->get_str());
}

ROSSA_EXT_SIG(_arbitrary_to_number, args, token, hash, stack_trace)
{
	auto x = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Number(number_t::Long(x->get_si()));
}

ROSSA_EXT_SIG(_arbitrary_add_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a + (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_add_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a + *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_sub_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a - (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_sub_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a - *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_mul_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a * (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_mul_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a * *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_div_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a / (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_div_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a / *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_mod_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a % (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_mod_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a % *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_less_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a < (long)args[1].getNumber(token, stack_trace).getLong());
}

ROSSA_EXT_SIG(_arbitrary_less_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a < *b);
}

ROSSA_EXT_SIG(_arbitrary_more_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a > (long)args[1].getNumber(token, stack_trace).getLong());
}

ROSSA_EXT_SIG(_arbitrary_more_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a > *b);
}

ROSSA_EXT_SIG(_arbitrary_eless_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a <= (long)args[1].getNumber(token, stack_trace).getLong());
}

ROSSA_EXT_SIG(_arbitrary_eless_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a <= *b);
}

ROSSA_EXT_SIG(_arbitrary_emore_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a >= (long)args[1].getNumber(token, stack_trace).getLong());
}

ROSSA_EXT_SIG(_arbitrary_emore_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a >= *b);
}

ROSSA_EXT_SIG(_arbitrary_equals_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a == (long)args[1].getNumber(token, stack_trace).getLong());
}

ROSSA_EXT_SIG(_arbitrary_equals_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a == *b);
}

ROSSA_EXT_SIG(_arbitrary_nequals_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a != (long)args[1].getNumber(token, stack_trace).getLong());
}

ROSSA_EXT_SIG(_arbitrary_nequals_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	return symbol_t::Boolean(*a != *b);
}

ROSSA_EXT_SIG(_arbitrary_band_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a & (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_band_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a & *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_bor_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a | (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_bor_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a | *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_bxor_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a ^ (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_bxor_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a ^ *b);
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_pow_number, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>();
	mpz_pow_ui(c->get_mpz_t(), a->get_mpz_t(), (long)args[1].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(c);
}

ROSSA_EXT_SIG(_arbitrary_pow_arbitrary, args, token, hash, stack_trace)
{
	auto a = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		mpz_class);
	auto b = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		mpz_class);
	// why is there no power function for mpz_t mpz_t?
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>();
	mpz_pow_ui(c->get_mpz_t(), a->get_mpz_t(), b->get_ui());
	return symbol_t::Pointer(c);
}

EXPORT_FUNCTIONS(lib_Arbitrary)
{
	ADD_EXT(_arbitrary_init);
	ADD_EXT(_arbitrary_to_string);
	ADD_EXT(_arbitrary_to_number);
	ADD_EXT(_arbitrary_add_number);
	ADD_EXT(_arbitrary_add_arbitrary);
	ADD_EXT(_arbitrary_sub_number);
	ADD_EXT(_arbitrary_sub_arbitrary);
	ADD_EXT(_arbitrary_mul_number);
	ADD_EXT(_arbitrary_mul_arbitrary);
	ADD_EXT(_arbitrary_div_number);
	ADD_EXT(_arbitrary_div_arbitrary);
	ADD_EXT(_arbitrary_mod_number);
	ADD_EXT(_arbitrary_mod_arbitrary);
	ADD_EXT(_arbitrary_less_number);
	ADD_EXT(_arbitrary_less_arbitrary);
	ADD_EXT(_arbitrary_more_number);
	ADD_EXT(_arbitrary_more_arbitrary);
	ADD_EXT(_arbitrary_eless_number);
	ADD_EXT(_arbitrary_eless_arbitrary);
	ADD_EXT(_arbitrary_emore_number);
	ADD_EXT(_arbitrary_emore_arbitrary);
	ADD_EXT(_arbitrary_equals_number);
	ADD_EXT(_arbitrary_equals_arbitrary);
	ADD_EXT(_arbitrary_nequals_number);
	ADD_EXT(_arbitrary_nequals_arbitrary);
	ADD_EXT(_arbitrary_pow_number);
	ADD_EXT(_arbitrary_pow_arbitrary);
	ADD_EXT(_arbitrary_set_number);
	ADD_EXT(_arbitrary_set_arbitrary);
}