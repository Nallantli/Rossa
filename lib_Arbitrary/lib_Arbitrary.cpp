#include "../main/mediator/mediator.h"

#include <gmpxx.h>

ROSSA_EXT_SIG(_arbitrary_init, args)
{
	std::shared_ptr<mpz_class> x = std::make_shared<mpz_class>(0);
	return MAKE_POINTER(x);
}

ROSSA_EXT_SIG(_arbitrary_set_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	*a = (long)COERCE_NUMBER(args[1]).getLong();
	return mediator_t();
}

ROSSA_EXT_SIG(_arbitrary_set_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	*a = *b;
	return mediator_t();
}

ROSSA_EXT_SIG(_arbitrary_to_string, args)
{
	auto x = COERCE_POINTER(args[0], mpz_class);
	return MAKE_STRING(x->get_str());
}

ROSSA_EXT_SIG(_arbitrary_to_number, args)
{
	auto x = COERCE_POINTER(args[0], mpz_class);
	return MAKE_NUMBER(number_t::Long(x->get_si()));
}

ROSSA_EXT_SIG(_arbitrary_add_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a + (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_add_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a + *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_sub_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a - (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_sub_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a - *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_mul_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a * (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_mul_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a * *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_div_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a / (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_div_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a / *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_mod_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a % (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_mod_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a % *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_less_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	return MAKE_BOOLEAN(*a < (long)COERCE_NUMBER(args[1]).getLong());
}

ROSSA_EXT_SIG(_arbitrary_less_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	return MAKE_BOOLEAN(*a < *b);
}

ROSSA_EXT_SIG(_arbitrary_more_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	return MAKE_BOOLEAN(*a > (long)COERCE_NUMBER(args[1]).getLong());
}

ROSSA_EXT_SIG(_arbitrary_more_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	return MAKE_BOOLEAN(*a > *b);
}

ROSSA_EXT_SIG(_arbitrary_eless_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	return MAKE_BOOLEAN(*a <= (long)COERCE_NUMBER(args[1]).getLong());
}

ROSSA_EXT_SIG(_arbitrary_eless_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	return MAKE_BOOLEAN(*a <= *b);
}

ROSSA_EXT_SIG(_arbitrary_emore_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	return MAKE_BOOLEAN(*a >= (long)COERCE_NUMBER(args[1]).getLong());
}

ROSSA_EXT_SIG(_arbitrary_emore_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	return MAKE_BOOLEAN(*a >= *b);
}

ROSSA_EXT_SIG(_arbitrary_equals_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	return MAKE_BOOLEAN(*a == (long)COERCE_NUMBER(args[1]).getLong());
}

ROSSA_EXT_SIG(_arbitrary_equals_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	return MAKE_BOOLEAN(*a == *b);
}

ROSSA_EXT_SIG(_arbitrary_nequals_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	return MAKE_BOOLEAN(*a != (long)COERCE_NUMBER(args[1]).getLong());
}

ROSSA_EXT_SIG(_arbitrary_nequals_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	return MAKE_BOOLEAN(*a != *b);
}

ROSSA_EXT_SIG(_arbitrary_band_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a & (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_band_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a & *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_bor_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a | (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_bor_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a | *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_bxor_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a ^ (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_bxor_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>(*a ^ *b);
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_pow_number, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>();
	mpz_pow_ui(c->get_mpz_t(), a->get_mpz_t(), (long)COERCE_NUMBER(args[1]).getLong());
	return MAKE_POINTER(c);
}

ROSSA_EXT_SIG(_arbitrary_pow_arbitrary, args)
{
	auto a = COERCE_POINTER(args[0], mpz_class);
	auto b = COERCE_POINTER(args[1], mpz_class);
	// why is there no power function for mpz_t mpzt?_
	std::shared_ptr<mpz_class> c = std::make_shared<mpz_class>();
	mpz_pow_ui(c->get_mpz_t(), a->get_mpz_t(), b->get_ui());
	return MAKE_POINTER(c);
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