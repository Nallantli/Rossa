#include "../ruota/Ruota.h"

#include <fstream>
#include <boost/filesystem.hpp>

RUOTA_LIB_HEADER

namespace fs
{
	RUOTA_EXT_SYM(_file_init, args)
	{
		auto filename = args[0].getString();
		auto fstr = std::make_shared<std::fstream>();
		fstr->open(filename, std::fstream::in | std::fstream::out | std::fstream::ate | std::fstream::app);
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

	RUOTA_EXT_SYM(_path_init, args)
	{
		auto pathstr = args[0].getString();
		auto path = std::make_shared<boost::filesystem::path>(pathstr);
		return Symbol(static_cast<std::shared_ptr<void>>(path));
	}

	RUOTA_EXT_SYM(_path_filename, args)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer().get();
		return Symbol(path->filename().string());
	}

	RUOTA_EXT_SYM(_path_string, args)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer().get();
		return Symbol(boost::filesystem::weakly_canonical(*path).string());
	}

	RUOTA_EXT_SYM(_path_mkdirs, args)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer().get();
		boost::filesystem::create_directories(*path);
		return Symbol();
	}

	RUOTA_EXT_SYM(_path_exists, args)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer().get();
		return Symbol(boost::filesystem::exists(path->string()));
	}

	RUOTA_EXT_SYM(_path_append_path, args)
	{
		auto path1 = (boost::filesystem::path *)args[0].getPointer().get();
		auto path2 = (boost::filesystem::path *)args[1].getPointer().get();
		return Symbol((*path1 / *path2).string());
	}

	RUOTA_EXT_SYM(_path_append_string, args)
	{
		auto path1 = (boost::filesystem::path *)args[0].getPointer().get();
		auto path2 = args[1].getString();
		return Symbol((*path1 / path2).string());
	}
} // namespace fs