#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ruota/Lexer.h"
#include "ruota/Node.h"
#include "ruota/Ruota.h"

enum TextColor
{
	BLACK_TEXT = 30,
	RED_TEXT = 31,
	GREEN_TEXT = 32,
	YELLOW_TEXT = 33,
	BLUE_TEXT = 34,
	MAGENTA_TEXT = 35,
	CYAN_TEXT = 36,
	WHITE_TEXT = 37,
	BRIGHT_BLACK_TEXT = 90,
	BRIGHT_RED_TEXT = 91,
	BRIGHT_GREEN_TEXT = 92,
	BRIGHT_YELLOW_TEXT = 93,
	BRIGHT_BLUE_TEXT = 94,
	BRIGHT_MAGENTA_TEXT = 95,
	BRIGHT_CYAN_TEXT = 96,
	BRIGHT_WHITE_TEXT = 97
};

void printc(const std::string &s, TextColor color)
{
	std::cout << "\033[" << color << "m" << s << "\033[0m";
}

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
#endif
	Ruota wrapper;

	while (true)
	{
		std::cout << ">>> ";
		std::string line;
		std::getline(std::cin, line);
		try
		{
			auto value = wrapper.parseCode(line);
			if (value.vectorSize() != 1)
			{
				int i = 0;
				for (auto &e : value.getVector())
				{
					printc("\t(" + std::to_string(i) + ")\t", CYAN_TEXT);
					std::cout << e.toString() << "\n";
					i++;
				}
			}
			else
			{
				std::cout << "\t" << value.indexVector(0).toString() << "\n";
			}
		}
		catch (const std::runtime_error &e)
		{
			printc(e.what(), BRIGHT_RED_TEXT);
			std::cout << "\n";
		}
	}

	return 0;
}