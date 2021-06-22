#include <iostream>
#include <fstream>

#include "rossa/rossa.h"
#include "rossa/parser/parser.h"
#include "rossa/symbol/symbol.h"
#include "rossa/function/function.h"

inline const std::pair<std::map<std::string, std::string>, std::vector<std::string>> parseOptions(int argc, char const *argv[])
{
	std::map<std::string, std::string> options = {
		{"tree", "false"},
		{"version", "false"},
		{"standard", "true"},
		{"file", ""},
		{"output", ""}
	};
	std::vector<std::string> passed;

	bool flag = false;

	for (int i = 1; i < argc; i++) {
		if (flag == false && argv[i][0] == '-') {
			if (std::string(argv[i]) == "--tree" || std::string(argv[i]) == "-t")
				options["tree"] = "true";
			else if (std::string(argv[i]) == "--no-standard" || std::string(argv[i]) == "-ns")
				options["standard"] = "false";
			else if (std::string(argv[i]) == "--version" || std::string(argv[i]) == "-v")
				options["version"] = "true";
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

int main(int argc, char const *argv[])
{
	auto parsed = parseOptions(argc, argv);
	auto options = parsed.first;
	if (options["version"] == "true") {
		std::cout << _ROSSA_VERSION_LONG_ << "\n";
		return 0;
	}
	parser_t wrapper(parsed.second);

	printc("", RESET_TEXT);
	bool tree = options["tree"] == "true";

	if (options["file"] == "") {
		std::cout << _ROSSA_INTERPRETER_START_ << "\n";

		if (options["standard"] == "true") {
			try {
				wrapper.runCode(wrapper.compileCode(KEYWORD_LOAD " \"standard\";", std::filesystem::current_path() / "*"), false);
				std::cout << _STANDARD_LIBRARY_LOADED_ << "\n";
			} catch (const rossa_error_t &e) {
				std::cout << _STANDARD_LIBRARY_LOAD_FAIL_ << std::string(e.what()) << "\n";
			}
		} else {
			std::cout << _OPTION_NO_STD_ << "\n";
		}

		std::string code;
		while (true) {
			std::cout << "> ";
			std::getline(std::cin, code);
			try {
				auto comp = wrapper.compileCode(code, std::filesystem::current_path() / "*");
				auto value = wrapper.runCode(std::move(comp), tree);
				trace_t stack_trace;
				if (value.getValueType() == value_type_enum::ARRAY) {
					if (value.vectorSize() != 1) {
						int i = 0;
						for (auto &e : value.getVector(NULL, stack_trace)) {
							printc("\t(" + std::to_string(i) + ")\t", CYAN_TEXT);
#ifdef DEBUG
							std::cout << e.toCodeString() << "\n";
#else
							std::cout << e.toString(NULL, stack_trace) << "\n";
#endif
							i++;
						}
					} else {
#ifdef DEBUG
						std::cout << "\t" << value.getVector(NULL, stack_trace)[0].toCodeString() << "\n";
#else
						std::cout << "\t" << value.getVector(NULL, stack_trace)[0].toString(NULL, stack_trace) << "\n";
#endif
					}
				}
			} catch (const rossa_error_t &e) {
				parser_t::printError(e);
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
			if (options["standard"] == "true")
				content = (KEYWORD_LOAD " \"standard\";\n") + content;
			auto entry = wrapper.compileCode(content, std::filesystem::path(options["file"]));
			wrapper.runCode(entry, tree);
		} catch (const rossa_error_t &e) {
			parser_t::printError(e);
			return 1;
		}
	}

	return 0;
}