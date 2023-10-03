#include "../main/mediator/mediator.h"

#ifdef __unix__
#include <ncurses.h>
#else
#include <ncurses/ncurses.h>
#endif

ROSSA_EXT_SIG(_initscr, args)
{
	std::shared_ptr<WINDOW> win = std::shared_ptr<WINDOW>(initscr());
	return MAKE_POINTER(win);
}

ROSSA_EXT_SIG(_newwin, args)
{
	std::shared_ptr<WINDOW> win = std::shared_ptr<WINDOW>(newwin(
		COERCE_NUMBER(args[0]).getLong(),
		COERCE_NUMBER(args[1]).getLong(),
		COERCE_NUMBER(args[2]).getLong(),
		COERCE_NUMBER(args[3]).getLong()));
	return MAKE_POINTER(win);
}

ROSSA_EXT_SIG(_box, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	return MAKE_NUMBER(number_t::Long(box(win,
										  COERCE_NUMBER(args[1]).getLong(),
										  COERCE_NUMBER(args[2]).getLong())));
}

ROSSA_EXT_SIG(_wborder, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	return MAKE_NUMBER(number_t::Long(wborder(win,
											  COERCE_NUMBER(args[1]).getLong(),
											  COERCE_NUMBER(args[2]).getLong(),
											  COERCE_NUMBER(args[3]).getLong(),
											  COERCE_NUMBER(args[4]).getLong(),
											  COERCE_NUMBER(args[5]).getLong(),
											  COERCE_NUMBER(args[6]).getLong(),
											  COERCE_NUMBER(args[7]).getLong(),
											  COERCE_NUMBER(args[8]).getLong())));
}

ROSSA_EXT_SIG(_delwin, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	return MAKE_NUMBER(number_t::Long(delwin(win)));
}

ROSSA_EXT_SIG(_wrefresh, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	return MAKE_NUMBER(number_t::Long(wrefresh(win)));
}

ROSSA_EXT_SIG(_wprintw, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	const std::string s = COERCE_STRING(args[1]);
	return MAKE_NUMBER(number_t::Long(wprintw(win, s.c_str())));
}

ROSSA_EXT_SIG(_waddch, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	const char c = COERCE_NUMBER(args[1]).getLong();
	return MAKE_NUMBER(number_t::Long(waddch(win, c)));
}

ROSSA_EXT_SIG(_wgetch, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	return MAKE_NUMBER(number_t::Long(wgetch(win)));
}

ROSSA_EXT_SIG(_endwin, args)
{
	return MAKE_NUMBER(number_t::Long(endwin()));
}

ROSSA_EXT_SIG(_raw, args)
{
	return MAKE_NUMBER(number_t::Long(raw()));
}

ROSSA_EXT_SIG(_noraw, args)
{
	return MAKE_NUMBER(number_t::Long(noraw()));
}

ROSSA_EXT_SIG(_cbreak, args)
{
	return MAKE_NUMBER(number_t::Long(cbreak()));
}

ROSSA_EXT_SIG(_nocbreak, args)
{
	return MAKE_NUMBER(number_t::Long(nocbreak()));
}

ROSSA_EXT_SIG(_echo, args)
{
	return MAKE_NUMBER(number_t::Long(echo()));
}

ROSSA_EXT_SIG(_noecho, args)
{
	return MAKE_NUMBER(number_t::Long(noecho()));
}

ROSSA_EXT_SIG(_keypad, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	return MAKE_NUMBER(number_t::Long(keypad(win, COERCE_BOOLEAN(args[1]) ? TRUE : FALSE)));
}

ROSSA_EXT_SIG(_halfdelay, args)
{
	return MAKE_NUMBER(number_t::Long(halfdelay(COERCE_NUMBER(args[0]).getLong())));
}

ROSSA_EXT_SIG(_typeahead, args)
{
	return MAKE_NUMBER(number_t::Long(typeahead(COERCE_NUMBER(args[0]).getLong())));
}

ROSSA_EXT_SIG(_wmove, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	const int y = COERCE_NUMBER(args[1]).getLong();
	const int x = COERCE_NUMBER(args[2]).getLong();
	return MAKE_NUMBER(number_t::Long(wmove(win, y, x)));
}

ROSSA_EXT_SIG(_wattron, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	const int a = COERCE_NUMBER(args[1]).getLong();
	return MAKE_NUMBER(number_t::Long(wattron(win, a)));
}

ROSSA_EXT_SIG(_wattroff, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	const int a = COERCE_NUMBER(args[1]).getLong();
	return MAKE_NUMBER(number_t::Long(wattroff(win, a)));
}

ROSSA_EXT_SIG(_wattrset, args)
{
	auto win = COERCE_POINTER(args[0], WINDOW).get();
	const int a = COERCE_NUMBER(args[1]).getLong();
	return MAKE_NUMBER(number_t::Long(wattrset(win, a)));
}

ROSSA_EXT_SIG(_fetch_constants, args)
{
	std::map<const std::string, const mediator_t> m = {
		{"NORMAL", MAKE_NUMBER(number_t::Long(A_NORMAL))},
		{"ATTRIBUTES", MAKE_NUMBER(number_t::Long(A_ATTRIBUTES))},
		{"CHARTEXT", MAKE_NUMBER(number_t::Long(A_CHARTEXT))},
		{"COLOR", MAKE_NUMBER(number_t::Long(A_COLOR))},
		{"STANDOUT", MAKE_NUMBER(number_t::Long(A_STANDOUT))},
		{"UNDERLINE", MAKE_NUMBER(number_t::Long(A_UNDERLINE))},
		{"REVERSE", MAKE_NUMBER(number_t::Long(A_REVERSE))},
		{"BLINK", MAKE_NUMBER(number_t::Long(A_BLINK))},
		{"DIM", MAKE_NUMBER(number_t::Long(A_DIM))},
		{"BOLD", MAKE_NUMBER(number_t::Long(A_BOLD))},
		{"ALTCHARSET", MAKE_NUMBER(number_t::Long(A_ALTCHARSET))},
		{"INVIS", MAKE_NUMBER(number_t::Long(A_INVIS))},
		{"PROTECT", MAKE_NUMBER(number_t::Long(A_PROTECT))},
		{"HORIZONTAL", MAKE_NUMBER(number_t::Long(A_HORIZONTAL))},
		{"LEFT", MAKE_NUMBER(number_t::Long(A_LEFT))},
		{"LOW", MAKE_NUMBER(number_t::Long(A_LOW))},
		{"RIGHT", MAKE_NUMBER(number_t::Long(A_RIGHT))},
		{"TOP", MAKE_NUMBER(number_t::Long(A_TOP))},
		{"VERTICAL", MAKE_NUMBER(number_t::Long(A_VERTICAL))},
		{"ITALIC", MAKE_NUMBER(number_t::Long(A_ITALIC))}};
	return MAKE_DICTIONARY(m);
}

EXPORT_FUNCTIONS(lib_ncurses)
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