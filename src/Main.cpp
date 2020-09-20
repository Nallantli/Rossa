#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ruota/Lexer.h"
#include "ruota/Node.h"
#include "ruota/Ruota.h"

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
#endif
	Ruota wrapper;

	while (true)
	{
		std::cout << "?> ";
		std::string line;
		std::getline(std::cin, line);
		wrapper.parseCode(line);
	}

	return 0;
}