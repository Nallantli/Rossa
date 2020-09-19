#include <boost/config.hpp>
#include <iostream>
#include <stdexcept>
#include "../ruota/RuotaPlugin.hpp"

namespace libsystem
{
	extern "C" BOOST_SYMBOL_EXPORT pDatum _puts(std::vector<pDatum> args)
	{
		std::cout << args[0].getString();
		return pDatum();
	}
} // namespace libsystem