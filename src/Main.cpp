#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ruota/Lexer.hpp"
#include "ruota/Node.hpp"
#include "ruota/Ruota.hpp"

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
#endif
	std::string test_code = R"RU0(
		extern _puts in "libsystem";

		def print(s) {
			extern_call libsystem._puts(s);
		}

		var s = "こんにちは";
		var f = @(x, y) {
			return x + y;
		};

		def g(q, a) {
			return q(a[0], a[1]);
		}

		g(f, [1,2]);
	)RU0";

	Ruota wrapper;

	wrapper.parseCode(test_code);

	while (true)
	{
		std::cout << "?> ";
		std::string line;
		std::getline(std::cin, line);
		wrapper.parseCode(line);
	}

	return 0;
}