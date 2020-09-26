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

	std::cout << "Ruota " << _RUOTA_VERSION_ << " Interpreter\n";
	try
	{
		wrapper.parseCode("load \"std.ruo\";", boost::filesystem::current_path() / "nil");
		std::cout << "Standard Library Loaded\n";
	}
	catch (const std::runtime_error &e)
	{
		std::cout << "Failed to load Standard Library: " << std::string(e.what()) << "\n";
	}

	while (true)
	{
		std::cout << "> ";
		std::string line;
		std::getline(std::cin, line);
		try
		{
			auto value = wrapper.parseCode(line, boost::filesystem::current_path() / "nil");
			if (value.getValueType() == VECTOR)
			{
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
					std::cout << "\t" << value.getVector()[0].toString() << "\n";
				}
			}
		}
		catch (const std::runtime_error &e)
		{
			std::cout << e.what() << "\n";
		}
	}

	return 0;
}