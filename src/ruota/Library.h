#ifndef LIBRARY_H
#define LIBRARY_H

#include "Ruota.h"

#include <boost/dll.hpp>
#include <boost/function.hpp>

namespace rdir
{
	extern std::vector<boost::filesystem::path> loaded;

	inline boost::filesystem::path findFile(boost::filesystem::path currentDir, const std::string &filename, const Token *token)
	{
		auto currentDirCheck = currentDir / filename;
		if (boost::filesystem::exists(currentDirCheck))
			return currentDirCheck;
		auto libDirCheck = boost::dll::program_location().parent_path() / "lib" / filename;
		if (boost::filesystem::exists(libDirCheck))
			return libDirCheck;
		throw RuotaError((boost::format(_FILE_NOT_FOUND_) % filename).str(), *token);
	}
} // namespace rdir

namespace rlib
{
	extern std::map<std::string, boost::function<const Symbol(std::vector<Symbol>, const Token *, Hash &)>> loaded;

	inline void loadFunction(boost::filesystem::path currentDir, const std::string &rawlibname, const std::string &fname, const Token *token)
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

		try
		{
			loaded[search] = boost::dll::import<const Symbol(std::vector<Symbol>, const Token *, Hash &)>(rdir::findFile(currentDir, libname, token), fname);
		}
		catch (const boost::wrapexcept<boost::system::system_error> &e)
		{
			throw RuotaError((boost::format("Error loading `%1%`: %2%") % search % e.what()).str(), *token);
		}
	}
} // namespace rlib

#endif