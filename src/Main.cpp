#include <iostream>
#include <fstream>

#include "../bin/include/Rossa.h"

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

inline const std::pair<std::map<std::string, std::string>, std::vector<std::string>> parseOptions(int argc, char const *argv[])
{
	std::map<std::string, std::string> options = {
		{"tree", "false"},
		{"version", "false"},
		{"std", "true"},
		{"file", ""},
		{"output", ""},
		{"compile", "false"} };
	std::vector<std::string> passed;

	bool flag = false;

	for (int i = 1; i < argc; i++) {
		if (flag == false && argv[i][0] == '-') {
			if (std::string(argv[i]) == "--tree" || std::string(argv[i]) == "-t")
				options["tree"] = "true";
			else if (std::string(argv[i]) == "--no-std" || std::string(argv[i]) == "-ns")
				options["std"] = "false";
			else if (std::string(argv[i]) == "--version" || std::string(argv[i]) == "-v")
				options["version"] = "true";
			else if (std::string(argv[i]) == "--compile" || std::string(argv[i]) == "-c")
				options["compile"] = "true";
			else if (std::string(argv[i]) == "--output" || std::string(argv[i]) == "-o")
				options["output"] = argv[++i];
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

void moveback(std::string &code, int c)
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
					PRINTC("└ ", rossa::BRIGHT_YELLOW_TEXT);
				}
				if (i > 0)
					std::cout << "\033[" << i << "C";
			} else {
				std::cout << "\033[1D \033[1D";
			}
		}
	}
}

static void compile(std::shared_ptr<rossa::Node> entry, const std::string &output)
{
#ifndef _WIN32
	std::filesystem::path outputExe = output;
#else
	std::filesystem::path outputExe = output + ".exe";
#endif

	std::cout << "Compiling to exe: " << output << "...\n";

	std::cout << "[0/6]\tParsing operators...\n";
	auto g = rossa::NodeParser::genParser(entry);

	std::cout << "[1/6]\tExporting hashtable...\n";
	std::string hashs = "{";
	size_t i = 0;
	for (auto &e : rossa::Rossa::MAIN_HASH.variable_hash) {
		if (i++ > 0)
			hashs += ", ";
		hashs += "\"" + e + "\"";
	}
	hashs += "}";

	std::cout << "[2/6]\tAdding library links...\n";
	std::string libs = "std::map<std::string, extf_t> fmap;\n";
	std::string libincludes = "";
	std::string libpaths = "\"" + (rossa::dir::getRuntimePath().parent_path() / "include" / "librossa.a").string() + "\" ";

	if (!rossa::lib::libPaths.empty()) {
		for (auto &l : rossa::lib::loaded) {
			if (l.first == "STANDARD")
				continue;
			libincludes += "#include \"" + (rossa::dir::getRuntimePath().parent_path() / "include" / (l.first + ".h")).string() + "\"\n";
			libs += "fmap.clear();\n" + l.first + "_rossaExportFunctions(fmap);\nlib::loaded[\"" + l.first + "\"] = fmap;\n";
			libpaths += "\"" + (rossa::dir::getRuntimePath().parent_path() / "include" / (l.first + ".a")).string() + "\" ";
		}
	}

	std::cout << "[3/6]\tTranspiling code...\n";
	std::ofstream file((rossa::dir::getRuntimePath().parent_path() / "include" / ".TEMP.cpp").string());
	file << "#include \"" + (rossa::dir::getRuntimePath().parent_path() / "include" / "Standard.h").string() + "\"\n" << libincludes << "using namespace rossa;\nint main(int argc, char const *argv[])\n{\nRossa r(rossa::dir::compiledOptions(argc, argv));\nRossa::MAIN_HASH.variable_hash = " << hashs << ";\nToken t;\n" << libs << "std::vector<Function> stack_trace;\nauto i = " << g->compile() << ";\ni->evaluate(r.main, stack_trace);\nreturn 0;\n}";
	file.close();

	std::cout << "[4/6]\tWriting executable...\n";
#ifndef _WIN32
	system(format::format("g++ -D_STATIC_ --std=c++17 -o {1} {2} {3}-O3 -ldl -pthread", { outputExe.string(), (rossa::dir::getRuntimePath().parent_path() / "include" / ".TEMP.cpp").string(), libpaths }).c_str());
#else
	std::cout << "system: " << format::format("g++ -D_STATIC_ --std=c++17 -o \"{1}\" \"{2}\" {3}-O3 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic", { outputExe.string(), (rossa::dir::getRuntimePath().parent_path() / "include" / ".TEMP.cpp").string(), libpaths });
	system(format::format("g++ -D_STATIC_ --std=c++17 -o \"{1}\" \"{2}\" {3}-O3 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic", { outputExe.string(), (rossa::dir::getRuntimePath().parent_path() / "include" / ".TEMP.cpp").string(), libpaths }).c_str());
#endif
	std::cout << "[5/6]\tCleaning...\n";
	std::filesystem::remove(rossa::dir::getRuntimePath().parent_path() / "include" / ".TEMP.cpp");

	std::cout << "[6/6]\tDone.\n";
}

int main(int argc, char const *argv[])
{
	auto parsed = parseOptions(argc, argv);
	auto options = parsed.first;
	if (options["version"] == "true") {
		std::cout << _ROSSA_VERSION_LONG_ << "\n";
		return 0;
	}
	rossa::Rossa wrapper(parsed.second);

	PRINTC("", 0);
	bool tree = options["tree"] == "true";

	if (options["file"] == "") {
		std::cout << _ROSSA_INTERPRETER_START_ << "\n";

		if (options["std"] == "true") {
			try {
				wrapper.runCode(wrapper.compileCode(KEYWORD_LOAD " \"std\";", std::filesystem::current_path() / "*"), false);
				std::cout << _STANDARD_LIBRARY_LOADED_ << "\n";
			} catch (const rossa::RTError &e) {
				std::cout << _STANDARD_LIBRARY_LOAD_FAIL_ << std::string(e.what()) << "\n";
			}
		} else {
			std::cout << _OPTION_NO_STD_ << "\n";
		}

		bool flag = true;
		bool force = false;
		std::string code = "";
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

			std::shared_ptr<rossa::Node> comp;
			if (!force) {
				try {
					flag = true;
					comp = wrapper.compileCode(code, std::filesystem::current_path() / "*");
				} catch (const rossa::RTError &e) {
					flag = false;
					std::cout << "\n";

					if (code.find('\n') != std::string::npos) {
						std::cout << "\033[1A";
						PRINTC("│ ", rossa::BRIGHT_YELLOW_TEXT);
						std::cout << "\033[2D\033[1B";
					}
					PRINTC("└ ", rossa::BRIGHT_YELLOW_TEXT);

					code += "\n";
				}
			} else {
				force = false;
				try {
					flag = true;
					comp = wrapper.compileCode(code, std::filesystem::current_path() / "*");
				} catch (const rossa::RTError &e) {
					flag = false;
					code = "";
					std::cout << "\n";
					rossa::Rossa::printError(e);
					std::cout << "> ";
				}
			}

			if (flag) {
				std::cout << "\n";
				code = "";
				try {
					auto value = wrapper.runCode(std::move(comp), tree);
					std::vector<rossa::Function> stack_trace;
					if (value.getValueType() == rossa::ARRAY) {
						if (value.vectorSize() != 1) {
							int i = 0;
							for (auto &e : value.getVector(NULL, stack_trace)) {
								PRINTC("\t(" + std::to_string(i) + ")\t", rossa::CYAN_TEXT);
								std::cout << e.toString(NULL, stack_trace) << "\n";
								i++;
							}
						} else {
							std::cout << "\t" << value.getVector(NULL, stack_trace)[0].toString(NULL, stack_trace) << "\n";
						}
					}
				} catch (const rossa::RTError &e) {
					rossa::Rossa::printError(e);
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

		std::string content = "";
		std::string line;
		while (std::getline(file, line))
			content += line + "\n";

		try {
			if (options["std"] == "true")
				content = (KEYWORD_LOAD " \"std\";\n") + content;
			auto entry = wrapper.compileCode(content, std::filesystem::path(options["file"]));
			if (options["compile"] == "true")
				compile(entry, (options["output"] == "" ? "out" : options["output"]));
			else
				wrapper.runCode(entry, tree);
		} catch (const rossa::RTError &e) {
			rossa::Rossa::printError(e);
			return 1;
		}
	}

	return 0;
}