#ifndef GLOBAL_H
#define GLOBAL_H

#include "../rossa.h"
#include "../rossa_error/rossa_error.h"

#include "../../mediator/mediator.h"

namespace dir
{
	extern std::vector<std::filesystem::path> loaded;

	const std::filesystem::path findFile(const std::filesystem::path &, const std::string &, const token_t *token);
}

namespace global
{
	extern std::map<std::string, std::map<std::string, extf_t>> loaded;

	void loadLibrary(const std::filesystem::path &, const std::string &, const token_t *token);
	extf_t loadFunction(const std::string &, const std::string &, const token_t *);

	const std::string getTypeString(const aug_type_t &);

	const mediator_t convertToMediator(const symbol_t &, const token_t *, trace_t &);
	const symbol_t convertToSymbol(const mediator_t &);
}

#endif