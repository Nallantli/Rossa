#include <iostream>
#include <fstream>

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

void printc(const string &s, TextColor color)
{
	std::cout << "\033[" << color << "m" << s << "\033[0m";
}

std::pair<std::map<string, string>, std::vector<string>> parseOptions(int argc, char const *argv[])
{
	std::map<string, string> options = {
		{"tree", "false"},
		{"version", "false"},
		{"std", "true"},
		{"file", ""} };
	std::vector<string> passed;

	bool flag = false;

	for (int i = 1; i < argc; i++) {
		if (flag == false && argv[i][0] == '-') {
			if (string(argv[i]) == "--tree" || string(argv[i]) == "-t")
				options["tree"] = "true";
			else if (string(argv[i]) == "--no-std" || string(argv[i]) == "-ns")
				options["std"] = "false";
			else if (string(argv[i]) == "--version" || string(argv[i]) == "-v")
				options["version"] = "true";
			else {
				std::cerr << "Unknown command line option: " << argv[i] << "\n";
				exit(1);
			}
		} else if (flag == false) {
			flag = true;
			options["file"] = argv[i];
		} else
			passed.push_back(argv[i]);
	}

	return { options, passed };
}

void printError(const RuotaError &e)
{
	string ret = "\033[" + std::to_string(RED_TEXT) + "m" + e.what() + "\n";

	if (e.getToken().getType() != NULL_TOK) {
		string lineInfoRaw = "<" + e.getToken().getFilename() + ">:" + std::to_string(e.getToken().getLineNumber() + 1) + " | ";
		ret += "\033[" + std::to_string(CYAN_TEXT) + "m<\033[4m" + e.getToken().getFilename() + "\033[0m\033[" + std::to_string(CYAN_TEXT) + "m>:" + std::to_string(e.getToken().getLineNumber() + 1) + " | ";
		ret += "\033[" + std::to_string(MAGENTA_TEXT) + "m" + e.getToken().getLine() + "\n";

		ret += "\033[" + std::to_string(RED_TEXT) + "m";
		for (size_t i = 0; i < e.getToken().getDist() - e.getToken().getValueString().size() + lineInfoRaw.size(); i++)
			ret += " ";
		ret += "^";

		if (e.getToken().getValueString().size() > 0)
			for (size_t i = 0; i < e.getToken().getValueString().size() - 1; i++)
				ret += "~";

		ret += "\033[0m";
	}

	std::cout << ret << "\n";
	size_t j = 0;
	while (!Ruota::stack_trace.empty()) {
		if (j++ > 10) {
			printc((boost::format(_STACK_TRACE_MORE_) % Ruota::stack_trace.size()).str(), MAGENTA_TEXT);
			std::cout << "\n";
			Ruota::stack_trace.clear();
			break;
		}
		auto f = Ruota::stack_trace.back();
		printc(" ^ ", MAGENTA_TEXT);
		std::cout << "\033[" << BRIGHT_BLACK_TEXT << "m";
		if (MAIN_HASH.deHash(f.getParent()->getHashedKey()) != "")
			std::cout << MAIN_HASH.deHash(f.getParent()->getHashedKey()) << ".";
		std::cout << MAIN_HASH.deHash(f.getKey()) << "(\033[0m";
		size_t i = 0;
		for (auto &p : f.getParams()) {
			if (i++ > 0)
				std::cout << ", ";
			switch (p.first) {
				case TOK_REF:
					std::cout << "\033[" << MAGENTA_TEXT << "mref\033[0m ";
					break;
				default:
					break;
			}
			std::cout << MAIN_HASH.deHash(p.second);
		}
		std::cout << "\033[" << BRIGHT_BLACK_TEXT << "m)\033[0m\n";
		Ruota::stack_trace.pop_back();
	}
}

void moveback(string &code, int c)
{
	for (int i = 0; i < c; i++) {
		if (code.size() > 0) {
			auto back = code.back();
			code.pop_back();
			if (back == '\n') {
				size_t i = 0;
				while (i < code.size() && code[code.size() - i - 1] != '\n')
					i++;
				if (code.size() == i) {
					std::cout << "\033[2D  \033[2D\033[1A> ";
				} else {
					std::cout << "\033[2D  \033[2D\033[1A";
					printc("└ ", BRIGHT_YELLOW_TEXT);
				}
				if (i > 0)
					std::cout << "\033[" << i << "C";
			} else {
				std::cout << "\033[1D \033[1D";
			}
		}
	}
}

int main(int argc, char const *argv[])
{
#ifdef _WIN32
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
#endif
	auto parsed = parseOptions(argc, argv);
	auto options = parsed.first;

	if (options["version"] == "true") {
		std::cout << _RUOTA_VERSION_LONG_ << "\n";
		return 0;
	}
	Ruota wrapper(parsed.second);
	bool tree = options["tree"] == "true";

	if (options["file"] == "") {
		std::cout << _RUOTA_INTERPRETER_START_ << "\n";

		if (options["std"] == "true") {
			try {
				wrapper.runCode(wrapper.compileCode("load \"std.ruo\";", boost::filesystem::current_path() / "nil"), false);
				std::cout << _STANDARD_LIBRARY_LOADED_ << "\n";
			} catch (const RuotaError &e) {
				std::cout << _STANDARD_LIBRARY_LOAD_FAIL_ << string(e.what()) << "\n";
			}
		} else {
			std::cout << _OPTION_NO_STD_ << "\n";
		}

		bool flag = true;
		bool force = false;
		string code = "";
		while (true) {
			if (flag)
				std::cout << "> ";
			char c;
			while ((c = getch_n()) != '\r') {
				if (c == '\n')
					break;
				else if (c == 3)
					exit(0);
				else if (c == 18) {
					force = true;
					break;
				} else if (c == '\t') {
					code += "    ";
					std::cout << "    ";
				} else if (c == 27) {
					while (!code.empty())
						moveback(code, 1);
				} else if (c == '\b' || c == 127)
					moveback(code, 1);
				else {
					code += c;
					std::cout << c;
				}
			}

			std::unique_ptr<Node> comp;
			if (!force) {
				try {
					flag = true;
					comp = wrapper.compileCode(code, boost::filesystem::current_path() / "nil");
				} catch (const RuotaError &e) {
					flag = false;
					std::cout << "\n";

					if (code.find('\n') != string::npos) {
						std::cout << "\033[1A";
						printc("│ ", BRIGHT_YELLOW_TEXT);
						std::cout << "\033[2D\033[1B";
					}
					printc("└ ", BRIGHT_YELLOW_TEXT);

					code += "\n";
				}
			} else {
				force = false;
				try {
					flag = true;
					comp = wrapper.compileCode(code, boost::filesystem::current_path() / "nil");
				} catch (const RuotaError &e) {
					flag = false;
					code = "";
					std::cout << "\n";
					printError(e);
					std::cout << "> ";
				}
			}

			if (flag) {
				std::cout << "\n";
				code = "";
				try {
					auto value = wrapper.runCode(std::move(comp), tree);
					if (value.getValueType() == ARRAY) {
						if (value.vectorSize() != 1) {
							int i = 0;
							for (auto &e : value.getVector(NULL)) {
								printc("\t(" + std::to_string(i) + ")\t", CYAN_TEXT);
								std::cout << e.toString(NULL) << "\n";
								i++;
							}
						} else {
							std::cout << "\t" << value.getVector(NULL)[0].toString(NULL) << "\n";
						}
					}
				} catch (const RuotaError &e) {
					printError(e);
				}
			}
		}
	} else {
		std::ifstream file;
		file.open(options["file"]);
		if (!file.is_open()) {
			std::cerr << _FAILURE_FILEPATH_ << options["file"] << "\n";
			return 1;
		}

		string content = "";
		string line;
		while (std::getline(file, line))
			content += line + "\n";

		try {
			if (options["std"] == "true")
				wrapper.runCode(wrapper.compileCode("load \"std.ruo\";", boost::filesystem::current_path() / "nil"), false);
			wrapper.runCode(wrapper.compileCode(content, boost::filesystem::path(options["file"])), tree);
		} catch (const RuotaError &e) {
			printError(e);
			return 1;
		}
	}

	return 0;
}