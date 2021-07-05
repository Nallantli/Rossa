#include "../main/rossa/rossa.h"
#include "../main/rossa/symbol/symbol.h"
#include "../main/rossa/function/function.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>

namespace lib_SDL
{
	struct capture_t
	{
		const symbol_t f;
		const token_t token;
		trace_t stack_trace;
		const symbol_t data;

		capture_t(const symbol_t &f, const token_t &token, const trace_t &stack_trace, const symbol_t &data)
			: f{f}, token{token}, stack_trace{stack_trace}, data{data}
		{
		}
	};

	void hint_callback(
		void *userdata,
		const char *name,
		const char *oldValue,
		const char *newValue)
	{
		capture_t *capture = reinterpret_cast<capture_t *>(userdata);
		std::vector<symbol_t> v = {
			capture->data,
			symbol_t::String(name),
			symbol_t::String(oldValue),
			symbol_t::String(newValue)};
		capture->f.call(v, &capture->token, capture->stack_trace);
	}
};

ROSSA_EXT_SIG(_lib_Init, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(SDL_Init(args[0].getNumber(token, stack_trace).getLong())));
}

ROSSA_EXT_SIG(_lib_InitSubSystem, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(SDL_InitSubSystem(args[0].getNumber(token, stack_trace).getLong())));
}

ROSSA_EXT_SIG(_lib_Quit, args, token, hash, stack_trace)
{
	SDL_Quit();
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_QuitSubSystem, args, token, hash, stack_trace)
{
	SDL_QuitSubSystem(args[0].getNumber(token, stack_trace).getLong());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_SetMainReady, args, token, hash, stack_trace)
{
	SDL_SetMainReady();
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_WasInit, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(SDL_WasInit(args[0].getNumber(token, stack_trace).getLong())));
}

ROSSA_EXT_SIG(_lib_AddHintCallback, args, token, hash, stack_trace)
{
	SDL_AddHintCallback(
		args[0].getString(token, stack_trace).c_str(),
		&lib_SDL::hint_callback,
		new lib_SDL::capture_t(
			args[1],
			*token,
			stack_trace,
			args[2]));
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_DelHintCallback, args, token, hash, stack_trace)
{
	SDL_DelHintCallback(
		args[0].getString(token, stack_trace).c_str(),
		&lib_SDL::hint_callback,
		new lib_SDL::capture_t(
			args[1],
			*token,
			stack_trace,
			args[2]));
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_ClearHints, args, token, hash, stack_trace)
{
	SDL_ClearHints();
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_GetHint, args, token, hash, stack_trace)
{
	return symbol_t::String(SDL_GetHint(args[0].getString(token, stack_trace).c_str()));
}

ROSSA_EXT_SIG(_lib_GetHintBoolean, args, token, hash, stack_trace)
{
	return symbol_t::Boolean(SDL_GetHintBoolean(args[0].getString(token, stack_trace).c_str(), args[1].getBool(token, stack_trace) ? SDL_TRUE : SDL_FALSE) == SDL_TRUE);
}

ROSSA_EXT_SIG(_lib_SetHint, args, token, hash, stack_trace)
{
	return symbol_t::Boolean(SDL_SetHint(args[0].getString(token, stack_trace).c_str(), args[1].getString(token, stack_trace).c_str()) == SDL_TRUE);
}

ROSSA_EXT_SIG(_lib_SetHintWithPriority, args, token, hash, stack_trace)
{
	return symbol_t::Boolean(SDL_SetHintWithPriority(args[0].getString(token, stack_trace).c_str(), args[1].getString(token, stack_trace).c_str(), static_cast<SDL_HintPriority>(args[2].getNumber(token, stack_trace).getLong())) == SDL_TRUE);
}

ROSSA_EXT_SIG(_lib_ClearError, args, token, hash, stack_trace)
{
	SDL_ClearError();
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_GetError, args, token, hash, stack_trace)
{
	return symbol_t::String(SDL_GetError());
}

ROSSA_EXT_SIG(_lib_SetError, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(SDL_SetError(args[0].getString(token, stack_trace).c_str())));
}

ROSSA_EXT_SIG(_lib_Log, args, token, hash, stack_trace)
{
	SDL_Log(args[0].getString(token, stack_trace).c_str());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogCritical, args, token, hash, stack_trace)
{
	SDL_LogCritical(args[0].getNumber(token, stack_trace).getLong(), args[1].getString(token, stack_trace).c_str());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogDebug, args, token, hash, stack_trace)
{
	SDL_LogDebug(args[0].getNumber(token, stack_trace).getLong(), args[1].getString(token, stack_trace).c_str());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogError, args, token, hash, stack_trace)
{
	SDL_LogError(args[0].getNumber(token, stack_trace).getLong(), args[1].getString(token, stack_trace).c_str());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogGetPriority, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(SDL_LogGetPriority(args[0].getNumber(token, stack_trace).getLong())));
}

ROSSA_EXT_SIG(_lib_LogInfo, args, token, hash, stack_trace)
{
	SDL_LogInfo(args[0].getNumber(token, stack_trace).getLong(), args[1].getString(token, stack_trace).c_str());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogMessage, args, token, hash, stack_trace)
{
	SDL_LogMessage(args[0].getNumber(token, stack_trace).getLong(), static_cast<SDL_LogPriority>(args[1].getNumber(token, stack_trace).getLong()), args[2].getString(token, stack_trace).c_str());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogResetPriorities, args, token, hash, stack_trace)
{
	SDL_LogResetPriorities();
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogSetAllPriority, args, token, hash, stack_trace)
{
	SDL_LogSetAllPriority(static_cast<SDL_LogPriority>(args[0].getNumber(token, stack_trace).getLong()));
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogSetPriority, args, token, hash, stack_trace)
{
	SDL_LogSetPriority(args[0].getNumber(token, stack_trace).getLong(), static_cast<SDL_LogPriority>(args[1].getNumber(token, stack_trace).getLong()));
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogVerbose, args, token, hash, stack_trace)
{
	SDL_LogVerbose(args[0].getNumber(token, stack_trace).getLong(), args[1].getString(token, stack_trace).c_str());
	return symbol_t();
}

ROSSA_EXT_SIG(_lib_LogWarn, args, token, hash, stack_trace)
{
	SDL_LogWarn(args[0].getNumber(token, stack_trace).getLong(), args[1].getString(token, stack_trace).c_str());
	return symbol_t();
}

EXPORT_FUNCTIONS(lib_SDL)
{
	ADD_EXT(_lib_Init);
	ADD_EXT(_lib_InitSubSystem);
	ADD_EXT(_lib_Quit);
	ADD_EXT(_lib_QuitSubSystem);
	ADD_EXT(_lib_SetMainReady);
	ADD_EXT(_lib_WasInit);
	ADD_EXT(_lib_ClearHints);
	ADD_EXT(_lib_GetHint);
	ADD_EXT(_lib_GetHintBoolean);
	ADD_EXT(_lib_SetHint);
	ADD_EXT(_lib_SetHintWithPriority);
	ADD_EXT(_lib_ClearError);
	ADD_EXT(_lib_GetError);
	ADD_EXT(_lib_SetError);
	ADD_EXT(_lib_Log);
	ADD_EXT(_lib_LogCritical);
	ADD_EXT(_lib_LogDebug);
	ADD_EXT(_lib_LogError);
	ADD_EXT(_lib_LogGetPriority);
	ADD_EXT(_lib_LogInfo);
	ADD_EXT(_lib_LogMessage);
	ADD_EXT(_lib_LogResetPriorities);
	ADD_EXT(_lib_LogSetAllPriority);
	ADD_EXT(_lib_LogSetPriority);
	ADD_EXT(_lib_LogVerbose);
	ADD_EXT(_lib_LogWarn);
	ADD_EXT(_lib_AddHintCallback);
	ADD_EXT(_lib_DelHintCallback);
}