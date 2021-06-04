#include "../main/rossa/rossa.h"

#include "../main/rossa/symbol/symbol.h"
#include "../main/rossa/parser/parser.h"
#include "../main/rossa/function/function.h"
#include "../main/rossa/signature/signature.h"

#include <random>
#include <regex>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#endif

namespace libstd
{
	inline void threadWrapper(const ptr_function_t &f)
	{
		trace_t stack_trace;
		try {
			function_evaluate(f, {}, NULL, stack_trace);
		} catch (const rossa_error_t &e) {
			parser_t::printError(e);
		}
	}
};

ROSSA_EXT_SIG(_puts, args, token, hash, stack_trace)
{
	std::cout << args[0].getString(token, stack_trace);
	return symbol_t();
}

ROSSA_EXT_SIG(_timeMS, args, token, hash, stack_trace)
{
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return symbol_t::Number(number_t::Long(ms.count()));
}

ROSSA_EXT_SIG(_math_rand, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(rand()));
}

ROSSA_EXT_SIG(_math_srand, args, token, hash, stack_trace)
{
	srand(args[0].getNumber(token, stack_trace).getLong());
	return symbol_t();
}

ROSSA_EXT_SIG(_rand_init, args, token, hash, stack_trace)
{
	auto rng = std::make_shared<std::mt19937>(args[0].getNumber(token, stack_trace).getLong());
	return symbol_t::Pointer(rng);
}

ROSSA_EXT_SIG(_rand_nextFloat, args, token, hash, stack_trace)
{
	auto rng = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::mt19937);

	std::uniform_real_distribution<long_double_t> distribution(args[1].getNumber(token, stack_trace).getDouble(), args[2].getNumber(token, stack_trace).getDouble());
	return symbol_t::Number(number_t::Double(distribution(*rng)));
}

ROSSA_EXT_SIG(_rand_nextInt, args, token, hash, stack_trace)
{
	auto rng = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::mt19937);

	std::uniform_int_distribution<long_int_t> distribution(args[1].getNumber(token, stack_trace).getLong(), args[2].getNumber(token, stack_trace).getLong());
	return symbol_t::Number(number_t::Long(distribution(*rng)));
}

ROSSA_EXT_SIG(_exit, args, token, hash, stack_trace)
{
	exit(args[0].getNumber(token, stack_trace).getLong());
}

ROSSA_EXT_SIG(_system_call, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(system(args[0].getString(token, stack_trace).c_str())));
}

ROSSA_EXT_SIG(_sleep, args, token, hash, stack_trace)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(args[0].getNumber(token, stack_trace).getLong()));
	return symbol_t();
}

ROSSA_EXT_SIG(_log, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::log(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_sin, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::sin(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_asin, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::asin(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_cos, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::cos(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_acos, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::acos(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_tan, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::tan(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_atan, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::atan(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_sinh, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::sinh(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_cosh, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::cosh(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_tanh, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::tanh(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_asinh, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::asinh(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_acosh, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::acosh(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_atanh, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Double(std::atanh(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_floor, args, token, hash, stack_trace)
{
	if (args[0].getNumber(token, stack_trace).type == number_t::LONG_NUM)
		return args[0];
	return symbol_t::Number(number_t::Long(std::floor(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_ceil, args, token, hash, stack_trace)
{
	if (args[0].getNumber(token, stack_trace).type == number_t::LONG_NUM)
		return args[0];
	return symbol_t::Number(number_t::Long(std::ceil(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_round, args, token, hash, stack_trace)
{
	if (args[0].getNumber(token, stack_trace).type == number_t::LONG_NUM)
		return args[0];
	return symbol_t::Number(number_t::Long(std::round(args[0].getNumber(token, stack_trace).getDouble())));
}

ROSSA_EXT_SIG(_input_line, args, token, hash, stack_trace)
{
	std::string line;
	std::getline(std::cin, line);
	return symbol_t::String(line);
}

ROSSA_EXT_SIG(_input_token, args, token, hash, stack_trace)
{
	std::string tok;
	std::cin >> tok;
	return symbol_t::String(tok);
}

ROSSA_EXT_SIG(_input_char, args, token, hash, stack_trace)
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
	return symbol_t::Number(number_t::Long(c));
}

ROSSA_EXT_SIG(_regex_match, args, token, hash, stack_trace)
{
	std::regex r(args[0].getString(token, stack_trace));
	std::string s = args[1].getString(token, stack_trace);
	std::vector<symbol_t> v;
	for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r); i != std::sregex_iterator(); i++) {
		std::string m = (*i).str();
		v.push_back(symbol_t::String(m));
	}
	return symbol_t::Array(v);
}

ROSSA_EXT_SIG(_regex_replace, args, token, hash, stack_trace)
{
	std::regex r(args[0].getString(token, stack_trace));
	std::string rpl = args[1].getString(token, stack_trace);
	std::string s = args[2].getString(token, stack_trace);
	return symbol_t::String(std::regex_replace(s, r, rpl));
}

ROSSA_EXT_SIG(_clock_format, args, token, hash, stack_trace)
{
	std::time_t raw = args[0].getNumber(token, stack_trace).getLong();
	std::tm *time = std::localtime(&raw);
	auto format = args[1].getString(token, stack_trace);
	std::stringstream ss;
	ss << std::put_time(time, format.c_str());
	return symbol_t::String(ss.str());
}

ROSSA_EXT_SIG(_thread_init, args, token, hash, stack_trace)
{
	//auto params = args[1].getVector(token, stack_trace);
	auto f = args[0].getFunction({}, token, stack_trace);
	auto t = std::make_shared<std::thread>(libstd::threadWrapper, f);
	return symbol_t::Pointer(t);
}

ROSSA_EXT_SIG(_thread_join, args, token, hash, stack_trace)
{
	auto t = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::thread);

	if (t->joinable())
		t->join();

	return symbol_t();
}

ROSSA_EXT_SIG(_thread_detach, args, token, hash, stack_trace)
{
	auto t = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::thread);

	if (t->joinable())
		t->detach();

	return symbol_t();
}

ROSSA_EXT_SIG(_string_size, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(args[0].getString(token, stack_trace).size()));
}

ROSSA_EXT_SIG(_function_split, args, token, hash, stack_trace)
{
	auto f = args[0].getFunctionOverloads(token, stack_trace);
	auto varg = args[0].getVARGFunction(token, stack_trace);
	std::map<const std::string, const symbol_t> m;
	for (auto &e : f) {
		if (e.first == 0) {
			m.insert({ "0", symbol_t::FunctionSIG({}, e.second.at(signature_t())) });
		} else {
			std::map<const std::string, const symbol_t> m2;
			for (auto &e2 : e.second) {
				m2.insert({ "Function<" + e2.first.toString() + ">", symbol_t::FunctionSIG(e2.first, e2.second) });
			}
			m.insert({ std::to_string(e.first), symbol_t::Dictionary(m2) });
		}
	}
	if (varg != nullptr)
		m.insert({ "...", symbol_t::FunctionVARG(varg) });
	return symbol_t::Dictionary(m);
}

EXPORT_FUNCTIONS(libstd)
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
	ADD_EXT(_thread_detach);
	ADD_EXT(_thread_init);
	ADD_EXT(_thread_join);
	ADD_EXT(_timeMS);
	ADD_EXT(_string_size);
	ADD_EXT(_function_split);
	ADD_EXT(_input_token);
}