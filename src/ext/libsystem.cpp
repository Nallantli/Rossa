#include <boost/config.hpp>
#include <iostream>
#include <stdexcept>
#include "../ruota/RuotaTypes.hpp"

namespace libsystem
{
	RUOTA_EXT_SYM(_puts, args)
	{
		std::cout << manager::toString(args[0]);
		return manager::newValue(true);
	}
} // namespace libsystem