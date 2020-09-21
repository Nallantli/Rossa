#include <boost/config.hpp>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>

#include "../ruota/RuotaTypes.h"

namespace libsystem
{
	RUOTA_EXT_SYM(_puts, args)
	{
		std::cout << args[0].toString();
		return Symbol();
	}

	RUOTA_EXT_SYM(_timeMS, args)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return Symbol(Number((long_int_t)ms.count()));
	}

	RUOTA_EXT_SYM(_exit, args)
	{
		exit((int)args[0].getNumber().getLong());
	}

	RUOTA_EXT_SYM(_sleep, args)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds((long long)args[0].getNumber().getLong()));
		return Symbol();
	}

	RUOTA_EXT_SYM(_log, args)
	{
		return Symbol(std::log(args[0].getNumber().getDouble()));
	}

	RUOTA_EXT_SYM(_floor, args)
	{
		if (args[0].getNumber().type == LONG_NUM)
			return args[0];
		return Symbol(std::floor(args[0].getNumber().getDouble()));
	}

	RUOTA_EXT_SYM(_ceil, args)
	{
		if (args[0].getNumber().type == LONG_NUM)
			return args[0];
		return Symbol(std::ceil(args[0].getNumber().getDouble()));
	}

	RUOTA_EXT_SYM(_round, args)
	{
		if (args[0].getNumber().type == LONG_NUM)
			return args[0];
		return Symbol(std::round(args[0].getNumber().getDouble()));
	}
} // namespace libsystem