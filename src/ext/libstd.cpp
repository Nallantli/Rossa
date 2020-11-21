#include "../rossa/Rossa.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <regex>
#include <ctime>
#include <iomanip>
#include <thread>
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

using namespace rossa;

namespace libstd
{
	void threadWrapper(const std::shared_ptr<const Function> &f, const std::vector<Symbol> &args)
	{
		std::vector<Function> stack_trace;
		try {
			f->evaluate(args, NULL, stack_trace);
		} catch (const RTError &e) {
			Rossa::printError(e);
		}
	}

	ROSSA_EXT_SYM(_puts, args, token, hash, stack_trace)
	{
		std::cout << args[0].getString(token, stack_trace);
		return Symbol();
	}

	ROSSA_EXT_SYM(_timeMS, args, token, hash, stack_trace)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return Symbol(RNumber::Long(ms.count()));
	}

	ROSSA_EXT_SYM(_rand_init, args, token, hash, stack_trace)
	{
		auto rng = new std::mt19937(args[0].getNumber(token, stack_trace).getLong());
		return Symbol(static_cast<std::shared_ptr<void>>(rng));
	}

	ROSSA_EXT_SYM(_rand_nextFloat, args, token, hash, stack_trace)
	{
		auto rng = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::mt19937);

		std::uniform_real_distribution<long_double_t> distribution(args[1].getNumber(token, stack_trace).getDouble(), args[2].getNumber(token, stack_trace).getDouble());
		return Symbol(RNumber::Double(distribution(*rng)));
	}

	ROSSA_EXT_SYM(_rand_nextInt, args, token, hash, stack_trace)
	{
		auto rng = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::mt19937);

		std::uniform_int_distribution<long_int_t> distribution(args[1].getNumber(token, stack_trace).getLong(), args[2].getNumber(token, stack_trace).getLong());
		return Symbol(RNumber::Long(distribution(*rng)));
	}

	ROSSA_EXT_SYM(_exit, args, token, hash, stack_trace)
	{
		exit(args[0].getNumber(token, stack_trace).getLong());
	}

	ROSSA_EXT_SYM(_system_call, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Long(system(args[0].getString(token, stack_trace).c_str())));
	}

	ROSSA_EXT_SYM(_sleep, args, token, hash, stack_trace)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(args[0].getNumber(token, stack_trace).getLong()));
		return Symbol();
	}

	ROSSA_EXT_SYM(_log, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Double(std::log(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_sin, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Double(std::sin(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_cos, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Double(std::cos(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_tan, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Double(std::tan(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_sinh, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Double(std::sinh(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_cosh, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Double(std::cosh(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_tanh, args, token, hash, stack_trace)
	{
		return Symbol(RNumber::Double(std::tanh(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_floor, args, token, hash, stack_trace)
	{
		if (args[0].getNumber(token, stack_trace).type == RNumber::LONG_NUM)
			return args[0];
		return Symbol(RNumber::Long(std::floor(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_ceil, args, token, hash, stack_trace)
	{
		if (args[0].getNumber(token, stack_trace).type == RNumber::LONG_NUM)
			return args[0];
		return Symbol(RNumber::Long(std::ceil(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_round, args, token, hash, stack_trace)
	{
		if (args[0].getNumber(token, stack_trace).type == RNumber::LONG_NUM)
			return args[0];
		return Symbol(RNumber::Long(std::round(args[0].getNumber(token, stack_trace).getDouble())));
	}

	ROSSA_EXT_SYM(_input_line, args, token, hash, stack_trace)
	{
		std::string line;
		std::getline(std::cin, line);
		return Symbol(line);
	}

	ROSSA_EXT_SYM(_input_char, args, token, hash, stack_trace)
	{
#ifdef __unix__
		struct termios oldattr, newattr;
		tcgetattr(0, &oldattr);
		newattr = oldattr;
		newattr.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(0, TCSANOW, &newattr);
		char c = getchar();
		tcsetattr(0, TCSANOW, &oldattr);
#else
		char c = getch();
#endif
		return Symbol(RNumber::Long(c));
	}

	ROSSA_EXT_SYM(_regex_match, args, token, hash, stack_trace)
	{
		std::regex r(args[0].getString(token, stack_trace));
		std::string s = args[1].getString(token, stack_trace);
		std::vector<Symbol> v;
		for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r); i != std::sregex_iterator(); i++) {
			std::string m = (*i).str();
			v.push_back(Symbol(m));
		}
		return Symbol(v);
	}

	ROSSA_EXT_SYM(_regex_replace, args, token, hash, stack_trace)
	{
		std::regex r(args[0].getString(token, stack_trace));
		std::string rpl = args[1].getString(token, stack_trace);
		std::string s = args[2].getString(token, stack_trace);
		return Symbol(std::regex_replace(s, r, rpl));
	}

	ROSSA_EXT_SYM(_clock_format, args, token, hash, stack_trace)
	{
		std::time_t raw = args[0].getNumber(token, stack_trace).getLong();
		std::tm *time = std::localtime(&raw);
		auto format = args[1].getString(token, stack_trace);
		std::stringstream ss;
		ss << std::put_time(time, format.c_str());
		return Symbol(ss.str());
	}

	ROSSA_EXT_SYM(_thread_init, args, token, hash, stack_trace)
	{
		auto params = args[1].getVector(token, stack_trace);
		auto f = args[0].getFunction(params, token, stack_trace);
		auto t = std::make_shared<std::thread>(threadWrapper, f, params);
		return Symbol(static_cast<std::shared_ptr<void>>(t));
	}

	ROSSA_EXT_SYM(_thread_join, args, token, hash, stack_trace)
	{
		auto t = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::thread);

		if (t->joinable())
			t->join();

		return Symbol();
	}

	ROSSA_EXT_SYM(_thread_detach, args, token, hash, stack_trace)
	{
		auto t = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::thread);

		if (t->joinable())
			t->detach();

		return Symbol();
	}
}