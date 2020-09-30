#include <iostream>
#include <fstream>
#include <stdexcept>

#include "ruota/Node.h"
#include "ruota/Ruota.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
char getch_n()
{
	return getch();
}
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>

int getch_n()
{
	struct termios oldattr, newattr;
	int ch;
	tcgetattr(0, &oldattr);
	newattr = oldattr;
	newattr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &newattr);
	ch = getchar();
	tcsetattr(0, TCSANOW, &oldattr);
	return (ch);
}
#endif

void printc(const std::string &s, TextColor color)
{
	std::cout << "\033[" << color << "m" << s << "\033[0m";
}

std::pair<std::map<std::string, std::string>, std::vector<std::string>> parseOptions(int argc, char const *argv[])
{
	std::map<std::string, std::string> options = {
		{"tree", "false"},
		{"version", "false"},
		{"std", "true"},
		{"file", ""}};
	std::vector<std::string> passed;

	bool flag = false;

	for (int i = 1; i < argc; i++)
	{
		if (flag == false && argv[i][0] == '-')
		{
			if (std::string(argv[i]) == "--tree" || std::string(argv[i]) == "-t")
			{
				options["tree"] = "true";
			}
			else if (std::string(argv[i]) == "--no-std" || std::string(argv[i]) == "-ns")
			{
				options["std"] = "false";
			}
			else if (std::string(argv[i]) == "--version" || std::string(argv[i]) == "-v")
			{
				options["version"] = "true";
			}
			else
			{
				std::cerr << "Unknown command line option: " << argv[i] << "\n";
				exit(1);
			}
		}
		else if (flag == false)
		{
			flag = true;
			options["file"] = argv[i];
		}
		else
		{
			passed.push_back(argv[i]);
		}
	}

	return {options, passed};
}

void printError(const std::runtime_error &e)
{
	printc(e.what(), RED_TEXT);
	std::cout << "\n";
	size_t j = 0;
	while (!Ruota::stack_trace.empty())
	{
		if (j++ > 10)
		{
			printc(" ... (" + std::to_string(Ruota::stack_trace.size()) + " more) ...\n", MAGENTA_TEXT);
			Ruota::stack_trace.clear();
			break;
		}
		auto f = Ruota::stack_trace.back();
		printc(" ^ ", MAGENTA_TEXT);
		std::cout << "\033[" << BRIGHT_BLACK_TEXT << "m";
		if (f.getParent()->getName() != "")
			std::cout << f.getParent()->getName() << ".";
		std::cout << hash.deHash(f.getKey()) << "(\033[0m";
		size_t i = 0;
		for (auto &p : f.getParams())
		{
			if (i > 0)
				std::cout << ", ";
			i++;
			switch (p.first)
			{
			case TOK_FINAL:
				std::cout << "\033[" << MAGENTA_TEXT << "mfinal\033[0m ";
				break;
			case TOK_REF:
				std::cout << "\033[" << MAGENTA_TEXT << "mref\033[0m ";
			default:
				break;
			}
			std::cout << hash.deHash(p.second);
		}
		std::cout << "\033[" << BRIGHT_BLACK_TEXT << "m)\033[0m\n";
		Ruota::stack_trace.pop_back();
	}
}

void moveback(std::string &code, int c)
{
	for (int i = 0; i < c; i++)
	{
		if (code.size() > 0)
		{
			auto back = code.back();
			code.pop_back();
			if (back == '\n')
			{
				size_t i = 0;
				while (i < code.size() && code[code.size() - i - 1] != '\n')
					i++;
				if (code.size() == i)
				{
					std::cout << "\033[2D  \033[2D\033[1A> ";
				}
				else
				{
					std::cout << "\033[2D  \033[2D\033[1A";
					printc("└ ", BRIGHT_YELLOW_TEXT);
				}
				if (i > 0)
					std::cout << "\033[" << i << "C";
			}
			else
			{
				std::cout << "\033[1D \033[1D";
			}
		}
	}
}

int main(int argc, char const *argv[])
{
	auto parsed = parseOptions(argc, argv);
	auto options = parsed.first;

	if (options["version"] == "true") {
		std::cout << _RUOTA_VERSION_LONG_ << "\n";
		return 0;
	}

#ifdef _WIN32
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
#endif
	Ruota wrapper(parsed.second);
	bool tree = options["tree"] == "true";

	if (options["file"] == "")
	{
		std::cout << "Ruota " << _RUOTA_VERSION_ << " Interpreter\n";

		if (options["std"] == "true")
		{
			try
			{
				wrapper.runCode(wrapper.compileCode("load \"std.ruo\";", boost::filesystem::current_path() / "nil", false));
				std::cout << "Standard Library Loaded\n";
			}
			catch (const std::runtime_error &e)
			{
				std::cout << "Failed to load Standard Library: " << std::string(e.what()) << "\n";
			}
		}
		else
		{
			std::cout << "Option --no-std (-ns) used; Standard Library not loaded\n";
		}

		bool flag = true;
		std::string code = "";
		while (true)
		{
			if (flag)
				std::cout << "> ";
			char c;
			while ((c = getch_n()) != '\r')
			{
				if (c == '\n')
					break;
				else if (c == 3)
					exit(0);
				else if (c == '\t')
				{
					code += "    ";
					std::cout << "    ";
				}
				else if (c == 27)
				{
					while (!code.empty())
						moveback(code, 1);
				}
				else if (c == '\b' | c == 127)
					moveback(code, 1);
				else
				{
					code += c;
					std::cout << c;
				}
			}

			std::shared_ptr<Instruction> comp;
			try
			{
				flag = true;
				comp = wrapper.compileCode(code, boost::filesystem::current_path() / "nil", tree);
			}
			catch (const std::runtime_error &e)
			{
				flag = false;
				std::cout << "\n";

				if (code.find('\n') != std::string::npos)
				{
					std::cout << "\033[1A";
					printc("│ ", BRIGHT_YELLOW_TEXT);
					std::cout << "\033[2D\033[1B";
				}
				printc("└ ", BRIGHT_YELLOW_TEXT);

				code += "\n";
			}

			if (flag)
			{
				std::cout << "\n";
				code = "";
				try
				{
					auto value = wrapper.runCode(comp);
					if (value.getValueType() == VECTOR)
					{
						if (value.vectorSize() != 1)
						{
							int i = 0;
							for (auto &e : value.getVector(NULL))
							{
								printc("\t(" + std::to_string(i) + ")\t", CYAN_TEXT);
								std::cout << e.toString(NULL) << "\n";
								i++;
							}
						}
						else
						{
							std::cout << "\t" << value.getVector(NULL)[0].toString(NULL) << "\n";
						}
					}
				}
				catch (const std::runtime_error &e)
				{
					printError(e);
				}
			}
		}
	}
	else
	{
		std::ifstream file;
		file.open(options["file"]);
		if (!file.is_open())
		{
			std::cerr << "Cannot find path to file: " << options["file"] << "\n";
			return 1;
		}

		std::string content = "";
		std::string line;
		while (std::getline(file, line))
			content += line + "\n";

		try
		{
			if (options["std"] == "true")
				wrapper.runCode(wrapper.compileCode("load \"std.ruo\";", boost::filesystem::current_path() / "nil", false));
			wrapper.runCode(wrapper.compileCode(content, boost::filesystem::path(options["file"]), tree));
		}
		catch (const std::runtime_error &e)
		{
			printError(e);
			return 1;
		}
	}

	return 0;
}