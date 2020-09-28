#include <iostream>
#include <fstream>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ruota/Node.h"
#include "ruota/Ruota.h"

void printc(const std::string &s, TextColor color)
{
	std::cout << "\033[" << color << "m" << s << "\033[0m";
}

std::map<std::string, std::string> parseOptions(int argc, char const *argv[])
{
	std::map<std::string, std::string> options = {
		{"tree", "false"},
		{"std", "true"},
		{"file", ""}};

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (std::string(argv[i]) == "--tree" || std::string(argv[i]) == "-t")
			{
				options["tree"] = "true";
			}
			else if (std::string(argv[i]) == "--no-std" || std::string(argv[i]) == "-ns")
			{
				options["std"] = "false";
			}
			else
			{
				std::cerr << "Unknown command line option: " << argv[i] << "\n";
				exit(1);
			}
		}
		else
		{
			if (options["file"] != "")
			{
				std::cerr << "File already given: " << options["file"] << "\n";
				exit(1);
			}
			options["file"] = argv[i];
		}
	}

	return options;
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

int main(int argc, char const *argv[])
{
	auto options = parseOptions(argc, argv);

#ifdef _WIN32
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
#endif
	Ruota wrapper;
	bool tree = options["tree"] == "true";

	if (options["file"] == "")
	{
		std::cout << "Ruota " << _RUOTA_VERSION_ << " Interpreter\n";

		if (options["std"] == "true")
		{
			try
			{
				wrapper.parseCode("load \"std.ruo\";", boost::filesystem::current_path() / "nil", false);
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

		while (true)
		{
			std::cout << "> ";
			std::string line;
			std::getline(std::cin, line);
			try
			{
				auto value = wrapper.parseCode(line, boost::filesystem::current_path() / "nil", tree);
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
				wrapper.parseCode("load \"std.ruo\";", boost::filesystem::current_path() / "nil", false);
			wrapper.parseCode(content, boost::filesystem::path(options["file"]), tree);
		}
		catch (const std::runtime_error &e)
		{
			printError(e);
			return 1;
		}
	}

	return 0;
}