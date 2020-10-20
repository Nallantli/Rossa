#include "../ruota/Ruota.h"

#include <fstream>
#include <boost/filesystem.hpp>

RUOTA_LIB_HEADER

namespace libfs
{
	RUOTA_EXT_SYM(_writer_init, args, token, hash, stack_trace)
	{
		auto filename = args[0].getString(token, stack_trace);
		auto fstr = std::make_shared<std::ofstream>();
		if (args[1].getBool(token, stack_trace))
			fstr->open(filename, std::ios::binary);
		else
			fstr->open(filename, std::ios::binary);
		return Symbol(static_cast<std::shared_ptr<void>>(fstr));
	}

	RUOTA_EXT_SYM(_writer_close, args, token, hash, stack_trace)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::ofstream);

		fstr->close();
		return Symbol();
	}

	RUOTA_EXT_SYM(_writer_isOpen, args, token, hash, stack_trace)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::ofstream);

		return Symbol(fstr->is_open());
	}

	RUOTA_EXT_SYM(_reader_init, args, token, hash, stack_trace)
	{
		auto filename = args[0].getString(token, stack_trace);
		auto fstr = std::make_shared<std::ifstream>();
		if (args[1].getBool(token, stack_trace))
			fstr->open(filename, std::ios::binary);
		else
			fstr->open(filename, std::ios::binary);
		return Symbol(static_cast<std::shared_ptr<void>>(fstr));
	}

	RUOTA_EXT_SYM(_reader_close, args, token, hash, stack_trace)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::ifstream);

		fstr->close();
		return Symbol();
	}

	RUOTA_EXT_SYM(_reader_isOpen, args, token, hash, stack_trace)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::ifstream);

		return Symbol(fstr->is_open());
	}

	RUOTA_EXT_SYM(_reader_readLine, args, token, hash, stack_trace)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::ifstream);

		std::string line;
		if (std::getline(*fstr, line))
			return Symbol(line);
		return Symbol();
	}

	RUOTA_EXT_SYM(_reader_read, args, token, hash, stack_trace)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::ifstream);

		size_t max = args[1].getNumber(token, stack_trace).getLong();

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

	RUOTA_EXT_SYM(_writer_write, args, token, hash, stack_trace)
	{
		auto fstr = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			std::ofstream);

		*fstr << args[1].getString(token, stack_trace);
		return Symbol();
	}

	RUOTA_EXT_SYM(_path_init, args, token, hash, stack_trace)
	{
		auto pathstr = args[0].getString(token, stack_trace);
		auto path = std::make_shared<boost::filesystem::path>(pathstr);

		return Symbol(static_cast<std::shared_ptr<void>>(path));
	}

	RUOTA_EXT_SYM(_path_filename, args, token, hash, stack_trace)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			boost::filesystem::path);

		return Symbol(path->filename().string());
	}

	RUOTA_EXT_SYM(_path_string, args, token, hash, stack_trace)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			boost::filesystem::path);

		return Symbol(boost::filesystem::weakly_canonical(*path).string());
	}

	RUOTA_EXT_SYM(_path_mkdirs, args, token, hash, stack_trace)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			boost::filesystem::path);

		boost::filesystem::create_directories(*path);
		return Symbol();
	}

	RUOTA_EXT_SYM(_path_exists, args, token, hash, stack_trace)
	{
		auto path = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			boost::filesystem::path);

		return Symbol(boost::filesystem::exists(path->string()));
	}

	RUOTA_EXT_SYM(_path_append_path, args, token, hash, stack_trace)
	{
		auto path1 = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			boost::filesystem::path);

		auto path2 = COERCE_PTR(
			args[1].getPointer(token, stack_trace),
			boost::filesystem::path);

		return Symbol((*path1 / *path2).string());
	}

	RUOTA_EXT_SYM(_path_append_string, args, token, hash, stack_trace)
	{
		auto path1 = COERCE_PTR(
			args[0].getPointer(token, stack_trace),
			boost::filesystem::path);

		auto path2 = args[1].getString(token, stack_trace);

		return Symbol((*path1 / path2).string());
	}
}