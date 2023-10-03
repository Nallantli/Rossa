#include "../main/mediator/mediator.h"

#include <zip.h>

#include <cstring>
#include <fstream>
#include <filesystem>

namespace lib_fs
{
	inline void unzip(const std::filesystem::path &zipdir, const std::filesystem::path &unzipdir)
	{
		int error;
		zip_t *z = zip_open(zipdir.string().c_str(), ZIP_RDONLY, &error);
		if (z == NULL)
		{
			switch (error)
			{
			case ZIP_ER_INVAL:
			case ZIP_ER_NOENT:
				throw library_error_t("Path to ZIP archive is invalid");
			case ZIP_ER_NOZIP:
				throw library_error_t("Path does not point to a ZIP archive");
			case ZIP_ER_OPEN:
				throw library_error_t("ZIP archive cannot be opened");
			case ZIP_ER_READ:
				throw library_error_t("ZIP archive cannot be read (possibly corrupt)");
			case ZIP_ER_MEMORY:
				throw library_error_t("Enough memory could not be allocated");
			default:
				throw library_error_t("An error occured while attempting to open archive");
			}
		}

		struct zip_stat statBuffer;
		zip_file_t *f;
		zip_uint64_t totalRead;
		char binBuffer[100];
		for (int i = 0; i < zip_get_num_entries(z, 0); i++)
		{
			if (zip_stat_index(z, i, 0, &statBuffer) == 0)
			{
				if (statBuffer.name[strlen(statBuffer.name) - 1] == '/')
				{
					std::filesystem::create_directories(unzipdir / statBuffer.name);
				}
				else
				{
					f = zip_fopen_index(z, i, 0);
					std::ofstream file(unzipdir / statBuffer.name, std::ios_base::binary);
					totalRead = 0;
					while (totalRead != statBuffer.size)
					{
						int nlen = zip_fread(f, binBuffer, 100);
						if (nlen < 0)
							throw library_error_t("Error reading file within archive (possibly corrupt)");
						file.write(binBuffer, nlen);
						totalRead += nlen;
					}
					file.close();
					zip_fclose(f);
				}
			}
			else
			{
				throw library_error_t("Error reading file within archive (possibly corrupt)");
			}
		}

		if (zip_close(z) == -1)
		{
			throw library_error_t("Attempt to close ZIP archive failed");
		}
	}
}

ROSSA_EXT_SIG(_writer_init, args)
{
	auto filename = COERCE_STRING(args[0]);
	auto fstr = std::make_shared<std::ofstream>();
	if (COERCE_BOOLEAN(args[1]))
		fstr->open(filename, std::ios::binary);
	else
		fstr->open(filename, std::ios::binary);
	if (!fstr->is_open())
		throw library_error_t("Failure to initialize writer for filepath <" + filename + ">");

	return MAKE_POINTER(fstr);
}

ROSSA_EXT_SIG(_writer_close, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ofstream);

	fstr->close();
	return mediator_t();
}

ROSSA_EXT_SIG(_writer_isOpen, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ofstream);

	return MAKE_BOOLEAN(fstr->is_open());
}

ROSSA_EXT_SIG(_reader_init, args)
{
	auto filename = COERCE_STRING(args[0]);
	auto fstr = std::make_shared<std::ifstream>();
	if (COERCE_BOOLEAN(args[1]))
		fstr->open(filename, std::ios::binary);
	else
		fstr->open(filename, std::ios::binary);
	if (!fstr->is_open())
		throw library_error_t("Failure to initialize reader for filepath <" + filename + ">");

	return MAKE_POINTER(fstr);
}

ROSSA_EXT_SIG(_reader_close, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ifstream);

	fstr->close();
	return mediator_t();
}

ROSSA_EXT_SIG(_reader_isOpen, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ifstream);

	return MAKE_BOOLEAN(fstr->is_open());
}

ROSSA_EXT_SIG(_reader_readLine, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ifstream);

	std::string line;
	if (std::getline(*fstr, line))
		return MAKE_STRING(line);
	return mediator_t();
}

ROSSA_EXT_SIG(_reader_read, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ifstream);

	size_t max = COERCE_NUMBER(args[1]).getLong();

	std::string line = "";
	char c;
	for (size_t i = 0; i < max; i++)
	{
		if (fstr->get(c))
			line += c;
		else
			break;
	}
	return MAKE_STRING(line);
}

ROSSA_EXT_SIG(_reader_size, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ifstream);

	fstr->seekg(0, std::ios::end);
	auto size = fstr->tellg();
	fstr->seekg(0, std::ios::beg);
	return MAKE_NUMBER(number_t::Long(size));
}

ROSSA_EXT_SIG(_writer_write, args)
{
	auto fstr = COERCE_POINTER(args[0], std::ofstream);

	*fstr << COERCE_STRING(args[1]);
	return mediator_t();
}

ROSSA_EXT_SIG(_path_init, args)
{
	auto pathstr = COERCE_STRING(args[0]);
	auto path = std::make_shared<std::filesystem::path>(pathstr);

	return MAKE_POINTER(path);
}

ROSSA_EXT_SIG(_path_filename, args)
{
	auto path = COERCE_POINTER(args[0], std::filesystem::path);

	return MAKE_STRING(path->filename().string());
}

ROSSA_EXT_SIG(_path_string, args)
{
	auto path = COERCE_POINTER(args[0], std::filesystem::path);

	return MAKE_STRING(std::filesystem::weakly_canonical(*path).string());
}

ROSSA_EXT_SIG(_path_mkdirs, args)
{
	auto path = COERCE_POINTER(args[0], std::filesystem::path);

	std::filesystem::create_directories(*path);
	return mediator_t();
}

ROSSA_EXT_SIG(_path_exists, args)
{
	auto path = COERCE_POINTER(args[0], std::filesystem::path);

	return MAKE_BOOLEAN(std::filesystem::exists(path->string()));
}

ROSSA_EXT_SIG(_path_append_path, args)
{
	auto path1 = COERCE_POINTER(args[0], std::filesystem::path);
	auto path2 = COERCE_POINTER(args[1], std::filesystem::path);

	return MAKE_STRING((*path1 / *path2).string());
}

ROSSA_EXT_SIG(_path_append_string, args)
{
	auto path1 = COERCE_POINTER(args[0], std::filesystem::path);
	auto path2 = COERCE_STRING(args[1]);

	return MAKE_STRING((*path1 / path2).string());
}

ROSSA_EXT_SIG(_path_unzip_a, args)
{
	auto path1 = COERCE_POINTER(args[0], std::filesystem::path);
	auto path2 = COERCE_POINTER(args[1], std::filesystem::path);

	lib_fs::unzip(*path1, *path2);
	return mediator_t();
}

ROSSA_EXT_SIG(_path_unzip_b, args)
{
	auto path1 = COERCE_POINTER(args[0], std::filesystem::path);
	auto path2 = COERCE_STRING(args[1]);

	lib_fs::unzip(*path1, path2);
	return mediator_t();
}

EXPORT_FUNCTIONS(lib_fs)
{
	ADD_EXT(_path_append_path);
	ADD_EXT(_path_append_string);
	ADD_EXT(_path_exists);
	ADD_EXT(_path_exists);
	ADD_EXT(_path_filename);
	ADD_EXT(_path_init);
	ADD_EXT(_path_mkdirs);
	ADD_EXT(_path_string);
	ADD_EXT(_reader_close);
	ADD_EXT(_reader_init);
	ADD_EXT(_reader_size);
	ADD_EXT(_reader_isOpen);
	ADD_EXT(_reader_read);
	ADD_EXT(_reader_readLine);
	ADD_EXT(_writer_close);
	ADD_EXT(_writer_init);
	ADD_EXT(_writer_isOpen);
	ADD_EXT(_writer_write);
	ADD_EXT(_path_unzip_a);
	ADD_EXT(_path_unzip_b);
}