#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <boost/generator_iterator.hpp>

#ifndef __unix__
#include <conio.h>
#endif

#include "../ruota/RuotaTypes.h"

RUOTA_LIB_HEADER

namespace libstd
{
	RUOTA_EXT_SYM(_puts, args)
	{
		std::cout << args[0].getString();
		return Symbol();
	}

	RUOTA_EXT_SYM(_timeMS, args)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return Symbol(NUMBER_NEW_LONG(ms.count()));
	}

	RUOTA_EXT_SYM(_rand_init, args)
	{
		auto rng = std::make_shared<std::default_random_engine>(NUMBER_GET_LONG(args[0].getNumber()));
		return Symbol(static_cast<std::shared_ptr<void>>(rng));
	}

	RUOTA_EXT_SYM(_rand_nextFloat, args)
	{
		auto rng = static_cast<std::default_random_engine *>(args[0].getPointer().get());
		std::uniform_real_distribution<long_double_t> distribution(NUMBER_GET_DOUBLE(args[1].getNumber()), NUMBER_GET_DOUBLE(args[2].getNumber()));
		return Symbol(NUMBER_NEW_DOUBLE(distribution(*rng)));
	}

	RUOTA_EXT_SYM(_rand_nextInt, args)
	{
		auto rng = static_cast<std::default_random_engine *>(args[0].getPointer().get());
		std::uniform_int_distribution<long_int_t> distribution(NUMBER_GET_LONG(args[1].getNumber()), NUMBER_GET_LONG(args[2].getNumber()));
		return Symbol(NUMBER_NEW_LONG(distribution(*rng)));
	}

	RUOTA_EXT_SYM(_exit, args)
	{
		exit(NUMBER_GET_LONG(args[0].getNumber()));
	}

	RUOTA_EXT_SYM(_system_call, args)
	{
		return Symbol(NUMBER_NEW_LONG(system(args[0].getString().c_str())));
	}

	RUOTA_EXT_SYM(_sleep, args)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds((long long)NUMBER_GET_LONG(args[0].getNumber())));
		return Symbol();
	}

	RUOTA_EXT_SYM(_log, args)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::log(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_sin, args)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::sin(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_cos, args)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::cos(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_tan, args)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::tan(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_sinh, args)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::sinh(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_cosh, args)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::cosh(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_tanh, args)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::tanh(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_floor, args)
	{
#ifdef _USE_CONV_NUM_
		if (args[0].getNumber().type == LONG_NUM)
			return args[0];
#endif
		return Symbol(NUMBER_NEW_LONG(std::floor(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_ceil, args)
	{
#ifdef _USE_CONV_NUM_
		if (args[0].getNumber().type == LONG_NUM)
			return args[0];
#endif
		return Symbol(NUMBER_NEW_LONG(std::ceil(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_round, args)
	{
#ifdef _USE_CONV_NUM_
		if (args[0].getNumber().type == LONG_NUM)
			return args[0];
#endif
		return Symbol(NUMBER_NEW_LONG(std::round(NUMBER_GET_DOUBLE(args[0].getNumber()))));
	}

	RUOTA_EXT_SYM(_input_line, args)
	{
		std::string line;
		std::getline(std::cin, line);
		return Symbol(line);
	}

	RUOTA_EXT_SYM(_input_char, args)
	{
#ifdef __unix__
		system("stty raw");
		char c = getchar();
		system("stty cooked");
#else
		char c = getch();
#endif
		return Symbol(std::string(1, c));
	}
} // namespace libstd