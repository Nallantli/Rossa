#pragma once

#include "../../bin/include/Rossa.h"

#ifdef _STATIC_
#undef EXPORT_FUNCTIONS
#define EXPORT_FUNCTIONS(name) void name##_rossaExportFunctions(std::map<std::string, rossa::extf_t> &fmap)
#endif

EXPORT_FUNCTIONS(libfs);