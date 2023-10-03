#include "../main/mediator/mediator.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>

/*
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
			MAKE_STRING(name),
			MAKE_STRING(oldValue),
			MAKE_STRING(newValue)};
		capture->f.call(v, &capture->token, capture->stack_trace);
	}
};
*/

ROSSA_EXT_SIG(_lib_Init, args)
{
	return MAKE_NUMBER(number_t::Long(SDL_Init(COERCE_NUMBER(args[0]).getLong())));
}

ROSSA_EXT_SIG(_lib_InitSubSystem, args)
{
	return MAKE_NUMBER(number_t::Long(SDL_InitSubSystem(COERCE_NUMBER(args[0]).getLong())));
}

ROSSA_EXT_SIG(_lib_Quit, args)
{
	SDL_Quit();
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_QuitSubSystem, args)
{
	SDL_QuitSubSystem(COERCE_NUMBER(args[0]).getLong());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_SetMainReady, args)
{
	SDL_SetMainReady();
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_WasInit, args)
{
	return MAKE_NUMBER(number_t::Long(SDL_WasInit(COERCE_NUMBER(args[0]).getLong())));
}

/*
ROSSA_EXT_SIG(_lib_AddHintCallback, args)
{
	SDL_AddHintCallback(
		COERCE_STRING(args[0]).c_str(),
		&lib_SDL::hint_callback,
		new lib_SDL::capture_t(
			args[1],
			*token,
			stack_trace,
			args[2]));
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_DelHintCallback, args)
{
	SDL_DelHintCallback(
		COERCE_STRING(args[0]).c_str(),
		&lib_SDL::hint_callback,
		new lib_SDL::capture_t(
			args[1],
			*token,
			stack_trace,
			args[2]));
	return mediator_t();
}
*/

ROSSA_EXT_SIG(_lib_ClearHints, args)
{
	SDL_ClearHints();
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_GetHint, args)
{
	return MAKE_STRING(SDL_GetHint(COERCE_STRING(args[0]).c_str()));
}

ROSSA_EXT_SIG(_lib_GetHintBoolean, args)
{
	return MAKE_BOOLEAN(SDL_GetHintBoolean(COERCE_STRING(args[0]).c_str(), COERCE_BOOLEAN(args[1]) ? SDL_TRUE : SDL_FALSE) == SDL_TRUE);
}

ROSSA_EXT_SIG(_lib_SetHint, args)
{
	return MAKE_BOOLEAN(SDL_SetHint(COERCE_STRING(args[0]).c_str(), COERCE_STRING(args[1]).c_str()) == SDL_TRUE);
}

ROSSA_EXT_SIG(_lib_SetHintWithPriority, args)
{
	return MAKE_BOOLEAN(SDL_SetHintWithPriority(COERCE_STRING(args[0]).c_str(), COERCE_STRING(args[1]).c_str(), static_cast<SDL_HintPriority>(COERCE_NUMBER(args[2]).getLong())) == SDL_TRUE);
}

ROSSA_EXT_SIG(_lib_ClearError, args)
{
	SDL_ClearError();
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_GetError, args)
{
	return MAKE_STRING(SDL_GetError());
}

ROSSA_EXT_SIG(_lib_SetError, args)
{
	return MAKE_NUMBER(number_t::Long(SDL_SetError(COERCE_STRING(args[0]).c_str())));
}

ROSSA_EXT_SIG(_lib_Log, args)
{
	SDL_Log(COERCE_STRING(args[0]).c_str());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogCritical, args)
{
	SDL_LogCritical(COERCE_NUMBER(args[0]).getLong(), COERCE_STRING(args[1]).c_str());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogDebug, args)
{
	SDL_LogDebug(COERCE_NUMBER(args[0]).getLong(), COERCE_STRING(args[1]).c_str());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogError, args)
{
	SDL_LogError(COERCE_NUMBER(args[0]).getLong(), COERCE_STRING(args[1]).c_str());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogGetPriority, args)
{
	return MAKE_NUMBER(number_t::Long(SDL_LogGetPriority(COERCE_NUMBER(args[0]).getLong())));
}

ROSSA_EXT_SIG(_lib_LogInfo, args)
{
	SDL_LogInfo(COERCE_NUMBER(args[0]).getLong(), COERCE_STRING(args[1]).c_str());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogMessage, args)
{
	SDL_LogMessage(COERCE_NUMBER(args[0]).getLong(), static_cast<SDL_LogPriority>(COERCE_NUMBER(args[1]).getLong()), COERCE_STRING(args[2]).c_str());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogResetPriorities, args)
{
	SDL_LogResetPriorities();
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogSetAllPriority, args)
{
	SDL_LogSetAllPriority(static_cast<SDL_LogPriority>(COERCE_NUMBER(args[0]).getLong()));
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogSetPriority, args)
{
	SDL_LogSetPriority(COERCE_NUMBER(args[0]).getLong(), static_cast<SDL_LogPriority>(COERCE_NUMBER(args[1]).getLong()));
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogVerbose, args)
{
	SDL_LogVerbose(COERCE_NUMBER(args[0]).getLong(), COERCE_STRING(args[1]).c_str());
	return mediator_t();
}

ROSSA_EXT_SIG(_lib_LogWarn, args)
{
	SDL_LogWarn(COERCE_NUMBER(args[0]).getLong(), COERCE_STRING(args[1]).c_str());
	return mediator_t();
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
	// ADD_EXT(_lib_AddHintCallback);
	// ADD_EXT(_lib_DelHintCallback);
}