#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ruota/Node.h"
#include "ruota/Ruota.h"

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