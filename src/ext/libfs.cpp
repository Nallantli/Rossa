#include "libfs.h"
#include <zip.h>

#include <fstream>

#ifndef _STATIC_
#ifdef __unix__
COMPILER_COMMANDS(libsdl, "-lzip")
#else
COMPILER_COMMANDS(libsdl, "-lzip")
#endif
#endif

namespace libfs
{
	inline void unzip(const std::filesystem::path &zipdir, const std::filesystem::path &unzipdir, const token_t *token, trace_t &stack_trace)
	{
		int error;
		zip_t *z = zip_open(zipdir.string().c_str(), ZIP_RDONLY, &error);
		if (z == NULL) {
			switch (error) {
				case ZIP_ER_INVAL:
				case ZIP_ER_NOENT:
					throw RTError("Path to ZIP archive is invalid", *token, stack_trace);
				case ZIP_ER_NOZIP:
					throw RTError("Path does not point to a ZIP archive", *token, stack_trace);
				case ZIP_ER_OPEN:
					throw RTError("ZIP archive cannot be opened", *token, stack_trace);
				case ZIP_ER_READ:
					throw RTError("ZIP archive cannot be read (possibly corrupt)", *token, stack_trace);
				case ZIP_ER_MEMORY:
					throw RTError("Enough memory could not be allocated", *token, stack_trace);
				default:
					throw RTError("An error occured while attempting to open archive", *token, stack_trace);
			}
		}

		struct zip_stat statBuffer;
		zip_file_t *f;
		zip_uint64_t totalRead;
		char binBuffer[100];
		for (int i = 0; i < zip_get_num_entries(z, 0); i++) {
			if (zip_stat_index(z, i, 0, &statBuffer) == 0) {
				if (statBuffer.name[strlen(statBuffer.name) - 1] == '/') {
					std::filesystem::create_directories(unzipdir / statBuffer.name);
				} else {
					f = zip_fopen_index(z, i, 0);
					std::ofstream file(unzipdir / statBuffer.name, std::ios_base::binary);
					totalRead = 0;
					while (totalRead != statBuffer.size) {
						int nlen = zip_fread(f, binBuffer, 100);
						if (nlen < 0)
							throw RTError("Error reading file within archive (possibly corrupt)", *token, stack_trace);
						file.write(binBuffer, nlen);
						totalRead += nlen;
					}
					file.close();
					zip_fclose(f);
				}
			} else {
				throw RTError("Error reading file within archive (possibly corrupt)", *token, stack_trace);
			}
		}

		if (zip_close(z) == -1) {
			throw RTError("Attempt to close ZIP archive failed", *token, stack_trace);
		}
	}
}

ROSSA_EXT_SIG(_writer_init, args, token, hash, stack_trace)
{
	auto filename = args[0].getString(token, stack_trace);
	auto fstr = std::make_shared<std::ofstream>();
	if (args[1].getBool(token, stack_trace))
		fstr->open(filename, std::ios::binary);
	else
		fstr->open(filename, std::ios::binary);
	return sym_t::Pointer(fstr);
}

ROSSA_EXT_SIG(_writer_close, args, token, hash, stack_trace)
{
	auto fstr = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::ofstream);

	fstr->close();
	return sym_t();
}

ROSSA_EXT_SIG(_writer_isOpen, args, token, hash, stack_trace)
{
	auto fstr = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::ofstream);

	return sym_t::Boolean(fstr->is_open());
}

ROSSA_EXT_SIG(_reader_init, args, token, hash, stack_trace)
{
	auto filename = args[0].getString(token, stack_trace);
	auto fstr = std::make_shared<std::ifstream>();
	if (args[1].getBool(token, stack_trace))
		fstr->open(filename, std::ios::binary);
	else
		fstr->open(filename, std::ios::binary);
	return sym_t::Pointer(fstr);
}

ROSSA_EXT_SIG(_reader_close, args, token, hash, stack_trace)
{
	auto fstr = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::ifstream);

	fstr->close();
	return sym_t();
}

ROSSA_EXT_SIG(_reader_isOpen, args, token, hash, stack_trace)
{
	auto fstr = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::ifstream);

	return sym_t::Boolean(fstr->is_open());
}

ROSSA_EXT_SIG(_reader_readLine, args, token, hash, stack_trace)
{
	auto fstr = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::ifstream);

	std::string line;
	if (std::getline(*fstr, line))
		return sym_t::String(line);
	return sym_t();
}

ROSSA_EXT_SIG(_reader_read, args, token, hash, stack_trace)
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
	return sym_t::String(line);
}

ROSSA_EXT_SIG(_writer_write, args, token, hash, stack_trace)
{
	auto fstr = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::ofstream);

	*fstr << args[1].getString(token, stack_trace);
	return sym_t();
}

ROSSA_EXT_SIG(_path_init, args, token, hash, stack_trace)
{
	auto pathstr = args[0].getString(token, stack_trace);
	auto path = std::make_shared<std::filesystem::path>(pathstr);

	return sym_t::Pointer(path);
}

ROSSA_EXT_SIG(_path_filename, args, token, hash, stack_trace)
{
	auto path = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);

	return sym_t::String(path->filename().string());
}

ROSSA_EXT_SIG(_path_string, args, token, hash, stack_trace)
{
	auto path = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);

	return sym_t::String(std::filesystem::weakly_canonical(*path).string());
}

ROSSA_EXT_SIG(_path_mkdirs, args, token, hash, stack_trace)
{
	auto path = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);

	std::filesystem::create_directories(*path);
	return sym_t();
}

ROSSA_EXT_SIG(_path_exists, args, token, hash, stack_trace)
{
	auto path = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);

	return sym_t::Boolean(std::filesystem::exists(path->string()));
}

ROSSA_EXT_SIG(_path_append_path, args, token, hash, stack_trace)
{
	auto path1 = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);

	auto path2 = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		std::filesystem::path);

	return sym_t::String((*path1 / *path2).string());
}

ROSSA_EXT_SIG(_path_append_string, args, token, hash, stack_trace)
{
	auto path1 = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);

	auto path2 = args[1].getString(token, stack_trace);

	return sym_t::String((*path1 / path2).string());
}

ROSSA_EXT_SIG(_path_unzip_a, args, token, hash, stack_trace)
{
	auto path1 = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);
	auto path2 = COERCE_PTR(
		args[1].getPointer(token, stack_trace),
		std::filesystem::path);

	libfs::unzip(*path1, *path2, token, stack_trace);
	return sym_t();
}

ROSSA_EXT_SIG(_path_unzip_b, args, token, hash, stack_trace)
{
	auto path1 = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		std::filesystem::path);
	auto path2 = args[1].getString(token, stack_trace);

	libfs::unzip(*path1, path2, token, stack_trace);
	return sym_t();
}

EXPORT_FUNCTIONS(libfs)
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