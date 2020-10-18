#include "../ruota/Ruota.h"

#include <fstream>
#include <boost/filesystem.hpp>

RUOTA_LIB_HEADER

namespace libfs
{
	RUOTA_EXT_SYM(_writer_init, args, token, hash)
	{
		auto filename = args[0].getString(token);
		auto fstr = std::make_shared<std::ofstream>();
		if (args[1].getBool(token))
			fstr->open(filename, std::ios::binary);
		else
			fstr->open(filename, std::ios::binary);
		return Symbol(static_cast<std::shared_ptr<void>>(fstr));
	}

	RUOTA_EXT_SYM(_writer_close, args, token, hash)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token),
			std::ofstream);

		fstr->close();
		return Symbol();
	}

	RUOTA_EXT_SYM(_writer_isOpen, args, token, hash)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token),
			std::ofstream);

		return Symbol(fstr->is_open());
	}

	RUOTA_EXT_SYM(_reader_init, args, token, hash)
	{
		auto filename = args[0].getString(token);
		auto fstr = std::make_shared<std::ifstream>();
		if (args[1].getBool(token))
			fstr->open(filename, std::ios::binary);
		else
			fstr->open(filename, std::ios::binary);
		return Symbol(static_cast<std::shared_ptr<void>>(fstr));
	}

	RUOTA_EXT_SYM(_reader_close, args, token, hash)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token),
			std::ifstream);

		fstr->close();
		return Symbol();
	}

	RUOTA_EXT_SYM(_reader_isOpen, args, token, hash)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token),
			std::ifstream);

		return Symbol(fstr->is_open());
	}

	RUOTA_EXT_SYM(_reader_readLine, args, token, hash)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token),
			std::ifstream);

		std::string line;
		if (std::getline(*fstr, line))
			return Symbol(line);
		return Symbol();
	}

	RUOTA_EXT_SYM(_reader_read, args, token, hash)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token),
			std::ifstream);

		size_t max = args[1].getNumber(token).getLong();

		std::string line = "";
		char c;
		for (size_t i = 0; i < max; i++) {
			if (fstr->get(c))
				line += c;
			else
				break;
		}
		return Symbol(line);
	}

	RUOTA_EXT_SYM(_writer_write, args, token, hash)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token),
			std::ofstream);

		*fstr << args[1].getString(token);
		return Symbol();
	}

	RUOTA_EXT_SYM(_path_init, args, token, hash)
	{
		auto pathstr = args[0].getString(token);
		auto path = std::make_shared<boost::filesystem::path>(pathstr);

		return Symbol(static_cast<std::shared_ptr<void>>(path));
	}

	RUOTA_EXT_SYM(_path_filename, args, token, hash)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token),
			boost::filesystem::path);

		return Symbol(path->filename().string());
	}

	RUOTA_EXT_SYM(_path_string, args, token, hash)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token),
			boost::filesystem::path);

		return Symbol(boost::filesystem::weakly_canonical(*path).string());
	}

	RUOTA_EXT_SYM(_path_mkdirs, args, token, hash)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token),
			boost::filesystem::path);

		boost::filesystem::create_directories(*path);
		return Symbol();
	}

	RUOTA_EXT_SYM(_path_exists, args, token, hash)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token),
			boost::filesystem::path);

		return Symbol(boost::filesystem::exists(path->string()));
	}

	RUOTA_EXT_SYM(_path_append_path, args, token, hash)
	{
		auto path1 = COERCE_PTR(
			args[0].getPointer(token),
			boost::filesystem::path);

		auto path2 = COERCE_PTR(
			args[1].getPointer(token),
			boost::filesystem::path);

		return Symbol((*path1 / *path2).string());
	}

	RUOTA_EXT_SYM(_path_append_string, args, token, hash)
	{
		auto path1 = COERCE_PTR(
			args[0].getPointer(token),
			boost::filesystem::path);

		auto path2 = args[1].getString(token);

		return Symbol((*path1 / path2).string());
	}
}