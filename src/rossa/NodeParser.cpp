#include "Node.h"
#include <fstream>

using namespace rossa;

NodeParser::NodeParser(
	const std::vector<Token> &tokens,
	const boost::filesystem::path &currentFile) : tokens(tokens),
	currentFile(currentFile)
{}

void NodeParser::nextToken()
{
	if (index < this->tokens.size())
		currentToken = this->tokens.at(index++);
	else
		currentToken = Token();
}

std::shared_ptr<Node> NodeParser::parseNumNode()
{
	auto n = std::make_shared<ContainerNode>(Symbol(currentToken.valueNumber), currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseBoolNode()
{
	auto n = std::make_shared<ContainerNode>(Symbol(currentToken.valueString == KEYWORD_TRUE), currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseIDNode()
{
	auto n = std::make_shared<IDNode>(ROSSA_HASH(currentToken.valueString), currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseBIDNode()
{
	if (currentToken.type == TOK_IDF) {
		return parseIDNode();
	}
	auto n = std::make_shared<BIDNode>(currentToken.valueString, currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseEntryNode()
{
	std::vector<std::shared_ptr<Node>> v;
	while (currentToken.type != NULL_TOK)
		v.push_back(parseExprNode());
	return std::make_shared<VectorNode>(v, false, currentToken);
}

std::shared_ptr<Node> NodeParser::parseTrailingNode(std::shared_ptr<Node> ret, bool allowInner)
{
	if (currentToken.type == NULL_TOK)
		return ret;

	switch (currentToken.type) {
		case TOK_UNTILT:
			return parseUntilNode(ret, true);
		case TOK_UNTILF:
			return parseUntilNode(ret, false);
		case TOK_INNER:
			if (allowInner)
				return parseInsNode(ret);
			else
				return ret;
		case '(':
			return parseCallNode(ret);
		case '[':
			return parseIndexNode(ret);
		default:
			return ret;
	}
}

std::shared_ptr<Node> NodeParser::parseCallBuiltNode()
{
	LexerTokenType t = static_cast<LexerTokenType>(currentToken.type);
	std::string temp = currentToken.valueString;
	auto marker = currentToken;
	nextToken();
	if (currentToken.type != '(')
		return std::make_shared<IDNode>(ROSSA_HASH(temp), marker);
	nextToken();
	auto arg = parseEquNode();
	if (currentToken.type != ')')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	nextToken();
	auto ret = std::make_shared<CallBuiltNode>(t, arg, marker);
	return parseTrailingNode(ret, true);
}

std::shared_ptr<Node> NodeParser::parseCallNode(std::shared_ptr<Node> a)
{
	auto marker = tokens.at(index - 2);
	nextToken();
	std::vector<std::shared_ptr<Node>> args;
	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != ')') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(b);
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.type == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}

	nextToken();

	std::shared_ptr<Node> ret;
	if (a->getType() == INS_NODE) {
		auto a_a = reinterpret_cast<InsNode *>(a.get())->getCallee();
		auto a_b = reinterpret_cast<InsNode *>(a.get())->getArg();
		if (a_b->getType() == BID_NODE) {
			auto nkey = reinterpret_cast<BIDNode *>(a_b.get());
			std::string key = nkey->getKey();
			/*if (!args.empty())
				return logErrorN("Built in functions take a single argument", currentToken);*/

			if (key == KEYWORD_LENGTH)
				ret = std::make_shared<CallBuiltNode>(TOK_LENGTH, a_a, marker);
			if (key == KEYWORD_SIZE)
				ret = std::make_shared<CallBuiltNode>(TOK_SIZE, a_a, marker);
			if (key == KEYWORD_ALLOC)
				ret = std::make_shared<CallBuiltNode>(TOK_ALLOC, a_a, marker);
			if (key == KEYWORD_PARSE)
				ret = std::make_shared<CallBuiltNode>(TOK_PARSE, a_a, marker);
			if (key == KEYWORD_CHAR_N)
				ret = std::make_shared<CallBuiltNode>(TOK_CHARN, a_a, marker);
			if (key == KEYWORD_CHAR_S)
				ret = std::make_shared<CallBuiltNode>(TOK_CHARS, a_a, marker);

			ret = parseTrailingNode(ret, true);
		} else {
			ret = std::make_shared<CallNode>(std::make_shared<InsNode>(a_a, a_b, currentToken), args, marker);
		}
		return parseTrailingNode(ret, true);
	}

	ret = std::make_shared<CallNode>(a, args, marker);
	return parseTrailingNode(ret, true);
}

std::shared_ptr<Node> NodeParser::parseExternCallNode()
{
	nextToken();
	std::string libname = currentToken.valueString;
	nextToken();
	if (currentToken.type != TOK_INNER)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ".").str(), currentToken);
	nextToken();
	std::string fname = currentToken.valueString;

	auto marker = currentToken;

	nextToken();

	if (currentToken.type != '(')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "(").str(), currentToken);
	nextToken();

	std::vector<std::shared_ptr<Node>> args;
	int i = 0;
	while (currentToken.type != ')') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(b);
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.type == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}
	nextToken();

	auto f = lib::loaded.at(libname + "$" + fname);

	return std::make_shared<ExternCallNode>(libname + "$" + fname, args, marker);
}

std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> NodeParser::parseSigNode(ValueType start)
{
	if (currentToken.type != '(')
		return logErrorSN((boost::format(_EXPECTED_ERROR_) % "(").str(), currentToken);
	nextToken();

	std::vector<std::pair<LexerTokenType, hash_ull>> args;
	std::vector<type_sll> types;

	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != ')') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorSN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;

		int type = currentToken.type;
		if (type != TOK_REF)
			type = TOK_NIL;
		else
			nextToken();

		std::string arg;

		if (currentToken.type == TOK_IDF)
			arg = currentToken.valueString;
		else
			return logErrorSN("Expected variable identifier", currentToken);
		nextToken();

		type_sll ftype = NIL;
		if (currentToken.type == ':') {
			nextToken();
			switch (currentToken.type) {
				case TOK_BOOLEAN:
					ftype = BOOLEAN_D;
					nextToken();
					break;
				case TOK_NUMBER:
					ftype = NUMBER;
					nextToken();
					break;
				case TOK_ARRAY:
					ftype = ARRAY;
					nextToken();
					break;
				case TOK_STRING:
					ftype = STRING;
					nextToken();
					break;
				case TOK_DICTIONARY:
					ftype = DICTIONARY;
					nextToken();
					break;
				case TOK_OBJECT:
					ftype = OBJECT;
					nextToken();
					break;
				case TOK_FUNCTION:
					ftype = FUNCTION;
					nextToken();
					break;
				case TOK_POINTER:
					ftype = POINTER;
					nextToken();
					break;
				case TOK_TYPE_NAME:
					ftype = TYPE_NAME;
					nextToken();
					break;
				case '@':
				{
					nextToken();
					std::string typestr = "";
					while (currentToken.type == TOK_IDF) {
						typestr += currentToken.valueString;
						nextToken();
						if (currentToken.type != TOK_INNER)
							break;
						typestr += ".";
						nextToken();
					}
					ftype = ROSSA_HASH(typestr);
					break;
				}
				default:
					return logErrorSN(_EXPECTED_BASE_TYPE_, currentToken);
			}
		}

		args.push_back({ static_cast<LexerTokenType>(type), ROSSA_HASH(arg) });
		types.push_back(ftype);

		if (currentToken.type == NULL_TOK)
			return logErrorSN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}
	nextToken();

	if (start != NIL && !types.empty())
		types[0] = start;

	return { sig_t(types), args };
}

std::shared_ptr<Node> NodeParser::parseDefineNode()
{
	nextToken();

	ValueType ftype = NIL;
	if (currentToken.type != TOK_IDF && currentToken.type != '~' && currentToken.type != TOK_LENGTH && currentToken.type != TOK_SIZE && currentToken.type != TOK_ALLOC && currentToken.type != TOK_CHARN && currentToken.type != TOK_CHARS && currentToken.type != TOK_PARSE) {
		switch (currentToken.type) {
			case TOK_BOOLEAN:
				ftype = BOOLEAN_D;
				break;
			case TOK_NUMBER:
				ftype = NUMBER;
				break;
			case TOK_ARRAY:
				ftype = ARRAY;
				break;
			case TOK_STRING:
				ftype = STRING;
				break;
			case TOK_DICTIONARY:
				ftype = DICTIONARY;
				break;
			case TOK_OBJECT:
				ftype = OBJECT;
				break;
			case TOK_FUNCTION:
				ftype = FUNCTION;
				break;
			case TOK_POINTER:
				ftype = POINTER;
				break;
			case TOK_TYPE_NAME:
				ftype = TYPE_NAME;
				break;
			default:
				return logErrorN(_EXPECTED_BASE_TYPE_, currentToken);
		}
		nextToken();
		if (currentToken.type != TOK_DEF_TYPE)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "::").str(), currentToken);
		nextToken();
	}

	if (currentToken.type == NULL_TOK || (currentToken.type != TOK_IDF && currentToken.type != '~' && currentToken.type != TOK_LENGTH && currentToken.type != TOK_SIZE && currentToken.type != TOK_ALLOC && currentToken.type != TOK_CHARN && currentToken.type != TOK_CHARS && currentToken.type != TOK_PARSE))
		return logErrorN(_EXPECTED_FUNCTION_NAME_, currentToken);
	auto key = ROSSA_HASH(currentToken.valueString);
	nextToken();

	auto sig = parseSigNode(ftype);
	if (!sig.second.empty() && sig.second[0].first == 0)
		return logErrorN(_EXPECTED_FUNCTION_SIG_, currentToken);

	if (currentToken.type != '{') {
		auto body = parseEquNode();

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		if (currentToken.type != ';')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		nextToken();

		return std::make_shared<DefineNode>(key, sig.first, sig.second, body, currentToken);
	} else {
		std::vector<std::shared_ptr<Node>> vbody;

		nextToken();
		while (currentToken.type != NULL_TOK) {
			if (currentToken.type == NULL_TOK || currentToken.type == '}')
				break;

			if (auto e = parseExprNode())
				vbody.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.type != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();

		auto body = std::make_shared<VectorNode>(vbody, true, currentToken);
		return std::make_shared<DefineNode>(key, sig.first, sig.second, body, currentToken);
	}
}

std::shared_ptr<Node> NodeParser::parseNewNode()
{
	nextToken();

	auto body = parseUnitNode();
	if (body->getType() != CALL_NODE)
		return logErrorN(_EXPECTED_OBJECT_NAME_, currentToken);
	auto object = reinterpret_cast<CallNode *>(body.get())->getCallee();
	auto params = std::make_shared<VectorNode>((reinterpret_cast<CallNode *>(body.get())->getArgs()), false, currentToken);

	return std::make_shared<NewNode>(object, params, currentToken);
}

std::shared_ptr<Node> NodeParser::parseLambdaNode()
{
	nextToken();

	auto sig = parseSigNode(NIL);
	if (!sig.second.empty() && sig.second[0].first == 0)
		return logErrorN(_EXPECTED_FUNCTION_SIG_, currentToken);

	if (currentToken.type != '{') {
		auto body = parseEquNode();

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		return std::make_shared<DefineNode>(0, sig.first, sig.second, body, currentToken);
	} else {
		std::vector<std::shared_ptr<Node>> vbody;

		nextToken();
		while (currentToken.type != NULL_TOK) {
			if (currentToken.type == NULL_TOK || currentToken.type == '}')
				break;

			if (auto e = parseExprNode())
				vbody.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.type != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();

		auto body = std::make_shared<VectorNode>(vbody, true, currentToken);
		return std::make_shared<DefineNode>(0, sig.first, sig.second, body, currentToken);
	}
}

std::shared_ptr<Node> NodeParser::parseIndexNode(std::shared_ptr<Node> a)
{
	auto marker = tokens.at(index - 2);
	nextToken();
	if (auto b = parseEquNode()) {
		if (currentToken.type != ']')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "]").str(), currentToken);
		nextToken();
		auto ret = std::make_shared<BinOpNode>("[]", a, b, marker);
		return parseTrailingNode(ret, true);
	}
	return nullptr;
}

std::shared_ptr<Node> NodeParser::parseTypeNode()
{
	nextToken();
	std::string typestr = "";
	while (currentToken.type == TOK_IDF) {
		typestr += currentToken.valueString;
		nextToken();
		if (currentToken.type != TOK_INNER)
			break;
		typestr += ".";
		nextToken();
	}
	return std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(ROSSA_HASH(typestr))), currentToken);
}

std::shared_ptr<Node> NodeParser::parseUntilNode(std::shared_ptr<Node> a, bool inclusive)
{
	nextToken();

	auto b = parseEquNode();
	if (!b)
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

	if (currentToken.type == ':') {
		nextToken();
		auto step = parseEquNode();
		if (!step)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		return std::make_shared<UntilNode>(a, b, step, inclusive, currentToken);
	}

	return std::make_shared<UntilNode>(a, b, nullptr, inclusive, currentToken);
}

std::shared_ptr<Node> NodeParser::parseBinOpNode(std::shared_ptr<Node> a)
{
	std::shared_ptr<Node> current = a;
	int pastPrec = 999;

	while (currentToken.type != NULL_TOK && Rossa::bOperators.find(currentToken.valueString) != Rossa::bOperators.end()) {
		std::string opStr = currentToken.valueString;
		int prec = Rossa::bOperators.at(opStr);

		auto marker = currentToken;

		nextToken();

		if (auto b = parseUnitNode()) {
			if (std::abs(prec) < std::abs(pastPrec) || (std::abs(prec) == std::abs(pastPrec) && prec > 0)) {
				current = std::make_shared<BinOpNode>(
					opStr,
					current,
					b,
					marker);
				pastPrec = prec;
			} else {
				std::shared_ptr<Node> parent = nullptr;
				auto n = current;
				while (n->getType() == BIN_OP_NODE) {
					auto bon = reinterpret_cast<BinOpNode *>(n.get());
					auto oldOp = bon->getOp();
					auto bon_b = bon->getB();

					if (!(std::abs(Rossa::bOperators.at(oldOp) < std::abs(prec) || (std::abs(Rossa::bOperators.at(oldOp)) == std::abs(prec) && Rossa::bOperators.at(oldOp) < 0)))) {
						reinterpret_cast<BinOpNode *>(parent.get())->setB(std::make_shared<BinOpNode>(
							opStr,
							n,
							b,
							marker
							));
						break;
					}
					if (bon_b->getType() != BIN_OP_NODE) {
						bon->setB(std::make_shared<BinOpNode>(
							opStr,
							bon_b,
							b,
							marker
							));
						break;
					}
					parent = n;
					n = bon_b;
				}
				pastPrec = Rossa::bOperators.at(reinterpret_cast<BinOpNode *>(current.get())->getOp());
			}
		} else {
			return logErrorN(_EXPECTED_RH_, currentToken);
		}
	}

	return current;
}

std::shared_ptr<Node> NodeParser::parseBaseNode()
{
	if (currentToken.type == NULL_TOK)
		logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	std::shared_ptr<Node> ret;
	switch (currentToken.type) {
		case TOK_NUM:
			return parseNumNode();
		case TOK_IDF:
			return parseIDNode();
		case TOK_STR_LIT:
			ret = std::make_shared<ContainerNode>(Symbol(currentToken.valueString), currentToken);
			nextToken();
			return ret;
		case TOK_NIL:
			ret = std::make_shared<ContainerNode>(Symbol(), currentToken);
			nextToken();
			return ret;
		case TOK_TRUE:
		case TOK_FALSE:
			return parseBoolNode();
		case '@':
			return parseTypeNode();
		case TOK_LAMBDA:
			return parseLambdaNode();
		case '[':
			return parseVectorNode();
		case '{':
			return parseMapNode();
		case TOK_NIL_NAME:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(NIL)), currentToken);
			nextToken();
			return ret;
		case TOK_NUMBER:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(NUMBER)), currentToken);
			nextToken();
			return ret;
		case TOK_STRING:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(STRING)), currentToken);
			nextToken();
			return ret;
		case TOK_BOOLEAN:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(BOOLEAN_D)), currentToken);
			nextToken();
			return ret;
		case TOK_ARRAY:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(ARRAY)), currentToken);
			nextToken();
			return ret;
		case TOK_DICTIONARY:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(DICTIONARY)), currentToken);
			nextToken();
			return ret;
		case TOK_OBJECT:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(OBJECT)), currentToken);
			nextToken();
			return ret;
		case TOK_FUNCTION:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(FUNCTION)), currentToken);
			nextToken();
			return ret;
		case TOK_TYPE_NAME:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(TYPE_NAME)), currentToken);
			nextToken();
			return ret;
		case TOK_POINTER:
			ret = std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(POINTER)), currentToken);
			nextToken();
			return ret;
		default:
			return nullptr;
	}
}

std::shared_ptr<Node> NodeParser::parseUnOpNode()
{
	std::string opStr = currentToken.valueString;
	auto marker = currentToken;
	nextToken();
	if (Rossa::uOperators.find(opStr) != Rossa::uOperators.end()) {
		if (auto a = parseUnitNode())
			return std::make_shared<UnOpNode>(opStr, a, marker);
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	} else {
		return logErrorN(_NOT_UNARY_OP_, marker);
	}
}

std::shared_ptr<Node> NodeParser::parseVectorNode()
{
	nextToken();
	std::vector<std::shared_ptr<Node>> args;
	std::vector<std::shared_ptr<Node>> curr;
	bool flag = false;
	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != ']') {
		int delim = ',';

		if (i > 0) {
			if (currentToken.type != ',' && currentToken.type != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",` `;").str(), currentToken);
			delim = currentToken.type;
			nextToken();
		}
		i++;

		if (delim == ';') {
			flag = true;
			args.push_back(std::make_shared<VectorNode>(curr, false, currentToken));
			curr.clear();
		}

		if (auto b = parseEquNode())
			curr.push_back((b));
		else
			return nullptr;
		if (currentToken.type == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "]").str(), currentToken);
	}
	nextToken();
	if (flag) {
		args.push_back(std::make_shared<VectorNode>(curr, false, currentToken));
		return std::make_shared<VectorNode>(args, false, currentToken);
	} else {
		return std::make_shared<VectorNode>(curr, false, currentToken);
	}
}

std::shared_ptr<Node> NodeParser::parseMapNode()
{
	nextToken();
	std::vector<std::pair<std::string, std::shared_ptr<Node>>> args;
	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != '}') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (!(currentToken.type == TOK_STR_LIT || currentToken.type == TOK_IDF || currentToken.type == TOK_NUM || currentToken.type == TOK_TRUE || currentToken.type == TOK_FALSE))
			return logErrorN(_VALUE_KEY_PAIR_ERROR_, currentToken);
		auto key = currentToken.valueString;
		nextToken();
		if (currentToken.type != ':')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ":").str(), currentToken);
		nextToken();
		if (auto b = parseEquNode()) {
			std::pair<std::string, std::shared_ptr<Node>> p = { key, b };
			args.push_back(p);
		} else
			return nullptr;
		if (currentToken.type == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	}
	nextToken();
	return std::make_shared<MapNode>(args, currentToken);
}

std::shared_ptr<Node> NodeParser::parseSwitchNode()
{
	nextToken();
	std::map<std::shared_ptr<Node>, size_t> cases;
	std::vector<std::shared_ptr<Node>> gotos;

	auto switchs = parseEquNode();
	if (!switchs)
		return logErrorN((boost::format(_EXPECTED_AFTER_) % KEYWORD_SWITCH).str(), currentToken);
	if (currentToken.type != TOK_OF)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_OF).str(), currentToken);
	nextToken();
	if (currentToken.type != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();

	while (currentToken.type != NULL_TOK && currentToken.type != '}') {
		if (currentToken.type != TOK_CASE)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_CASE).str(), currentToken);
		nextToken();

		std::vector<std::shared_ptr<Node>> keys;

		size_t i = 0;
		while (currentToken.type != NULL_TOK) {
			if (i++ > 0) {
				if (currentToken.type != ',')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
				nextToken();
			}
			auto c = parseUnitNode();
			if (!c)
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			keys.push_back(c);
			if (currentToken.type == TOK_DO)
				break;
		}

		if (currentToken.type != TOK_DO)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_DO).str(), currentToken);
		nextToken();

		if (currentToken.type != '{') {
			auto eq = parseEquNode();

			if (!eq)
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

			if (currentToken.type != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();

			gotos.push_back(eq);
		} else {
			nextToken();
			std::vector<std::shared_ptr<Node>> body;
			while (currentToken.type != NULL_TOK) {
				if (currentToken.type == '}')
					break;
				if (auto e = parseExprNode())
					body.push_back(e);
				else
					return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			}
			if (currentToken.type != '}')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
			nextToken();

			if (body.size() == 1)
				gotos.push_back(body[0]);
			else
				gotos.push_back(std::make_shared<VectorNode>(body, true, currentToken));
		}

		for (auto &c : keys)
			cases[c] = gotos.size();

		if (currentToken.type == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	}
	nextToken();
	auto ret = std::make_shared<SwitchNode>(switchs, cases, gotos, currentToken);

	if (currentToken.type == TOK_ELSE) {
		nextToken();
		if (currentToken.type != '{')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
		nextToken();
		std::vector<std::shared_ptr<Node>> elses;
		while (currentToken.type != NULL_TOK) {
			if (currentToken.type == '}')
				break;

			if (auto e = parseExprNode())
				elses.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.type != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();
		ret->setElse(std::make_shared<VectorNode>(elses, true, currentToken));
	}

	return ret;
}

std::shared_ptr<Node> NodeParser::parseUnitNode()
{
	if (currentToken.type == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	std::shared_ptr<Node> ret = nullptr;
	switch (currentToken.type) {
		case ';':
		case 0:
			return nullptr;
		case TOK_NUM:
		case TOK_IDF:
		case TOK_TRUE:
		case TOK_FALSE:
		case TOK_NIL:
		case TOK_STR_LIT:
		case '@':
		case '[':
		case '{':
		case TOK_NIL_NAME:
		case TOK_NUMBER:
		case TOK_STRING:
		case TOK_BOOLEAN:
		case TOK_ARRAY:
		case TOK_DICTIONARY:
		case TOK_OBJECT:
		case TOK_FUNCTION:
		case TOK_TYPE_NAME:
		case TOK_POINTER:
		case TOK_LAMBDA:
			ret = parseBaseNode();
			return parseTrailingNode(ret, true);
		case TOK_EXTERN_CALL:
			return parseExternCallNode();
		case TOK_LENGTH:
		case TOK_SIZE:
		case TOK_ALLOC:
		case TOK_CHARN:
		case TOK_CHARS:
		case TOK_PARSE:
			return parseCallBuiltNode();
		case TOK_NEW:
			return parseNewNode();
		case TOK_OPR:
			return parseUnOpNode();
		case '(':
			nextToken();
			if (ret = parseEquNode()) {
				if (currentToken.type != ')')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
				ret = std::make_shared<ParenNode>(ret, currentToken);
				nextToken();
				return parseTrailingNode(ret, true);
			}
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		default:
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
}

std::shared_ptr<Node> NodeParser::parseInsNode(std::shared_ptr<Node> ret)
{
	while (currentToken.type == TOK_INNER) {
		nextToken();
		ret = std::make_shared<InsNode>(ret, parseBIDNode(), currentToken);
	}

	return parseTrailingNode(ret, false);
}

std::shared_ptr<Node> NodeParser::parseThenNode(std::shared_ptr<Node> a)
{
	nextToken();
	auto b = parseEquNode();
	if (!b)
		return logErrorN((boost::format(_EXPECTED_AFTER_) % "?").str(), currentToken);
	if (currentToken.type != ':')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ":").str(), currentToken);
	nextToken();
	auto c = parseEquNode();
	if (!c)
		return logErrorN((boost::format(_EXPECTED_AFTER_) % ":").str(), currentToken);
	auto ret = std::make_shared<IfElseNode>(a, b, currentToken);
	ret->setElse(c);
	return ret;
}

std::shared_ptr<Node> NodeParser::parseEquNode()
{
	if (currentToken.type == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	if (auto ret = parseUnitNode()) {
		if (currentToken.type != NULL_TOK) {
			switch (currentToken.type) {
				case TOK_OPR:
					ret = parseBinOpNode(ret);
					break;
				default:
					break;
			}
			if (currentToken.type == '?')
				return parseThenNode(ret);
		}

		return ret;
	}

	return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
}

std::shared_ptr<Node> NodeParser::parseIfElseNode()
{
	nextToken();
	auto ifs = parseEquNode();
	if (!ifs)
		return nullptr;
	if (currentToken.type != TOK_THEN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_THEN).str(), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.type != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	std::shared_ptr<IfElseNode> ret;

	if (body.size() != 1)
		ret = std::make_shared<IfElseNode>(ifs, std::make_shared<VectorNode>(body, true, currentToken), marker);
	else
		ret = std::make_shared<IfElseNode>(ifs, (body[0]), marker);

	if (currentToken.type == NULL_TOK)
		return ret;

	switch (currentToken.type) {
		case TOK_ELSE:
		{
			nextToken();
			if (currentToken.type != '{')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
			nextToken();
			std::vector<std::shared_ptr<Node>> elses;
			while (currentToken.type != NULL_TOK) {
				if (currentToken.type == '}')
					break;

				if (auto e = parseExprNode())
					elses.push_back(e);
				else
					return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			}
			if (currentToken.type != '}')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
			nextToken();
			if (elses.size() != 1)
				ret->setElse(std::make_shared<VectorNode>(elses, true, currentToken));
			else
				ret->setElse(elses[0]);
			return ret;
		}
		case TOK_ELSEIF:
		{
			ret->setElse(parseIfElseNode());
			return ret;
		}
		default:
			return ret;
	}
}

std::shared_ptr<Node> NodeParser::parseTryCatchNode()
{
	nextToken();
	if (currentToken.type != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> tbody;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode())
			tbody.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	if (currentToken.type != TOK_CATCH)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_CATCH).str(), currentToken);
	nextToken();

	if (currentToken.type != TOK_IDF)
		return logErrorN(_EXPECTED_IDF_, currentToken);
	hash_ull key = ROSSA_HASH(currentToken.valueString);
	nextToken();

	if (currentToken.type != TOK_THEN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_THEN).str(), currentToken);
	nextToken();

	if (currentToken.type != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> cbody;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode())
			cbody.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_shared<TryCatchNode>(std::make_shared<VectorNode>(tbody, true, currentToken), std::make_shared<VectorNode>(cbody, true, currentToken), key, currentToken);
}

std::shared_ptr<Node> NodeParser::parseWhileNode()
{

	nextToken();
	auto ifs = parseEquNode();
	if (!ifs)
		return nullptr;
	if (currentToken.type != TOK_DO)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_DO).str(), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.type != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_shared<WhileNode>(ifs, body, marker);
}

std::shared_ptr<Node> NodeParser::parseForNode()
{

	nextToken();
	auto id = ROSSA_HASH(currentToken.valueString);
	nextToken();
	if (currentToken.type != TOK_IN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_IN).str(), currentToken);
	nextToken();
	auto fors = parseEquNode();
	if (currentToken.type != TOK_DO)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_DO).str(), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.type != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_shared<ForNode>(id, fors, body, marker);
}

std::shared_ptr<Node> NodeParser::parseExternNode()
{

	nextToken();
	std::string fname = currentToken.valueString;
	nextToken();

	if (currentToken.type != TOK_IN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_IN).str(), currentToken);
	nextToken();

	std::string libname = currentToken.valueString;

	nextToken();
	if (currentToken.type != ';')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
	nextToken();

	lib::loadFunction(currentFile.parent_path(), libname, fname, &currentToken);
	return nullptr;
}

std::shared_ptr<Node> NodeParser::parseClassNode()
{

	auto type = currentToken.type;
	nextToken();
	auto key = ROSSA_HASH(currentToken.valueString);
	nextToken();
	if (currentToken.type != TOK_CLASS)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % KEYWORD_CLASS).str(), currentToken);
	nextToken();
	std::shared_ptr<Node> extends = nullptr;
	if (currentToken.type == ':') {
		nextToken();
		extends = parseUnitNode();
	}
	if (currentToken.type != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_shared<ClassNode>(key, type, body, extends, currentToken);
}

std::shared_ptr<Node> NodeParser::parseLoadNode()
{
	nextToken();
	if (currentToken.type != TOK_STR_LIT)
		return logErrorN(_EXPECTED_FILE_, currentToken);
	std::string filename = currentToken.valueString + ".ra";
	nextToken();

	auto path = dir::findFile(currentFile.parent_path(), filename, &currentToken);

	if (std::find(dir::loaded.begin(), dir::loaded.end(), path) != dir::loaded.end()) {
		if (currentToken.type != ';')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		nextToken();
		return nullptr;
	}

	dir::loaded.push_back(path);

	std::string content = "";
	std::string line;
	std::ifstream myfile(path.string());
	if (myfile.is_open()) {
		while (getline(myfile, line))
			content += line + "\n";
		myfile.close();
	}

	auto tokens = Rossa::lexString(content, path.filename().string());
	NodeParser np(tokens, path);
	auto n = np.parse();

	if (currentToken.type != ';')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
	nextToken();

	return n;
}

std::shared_ptr<Node> NodeParser::parseExprNode()
{

	if (currentToken.type == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	switch (currentToken.type) {
		case ';':
			return logErrorN(_EXPECTED_EXPR_, currentToken);
		case TOK_LOAD:
			return parseLoadNode();
		case TOK_DEF:
			return parseDefineNode();
		case TOK_FOR:
			return parseForNode();
		case TOK_WHILE:
			return parseWhileNode();
		case TOK_IF:
			return parseIfElseNode();
		case TOK_EXTERN:
			return parseExternNode();
		case TOK_SWITCH:
			return parseSwitchNode();
		case TOK_TRY:
			return parseTryCatchNode();
		case TOK_STRUCT:
		case TOK_STATIC:
		case TOK_VIRTUAL:
			return parseClassNode();
		case TOK_VAR:
		{
			auto marker = currentToken;
			nextToken();
			int i = 0;
			std::vector<hash_ull> v;
			while (currentToken.type != NULL_TOK) {
				if (i++ > 0) {
					if (currentToken.type != ',')
						return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
					nextToken();
				}
				if (currentToken.type != TOK_IDF)
					return logErrorN(_EXPECTED_IDF_, currentToken);
				v.push_back(ROSSA_HASH(currentToken.valueString));
				nextToken();
				if (currentToken.type == ';') {
					nextToken();
					return std::make_shared<VarNode>(v, marker);
				}
			}
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		}
		case TOK_THROW:
		{
			auto marker = currentToken;
			nextToken();
			if (auto ret = parseEquNode()) {
				if (currentToken.type != ';')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
				nextToken();
				return std::make_shared<ThrowNode>(ret, marker);
			}
			return nullptr;
		}
		case TOK_BREAK:
			nextToken();
			if (currentToken.type != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();
			return std::make_shared<BreakNode>(currentToken);
		case TOK_CONTINUE:
			nextToken();
			if (currentToken.type != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();
			return std::make_shared<ContinueNode>(currentToken);
		case TOK_RETURN:
			nextToken();
			if (auto ret = parseEquNode()) {
				if (currentToken.type != ';')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
				nextToken();
				return std::make_shared<ReturnNode>(ret, currentToken);
			}
			return nullptr;
		case TOK_REFER:
			nextToken();
			if (auto ret = parseEquNode()) {
				if (currentToken.type != ';')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
				nextToken();
				return std::make_shared<ReferNode>(ret, currentToken);
			}
			return nullptr;
		default:
			if (auto ret = parseEquNode()) {
				if (currentToken.type != ';')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
				nextToken();
				return ret;
			}
			return nullptr;
	}
}

std::shared_ptr<Node> NodeParser::parse()
{
	nextToken();
	return parseEntryNode();
}

std::shared_ptr<Instruction> NodeParser::genParser(std::shared_ptr<Node> n)
{
	return n->genParser();
}

std::shared_ptr<Node> NodeParser::logErrorN(const std::string &s, const Token t)
{
	std::vector<Function> stack_trace;
	if (t.type == NULL_TOK)
		throw RTError(s, tokens.at(index - 1), stack_trace);
	else
		throw RTError(s, t, stack_trace);
	return nullptr;
}

std::pair<sig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> NodeParser::logErrorSN(const std::string &s, const Token t)
{
	logErrorN(s, t);
	return { sig_t({}), {{NULL_TOK, -1}} };
}