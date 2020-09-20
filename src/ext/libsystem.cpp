#include <boost/config.hpp>
#include <iostream>
#include <stdexcept>
#include <chrono>

#include "../ruota/RuotaTypes.h"

namespace libsystem
{
	RUOTA_EXT_SYM(_puts, args)
	{
		std::cout << manager::toString(args[0]);
		return manager::newValue();
	}

	RUOTA_EXT_SYM(_timeMS, args)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return manager::newValue((long_double_t)ms.count());
	}
} // namespace libsystem