#include "../ruota/Ruota.h"

#include <fstream>
#include <boost/filesystem.hpp>

RUOTA_LIB_HEADER

namespace libfs
{
	RUOTA_EXT_SYM(_writer_init, args, token)
	{
		auto filename = args[0].getString(NULL);
		auto fstr = std::make_shared<std::ofstream>();
		fstr->open(filename);
		return Symbol(static_cast<std::shared_ptr<void>>(fstr));
	}

	RUOTA_EXT_SYM(_writer_close, args, token)
	{
		auto fstr = (std::ofstream *)args[0].getPointer(NULL).get();
		fstr->close();
		return Symbol();
	}

	RUOTA_EXT_SYM(_writer_isOpen, args, token)
	{
		auto fstr = (std::ofstream *)args[0].getPointer(NULL).get();
		return Symbol(fstr->is_open());
	}

	RUOTA_EXT_SYM(_reader_init, args, token)
	{
		auto filename = args[0].getString(NULL);
		auto fstr = std::make_shared<std::ifstream>();
		fstr->open(filename);
		return Symbol(static_cast<std::shared_ptr<void>>(fstr));
	}

	RUOTA_EXT_SYM(_reader_close, args, token)
	{
		auto fstr = (std::ifstream *)args[0].getPointer(NULL).get();
		fstr->close();
		return Symbol();
	}

	RUOTA_EXT_SYM(_reader_isOpen, args, token)
	{
		auto fstr = (std::ifstream *)args[0].getPointer(NULL).get();
		return Symbol(fstr->is_open());
	}

	RUOTA_EXT_SYM(_reader_readLine, args, token)
	{
		auto fstr = (std::ifstream *)args[0].getPointer(NULL).get();
		std::string line;
		if (std::getline(*fstr, line))
			return Symbol(line);
		return Symbol();
	}

	RUOTA_EXT_SYM(_reader_read, args, token)
	{
		auto fstr = (std::ifstream *)args[0].getPointer(NULL).get();
		std::string line = "";
		char c;
		for (size_t i = 0; i < NUMBER_GET_LONG(args[1].getNumber(NULL)); i++)
		{
			if (fstr->get(c))
				line += c;
			else
				break;
		}
		return Symbol(line);
	}

	RUOTA_EXT_SYM(_writer_write, args, token)
	{
		auto fstr = (std::ofstream *)args[0].getPointer(NULL).get();
		*fstr << args[1].getString(NULL);
		return Symbol();
	}

	RUOTA_EXT_SYM(_path_init, args, token)
	{
		auto pathstr = args[0].getString(NULL);
		auto path = std::make_shared<boost::filesystem::path>(pathstr);
		return Symbol(static_cast<std::shared_ptr<void>>(path));
	}

	RUOTA_EXT_SYM(_path_filename, args, token)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer(NULL).get();
		return Symbol(path->filename().string());
	}

	RUOTA_EXT_SYM(_path_string, args, token)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer(NULL).get();
		return Symbol(boost::filesystem::weakly_canonical(*path).string());
	}

	RUOTA_EXT_SYM(_path_mkdirs, args, token)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer(NULL).get();
		boost::filesystem::create_directories(*path);
		return Symbol();
	}

	RUOTA_EXT_SYM(_path_exists, args, token)
	{
		auto path = (boost::filesystem::path *)args[0].getPointer(NULL).get();
		return Symbol(boost::filesystem::exists(path->string()));
	}

	RUOTA_EXT_SYM(_path_append_path, args, token)
	{
		auto path1 = (boost::filesystem::path *)args[0].getPointer(NULL).get();
		auto path2 = (boost::filesystem::path *)args[1].getPointer(NULL).get();
		return Symbol((*path1 / *path2).string());
	}

	RUOTA_EXT_SYM(_path_append_string, args, token)
	{
		auto path1 = (boost::filesystem::path *)args[0].getPointer(NULL).get();
		auto path2 = args[1].getString(NULL);
		return Symbol((*path1 / path2).string());
	}
} // namespace libfs