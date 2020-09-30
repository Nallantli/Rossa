#ifndef LIBRARY_H
#define LIBRARY_H

#include "Ruota.h"

#include <boost/dll.hpp>
#include <boost/function.hpp>

namespace rdir
{
	extern std::vector<boost::filesystem::path> loaded;

	inline boost::filesystem::path findFile(boost::filesystem::path currentDir, const std::string &filename)
	{
		auto currentDirCheck = currentDir / filename;
		if (boost::filesystem::exists(currentDirCheck))
			return currentDirCheck;
		auto libDirCheck = boost::dll::program_location().parent_path() / "lib" / filename;
		if (boost::filesystem::exists(libDirCheck))
			return libDirCheck;
		throw std::runtime_error("File `" + filename + "` cannot be found");
	}
} // namespace rdir

namespace rlib
{
	extern std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, const Token *)>> loaded;

	inline void loadFunction(boost::filesystem::path currentDir, const std::string &rawlibname, const std::string &fname)
	{
		std::string libname = rawlibname;

#ifdef __unix__
		libname += ".so";
#else
		libname += ".dll";
#endif

		std::string search = rawlibname + "$" + fname;
		if (loaded.find(search) != loaded.end())
			return;

		loaded[search] = boost::dll::import<const Symbol(std::vector<Symbol>, const Token *)>(rdir::findFile(currentDir, libname), fname);
	}
} // namespace rlib

#endif