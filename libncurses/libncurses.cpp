#include "../main/rossa/rossa.h"
#include "../main/rossa/symbol/symbol.h"
#include "../main/rossa/function/function.h"

#ifdef __unix__
#include <ncurses.h>
#else
#include <ncurses/ncurses.h>
#endif

ROSSA_EXT_SIG(_initscr, args, token, hash, stack_trace)
{
	std::shared_ptr<WINDOW> win = std::shared_ptr<WINDOW>(initscr());
	return symbol_t::Pointer(win);
}

ROSSA_EXT_SIG(_newwin, args, token, hash, stack_trace)
{
	std::shared_ptr<WINDOW> win = std::shared_ptr<WINDOW>(newwin(
		args[0].getNumber(token, stack_trace).getLong(),
		args[1].getNumber(token, stack_trace).getLong(),
		args[2].getNumber(token, stack_trace).getLong(),
		args[3].getNumber(token, stack_trace).getLong()
	));
	return symbol_t::Pointer(win);
}

ROSSA_EXT_SIG(_box, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	return symbol_t::Number(number_t::Long(box(win,
		args[1].getNumber(token, stack_trace).getLong(),
		args[2].getNumber(token, stack_trace).getLong()
	)));
}

ROSSA_EXT_SIG(_wborder, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	return symbol_t::Number(number_t::Long(wborder(win,
		args[1].getNumber(token, stack_trace).getLong(),
		args[2].getNumber(token, stack_trace).getLong(),
		args[3].getNumber(token, stack_trace).getLong(),
		args[4].getNumber(token, stack_trace).getLong(),
		args[5].getNumber(token, stack_trace).getLong(),
		args[6].getNumber(token, stack_trace).getLong(),
		args[7].getNumber(token, stack_trace).getLong(),
		args[8].getNumber(token, stack_trace).getLong()
	)));
}

ROSSA_EXT_SIG(_delwin, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	return symbol_t::Number(number_t::Long(delwin(win)));
}

ROSSA_EXT_SIG(_wrefresh, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	return symbol_t::Number(number_t::Long(wrefresh(win)));
}

ROSSA_EXT_SIG(_wprintw, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	const std::string s = args[1].getString(token, stack_trace);
	return symbol_t::Number(number_t::Long(wprintw(win, s.c_str())));
}

ROSSA_EXT_SIG(_waddch, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	const char c = args[1].getNumber(token, stack_trace).getLong();
	return symbol_t::Number(number_t::Long(waddch(win, c)));
}

ROSSA_EXT_SIG(_wgetch, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	return symbol_t::Number(number_t::Long(wgetch(win)));
}

ROSSA_EXT_SIG(_endwin, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(endwin()));
}

ROSSA_EXT_SIG(_raw, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(raw()));
}

ROSSA_EXT_SIG(_noraw, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(noraw()));
}

ROSSA_EXT_SIG(_cbreak, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(cbreak()));
}

ROSSA_EXT_SIG(_nocbreak, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(nocbreak()));
}

ROSSA_EXT_SIG(_echo, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(echo()));
}

ROSSA_EXT_SIG(_noecho, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(noecho()));
}

ROSSA_EXT_SIG(_keypad, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	return symbol_t::Number(number_t::Long(keypad(win, args[1].getBool(token, stack_trace) ? TRUE : FALSE)));
}

ROSSA_EXT_SIG(_halfdelay, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(halfdelay(args[0].getNumber(token, stack_trace).getLong())));
}

ROSSA_EXT_SIG(_typeahead, args, token, hash, stack_trace)
{
	return symbol_t::Number(number_t::Long(typeahead(args[0].getNumber(token, stack_trace).getLong())));
}

ROSSA_EXT_SIG(_wmove, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	const int y = args[1].getNumber(token, stack_trace).getLong();
	const int x = args[2].getNumber(token, stack_trace).getLong();
	return symbol_t::Number(number_t::Long(wmove(win, y, x)));
}

ROSSA_EXT_SIG(_wattron, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	const int a = args[1].getNumber(token, stack_trace).getLong();
	return symbol_t::Number(number_t::Long(wattron(win, a)));
}

ROSSA_EXT_SIG(_wattroff, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	const int a = args[1].getNumber(token, stack_trace).getLong();
	return symbol_t::Number(number_t::Long(wattroff(win, a)));
}

ROSSA_EXT_SIG(_wattrset, args, token, hash, stack_trace)
{
	auto win = COERCE_PTR(args[0].getPointer(token, stack_trace), WINDOW);
	const int a = args[1].getNumber(token, stack_trace).getLong();
	return symbol_t::Number(number_t::Long(wattrset(win, a)));
}

ROSSA_EXT_SIG(_fetch_constants, args, token, hash, stack_trace)
{
	std::map<const std::string, const symbol_t> m = {
		{"NORMAL", symbol_t::Number(number_t::Long(A_NORMAL)) },
		{"ATTRIBUTES", symbol_t::Number(number_t::Long(A_ATTRIBUTES))},
		{"CHARTEXT", symbol_t::Number(number_t::Long(A_CHARTEXT))},
		{"COLOR", symbol_t::Number(number_t::Long(A_COLOR))},
		{"STANDOUT", symbol_t::Number(number_t::Long(A_STANDOUT))},
		{"UNDERLINE", symbol_t::Number(number_t::Long(A_UNDERLINE))},
		{"REVERSE", symbol_t::Number(number_t::Long(A_REVERSE))},
		{"BLINK", symbol_t::Number(number_t::Long(A_BLINK))},
		{"DIM", symbol_t::Number(number_t::Long(A_DIM))},
		{"BOLD", symbol_t::Number(number_t::Long(A_BOLD))},
		{"ALTCHARSET", symbol_t::Number(number_t::Long(A_ALTCHARSET))},
		{"INVIS", symbol_t::Number(number_t::Long(A_INVIS))},
		{"PROTECT", symbol_t::Number(number_t::Long(A_PROTECT))},
		{"HORIZONTAL", symbol_t::Number(number_t::Long(A_HORIZONTAL))},
		{"LEFT", symbol_t::Number(number_t::Long(A_LEFT))},
		{"LOW", symbol_t::Number(number_t::Long(A_LOW))},
		{"RIGHT", symbol_t::Number(number_t::Long(A_RIGHT))},
		{"TOP", symbol_t::Number(number_t::Long(A_TOP))},
		{"VERTICAL", symbol_t::Number(number_t::Long(A_VERTICAL))},
		{"ITALIC", symbol_t::Number(number_t::Long(A_ITALIC))}
	};
	return symbol_t::Dictionary(m);
}

EXPORT_FUNCTIONS(libncurses)
{
	ADD_EXT(_initscr);
	ADD_EXT(_typeahead);
	ADD_EXT(_newwin);
	ADD_EXT(_delwin);
	ADD_EXT(_box);
	ADD_EXT(_wborder);
	ADD_EXT(_wrefresh);
	ADD_EXT(_wprintw);
	ADD_EXT(_wgetch);
	ADD_EXT(_endwin);
	ADD_EXT(_raw);
	ADD_EXT(_noraw);
	ADD_EXT(_cbreak);
	ADD_EXT(_nocbreak);
	ADD_EXT(_echo);
	ADD_EXT(_noecho);
	ADD_EXT(_keypad);
	ADD_EXT(_halfdelay);
	ADD_EXT(_waddch);
	ADD_EXT(_wmove);
	ADD_EXT(_fetch_constants);
	ADD_EXT(_wattron);
	ADD_EXT(_wattroff);
	ADD_EXT(_wattrset);
}