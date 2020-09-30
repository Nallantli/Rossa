#include "../ruota/Ruota.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <boost/generator_iterator.hpp>

#ifndef __unix__
#include <conio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#endif

RUOTA_LIB_HEADER

namespace libstd
{
	RUOTA_EXT_SYM(_puts, args, token)
	{
		std::cout << args[0].getString(token);
		return Symbol();
	}

	RUOTA_EXT_SYM(_timeMS, args, token)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return Symbol(NUMBER_NEW_LONG(ms.count()));
	}

	RUOTA_EXT_SYM(_rand_init, args, token)
	{
		auto rng = std::make_shared<std::default_random_engine>(NUMBER_GET_LONG(args[0].getNumber(token)));
		return Symbol(static_cast<std::shared_ptr<void>>(rng));
	}

	RUOTA_EXT_SYM(_rand_nextFloat, args, token)
	{
		auto rng = static_cast<std::default_random_engine *>(args[0].getPointer(token).get());
		std::uniform_real_distribution<long_double_t> distribution(NUMBER_GET_DOUBLE(args[1].getNumber(token)), NUMBER_GET_DOUBLE(args[2].getNumber(token)));
		return Symbol(NUMBER_NEW_DOUBLE(distribution(*rng)));
	}

	RUOTA_EXT_SYM(_rand_nextInt, args, token)
	{
		auto rng = static_cast<std::default_random_engine *>(args[0].getPointer(token).get());
		std::uniform_int_distribution<long_int_t> distribution(NUMBER_GET_LONG(args[1].getNumber(token)), NUMBER_GET_LONG(args[2].getNumber(token)));
		return Symbol(NUMBER_NEW_LONG(distribution(*rng)));
	}

	RUOTA_EXT_SYM(_exit, args, token)
	{
		exit(NUMBER_GET_LONG(args[0].getNumber(token)));
	}

	RUOTA_EXT_SYM(_system_call, args, token)
	{
		return Symbol(NUMBER_NEW_LONG(system(args[0].getString(token).c_str())));
	}

	RUOTA_EXT_SYM(_sleep, args, token)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds((long long)NUMBER_GET_LONG(args[0].getNumber(token))));
		return Symbol();
	}

	RUOTA_EXT_SYM(_log, args, token)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::log(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_sin, args, token)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::sin(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_cos, args, token)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::cos(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_tan, args, token)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::tan(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_sinh, args, token)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::sinh(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_cosh, args, token)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::cosh(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_tanh, args, token)
	{
		return Symbol(NUMBER_NEW_DOUBLE(std::tanh(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_floor, args, token)
	{
#ifdef _USE_CONV_NUM_
		if (args[0].getNumber(token).type == LONG_NUM)
			return args[0];
#endif
		return Symbol(NUMBER_NEW_LONG(std::floor(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_ceil, args, token)
	{
#ifdef _USE_CONV_NUM_
		if (args[0].getNumber(token).type == LONG_NUM)
			return args[0];
#endif
		return Symbol(NUMBER_NEW_LONG(std::ceil(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_round, args, token)
	{
#ifdef _USE_CONV_NUM_
		if (args[0].getNumber(token).type == LONG_NUM)
			return args[0];
#endif
		return Symbol(NUMBER_NEW_LONG(std::round(NUMBER_GET_DOUBLE(args[0].getNumber(token)))));
	}

	RUOTA_EXT_SYM(_input_line, args, token)
	{
		std::string line;
		std::getline(std::cin, line);
		return Symbol(line);
	}

	RUOTA_EXT_SYM(_input_char, args, token)
	{
#ifdef __unix__
		struct termios oldattr, newattr;
		int ch;
		tcgetattr(0, &oldattr);
		newattr = oldattr;
		newattr.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(0, TCSANOW, &newattr);
		char c = getchar();
		tcsetattr(0, TCSANOW, &oldattr);
#else
		char c = getch();
#endif
		return Symbol(NUMBER_NEW_LONG(c));
	}
} // namespace libstd