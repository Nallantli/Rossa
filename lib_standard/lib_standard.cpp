#include "../main/mediator/mediator.h"

#include <random>
#include <regex>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#endif

ROSSA_EXT_SIG(_puts, args)
{
	auto s = COERCE_STRING(args[0]);
	std::cout << s;
	return mediator_t();
}

ROSSA_EXT_SIG(_timeMS, args)
{
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(ms.count())));
}

ROSSA_EXT_SIG(_timeMicroS, args)
{
	auto ms = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(ms.count())));
}

ROSSA_EXT_SIG(_timeNanoS, args)
{
	auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch());
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(ms.count())));
}

ROSSA_EXT_SIG(_math_rand, args)
{
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(rand())));
}

ROSSA_EXT_SIG(_math_srand, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	srand(v0.getLong());
	return mediator_t();
}

ROSSA_EXT_SIG(_rand_init, args)
{

	auto v0 = COERCE_NUMBER(args[0]);
	auto rng = std::make_shared<std::mt19937>(v0.getLong());
	return mediator_t(
		MEDIATOR_POINTER,
		rng);
}

ROSSA_EXT_SIG(_rand_nextFloat, args)
{
	auto v0 = COERCE_POINTER(args[0], std::mt19937).get();
	auto v1 = COERCE_NUMBER(args[1]);
	auto v2 = COERCE_NUMBER(args[2]);

	std::uniform_real_distribution<long_double_t> distribution(v1.getDouble(), v2.getDouble());
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(distribution(*v0))));
}

ROSSA_EXT_SIG(_rand_nextInt, args)
{
	auto v0 = COERCE_POINTER(args[0], std::mt19937);
	auto v1 = COERCE_NUMBER(args[1]);
	auto v2 = COERCE_NUMBER(args[2]);

	std::uniform_int_distribution<long_int_t> distribution(v1.getLong(), v2.getLong());
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(distribution(*v0))));
}

ROSSA_EXT_SIG(_exit, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	exit(v0.getLong());
	return mediator_t();
}

ROSSA_EXT_SIG(_system_call, args)
{
	auto v0 = COERCE_STRING(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(system(v0.c_str()))));
}

ROSSA_EXT_SIG(_sleep, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	std::this_thread::sleep_for(std::chrono::milliseconds(v0.getLong()));
	return mediator_t();
}

ROSSA_EXT_SIG(_log, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::log(v0.getDouble()))));
}

ROSSA_EXT_SIG(_sin, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::sin(v0.getDouble()))));
}

ROSSA_EXT_SIG(_asin, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::asin(v0.getDouble()))));
}

ROSSA_EXT_SIG(_cos, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::cos(v0.getDouble()))));
}

ROSSA_EXT_SIG(_acos, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::acos(v0.getDouble()))));
}

ROSSA_EXT_SIG(_tan, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::tan(v0.getDouble()))));
}

ROSSA_EXT_SIG(_atan, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::atan(v0.getDouble()))));
}

ROSSA_EXT_SIG(_sinh, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::sinh(v0.getDouble()))));
}

ROSSA_EXT_SIG(_cosh, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::cosh(v0.getDouble()))));
}

ROSSA_EXT_SIG(_tanh, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::tanh(v0.getDouble()))));
}

ROSSA_EXT_SIG(_asinh, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::asinh(v0.getDouble()))));
}

ROSSA_EXT_SIG(_acosh, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::acosh(v0.getDouble()))));
}

ROSSA_EXT_SIG(_atanh, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Double(std::atanh(v0.getDouble()))));
}

ROSSA_EXT_SIG(_floor, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	if (v0.type == number_t::LONG_NUM)
		return mediator_t(MEDIATOR_NUMBER, std::make_shared<number_t>(v0));
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(std::floor(v0.getDouble()))));
}

ROSSA_EXT_SIG(_ceil, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	if (v0.type == number_t::LONG_NUM)
		return mediator_t(MEDIATOR_NUMBER, std::make_shared<number_t>(v0));
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(std::ceil(v0.getDouble()))));
}

ROSSA_EXT_SIG(_round, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	if (v0.type == number_t::LONG_NUM)
		return mediator_t(MEDIATOR_NUMBER, std::make_shared<number_t>(v0));
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(std::round(v0.getDouble()))));
}

ROSSA_EXT_SIG(_input_line, args)
{
	std::string line;
	std::getline(std::cin, line);
	return mediator_t(
		MEDIATOR_STRING,
		std::make_shared<std::string>(line));
}

ROSSA_EXT_SIG(_input_token, args)
{
	std::string tok;
	std::cin >> tok;
	return mediator_t(
		MEDIATOR_STRING,
		std::make_shared<std::string>(tok));
}

ROSSA_EXT_SIG(_input_char, args)
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
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(c)));
}

ROSSA_EXT_SIG(_regex_match, args)
{
	auto v0 = COERCE_STRING(args[0]);
	auto v1 = COERCE_STRING(args[1]);

	std::regex r(v0);
	std::string s = v1;
	std::vector<mediator_t> v;
	for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r); i != std::sregex_iterator(); i++)
	{
		std::string m = (*i).str();
		v.push_back(mediator_t(MEDIATOR_STRING, std::make_shared<std::string>(m)));
	}
	return mediator_t(
		MEDIATOR_ARRAY,
		std::make_shared<std::vector<mediator_t>>(v));
}

ROSSA_EXT_SIG(_regex_replace, args)
{
	auto v0 = COERCE_STRING(args[0]);
	auto v1 = COERCE_STRING(args[1]);
	auto v2 = COERCE_STRING(args[2]);

	std::regex r(v0);
	std::string rpl = v1;
	std::string s = v2;

	return mediator_t(
		MEDIATOR_STRING,
		std::make_shared<std::string>(std::regex_replace(s, r, rpl)));
}

ROSSA_EXT_SIG(_clock_format, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	auto v1 = COERCE_STRING(args[1]);

	std::time_t raw = v0.getLong();
	std::tm *time = std::localtime(&raw);
	auto format = v1;
	std::stringstream ss;
	ss << std::put_time(time, format.c_str());
	return mediator_t(
		MEDIATOR_STRING,
		std::make_shared<std::string>(ss.str()));
}

ROSSA_EXT_SIG(_string_size, args)
{
	auto v0 = COERCE_STRING(args[0]);
	return mediator_t(
		MEDIATOR_NUMBER,
		std::make_shared<number_t>(number_t::Long(v0.size())));
}

/*
ROSSA_EXT_SIG(_function_split, args, token, hash, stack_trace)
{
	auto f = args[0].getFunctionOverloads(token, stack_trace);
	auto varg = args[0].getVARGFunction(token, stack_trace);
	std::map<const std::string, const symbol_t> m;
	for (auto &e : f)
	{
		if (e.first == 0)
		{
			m.insert({"0", symbol_t::FunctionSIG({}, e.second.at(signature_t()))});
		}
		else
		{
			std::map<const std::string, const symbol_t> m2;
			for (auto &e2 : e.second)
			{
				m2.insert({"Function<" + e2.first.toString() + ">", symbol_t::FunctionSIG(e2.first, e2.second)});
			}
			m.insert({std::to_string(e.first), symbol_t::Dictionary(m2)});
		}
	}
	if (varg != nullptr)
		m.insert({"...", symbol_t::FunctionVARG(varg)});
	return symbol_t::Dictionary(m);
}
*/

EXPORT_FUNCTIONS(lib_standard)
{
	ADD_EXT(_acos);
	ADD_EXT(_asin);
	ADD_EXT(_atan);
	ADD_EXT(_acosh);
	ADD_EXT(_asinh);
	ADD_EXT(_atanh);
	ADD_EXT(_ceil);
	ADD_EXT(_clock_format);
	ADD_EXT(_cos);
	ADD_EXT(_cosh);
	ADD_EXT(_exit);
	ADD_EXT(_exit);
	ADD_EXT(_floor);
	ADD_EXT(_input_char);
	ADD_EXT(_input_line);
	ADD_EXT(_log);
	ADD_EXT(_math_rand);
	ADD_EXT(_math_srand);
	ADD_EXT(_puts);
	ADD_EXT(_rand_init);
	ADD_EXT(_rand_nextFloat);
	ADD_EXT(_rand_nextInt);
	ADD_EXT(_regex_match);
	ADD_EXT(_regex_replace);
	ADD_EXT(_round);
	ADD_EXT(_sin);
	ADD_EXT(_sinh);
	ADD_EXT(_sleep);
	ADD_EXT(_system_call);
	ADD_EXT(_system_call);
	ADD_EXT(_tan);
	ADD_EXT(_tanh);
	ADD_EXT(_timeMS);
	ADD_EXT(_timeMicroS);
	ADD_EXT(_timeNanoS);
	ADD_EXT(_string_size);
	// ADD_EXT(_function_split);
	ADD_EXT(_input_token);
}