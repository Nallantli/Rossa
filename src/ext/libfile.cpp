#include "../ruota/Ruota.h"

#include <fstream>

RUOTA_LIB_HEADER

namespace libfile
{
	RUOTA_EXT_SYM(_file_init, args)
	{
		auto filename = args[0].getString();
		auto fstr = std::make_shared<std::fstream>();
		fstr->open(filename);
		return Symbol(static_cast<std::shared_ptr<void>>(fstr));
	}

	RUOTA_EXT_SYM(_file_close, args)
	{
		auto fstr = (std::fstream *)args[0].getPointer().get();
		fstr->close();
		return Symbol();
	}

	RUOTA_EXT_SYM(_file_isOpen, args)
	{
		auto fstr = (std::fstream *)args[0].getPointer().get();
		return Symbol(fstr->is_open());
	}

	RUOTA_EXT_SYM(_file_readLine, args)
	{
		auto fstr = (std::fstream *)args[0].getPointer().get();
		std::string line;
		if (std::getline(*fstr, line))
			return Symbol(line);
		return Symbol();
	}

	RUOTA_EXT_SYM(_file_read, args)
	{
		auto fstr = (std::fstream *)args[0].getPointer().get();
		std::string line = "";
		char c;
		for (size_t i = 0; i < NUMBER_GET_LONG(args[1].getNumber()); i++)
		{
			if (fstr->get(c))
				line += c;
			else
				break;
		}
		return Symbol(line);
	}

	RUOTA_EXT_SYM(_file_write, args)
	{
		auto fstr = (std::fstream *)args[0].getPointer().get();
		*fstr << args[1].getString();
		return Symbol();
	}
} // namespace libfile