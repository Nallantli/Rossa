#include "NodeParser.h"
#include "Library.h"
#include <fstream>

NodeParser::NodeParser(
	const std::vector<Token> &tokens,
	const std::map<string, signed int> &bOperators,
	const std::map<string, signed int> &uOperators,
	const boost::filesystem::path &currentFile) : tokens(tokens),
	bOperators(bOperators),
	uOperators(uOperators),
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
	auto n = std::make_shared<ContainerNode>(Symbol(currentToken.getValueNumber()), currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseBoolNode()
{
	auto n = std::make_shared<ContainerNode>(Symbol(currentToken.getValueString() == "true"), currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseIDNode()
{
	auto n = std::make_shared<IDNode>(MAIN_HASH.hashString(currentToken.getValueString()), currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseBIDNode()
{
	if (currentToken.getType() == TOK_IDF) {
		return parseIDNode();
	}
	auto n = std::make_shared<BIDNode>(currentToken.getValueString(), currentToken);
	nextToken();
	return n;
}

std::shared_ptr<Node> NodeParser::parseEntryNode()
{
	std::vector<std::shared_ptr<Node>> v;
	while (currentToken.getType() != NULL_TOK) {
		if (auto n = parseExprNode())
			v.push_back(n);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	return std::make_shared<VectorNode>(v, false, currentToken);
}

std::shared_ptr<Node> NodeParser::parseTrailingNode(std::shared_ptr<Node> ret, bool allowInner)
{
	if (currentToken.getType() == NULL_TOK)
		return ret;

	switch (currentToken.getType()) {
		case TOK_UNTILT:
			return parseUntilNode(ret, true);
		case TOK_UNTILF:
			return parseUntilNode(ret, false);
		case TOK_CAST:
			return parseCastToNode(ret);
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
	LexerTokenType t = static_cast<LexerTokenType>(currentToken.getType());
	string temp = currentToken.getValueString();
	auto marker = currentToken;
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '(')
		return std::make_shared<IDNode>(MAIN_HASH.hashString(temp), marker);
	nextToken();
	auto arg = parseEquNode();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != ')')
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
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != ')') {
		if (i > 0) {
			if (currentToken.getType() != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(b);
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}

	nextToken();

	std::shared_ptr<Node> ret;
	if (a->getType() == INS_NODE) {
		auto a_a = reinterpret_cast<InsNode *>(a.get())->getCallee();
		auto a_b = reinterpret_cast<InsNode *>(a.get())->getArg();
		if (a_b->getType() == BID_NODE) {
			auto nkey = reinterpret_cast<BIDNode *>(a_b.get());
			string key = nkey->getKey();
			/*if (!args.empty())
				return logErrorN("Built in functions take a single argument", currentToken);*/

			if (key == "length")
				ret = std::make_shared<CallBuiltNode>(TOK_LENGTH, a_a, marker);
			if (key == "size")
				ret = std::make_shared<CallBuiltNode>(TOK_SIZE, a_a, marker);
			if (key == "alloc")
				ret = std::make_shared<CallBuiltNode>(TOK_ALLOC, a_a, marker);
			if (key == "charn")
				ret = std::make_shared<CallBuiltNode>(TOK_CHARN, a_a, marker);
			if (key == "chars")
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
	string libname = currentToken.getValueString();
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_INNER)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ".").str(), currentToken);
	nextToken();
	string fname = currentToken.getValueString();

	auto marker = currentToken;

	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '(')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "(").str(), currentToken);
	nextToken();

	std::vector<std::shared_ptr<Node>> args;
	int i = 0;
	while (currentToken.getType() != ')') {
		if (i > 0) {
			if (currentToken.getType() != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(b);
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}
	nextToken();

	auto f = rlib::loaded.at(libname + "$" + fname);

	return std::make_shared<ExternCallNode>(libname + "$" + fname, args, marker);
}

std::pair<Signature, std::vector<std::pair<LexerTokenType, hash_ull>>> NodeParser::parseSigNode(ValueType start)
{
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '(')
		return logErrorSN((boost::format(_EXPECTED_ERROR_) % "(").str(), currentToken);
	nextToken();

	std::vector<std::pair<LexerTokenType, hash_ull>> args;
	std::vector<type_sll> types;

	int i = 0;
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != ')') {
		if (i > 0) {
			if (currentToken.getType() != ',')
				return logErrorSN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;

		int type = currentToken.getType();
		if (type != TOK_REF)
			type = TOK_NIL;
		else
			nextToken();

		string arg;

		if (currentToken.getType() != NULL_TOK && currentToken.getType() == TOK_IDF)
			arg = currentToken.getValueString();
		else
			return logErrorSN("Expected variable identifier", currentToken);
		nextToken();

		type_sll ftype = NIL;
		if (currentToken.getType() != NULL_TOK && currentToken.getType() == ':') {
			nextToken();
			switch (currentToken.getType()) {
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
					string typestr = "";
					while (currentToken.getType() != NULL_TOK && currentToken.getType() == TOK_IDF) {
						typestr += currentToken.getValueString();
						nextToken();
						if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_INNER)
							break;
						typestr += ".";
						nextToken();
					}
					ftype = MAIN_HASH.hashString(typestr);
					break;
				}
				default:
					return logErrorSN(_EXPECTED_BASE_TYPE_, currentToken);
			}
		}

		args.push_back({ static_cast<LexerTokenType>(type), MAIN_HASH.hashString(arg) });
		types.push_back(ftype);

		if (currentToken.getType() == NULL_TOK)
			return logErrorSN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}
	nextToken();

	if (start != NIL && !types.empty())
		types[0] = start;

	return { Signature(types), args };
}

std::shared_ptr<Node> NodeParser::parseDefineNode()
{
	nextToken();

	ValueType ftype = NIL;
	if (currentToken.getType() != TOK_IDF && currentToken.getType() != '~' && currentToken.getType() != TOK_LENGTH && currentToken.getType() != TOK_SIZE && currentToken.getType() != TOK_ALLOC && currentToken.getType() != TOK_CHARN && currentToken.getType() != TOK_CHARS) {
		switch (currentToken.getType()) {
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
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_DEF_TYPE)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "::").str(), currentToken);
		nextToken();
	}

	if (currentToken.getType() == NULL_TOK || (currentToken.getType() != TOK_IDF && currentToken.getType() != '~' && currentToken.getType() != TOK_LENGTH && currentToken.getType() != TOK_SIZE && currentToken.getType() != TOK_ALLOC && currentToken.getType() != TOK_CHARN && currentToken.getType() != TOK_CHARS))
		return logErrorN(_EXPECTED_FUNCTION_NAME_, currentToken);
	auto key = MAIN_HASH.hashString(currentToken.getValueString());
	nextToken();

	auto sig = parseSigNode(ftype);
	if (!sig.second.empty() && sig.second[0].first == 0)
		return logErrorN(_EXPECTED_FUNCTION_SIG_, currentToken);

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{') {
		auto body = parseEquNode();

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		nextToken();

		return std::make_shared<DefineNode>(key, sig.first, sig.second, body, currentToken);
	} else {
		std::vector<std::shared_ptr<Node>> vbody;

		nextToken();
		while (currentToken.getType() != NULL_TOK) {
			if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				vbody.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
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

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{') {
		auto body = parseEquNode();

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		return std::make_shared<DefineNode>(0, sig.first, sig.second, body, currentToken);
	} else {
		std::vector<std::shared_ptr<Node>> vbody;

		nextToken();
		while (currentToken.getType() != NULL_TOK) {
			if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				vbody.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
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
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ']')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "]").str(), currentToken);
		nextToken();
		auto ret = std::make_shared<BinOpNode>("[]", a, b, marker);
		return parseTrailingNode(ret, true);
	}
	return nullptr;
}

std::shared_ptr<Node> NodeParser::parseCastToNode(std::shared_ptr<Node> a)
{
	nextToken();

	ValueType convert;
	switch (currentToken.getType()) {
		case TOK_NIL_NAME:
			convert = NIL;
			break;
		case TOK_BOOLEAN:
			convert = BOOLEAN_D;
			break;
		case TOK_NUMBER:
			convert = NUMBER;
			break;
		case TOK_ARRAY:
			convert = ARRAY;
			break;
		case TOK_STRING:
			convert = STRING;
			break;
		case TOK_DICTIONARY:
			convert = DICTIONARY;
			break;
		case TOK_OBJECT:
			convert = OBJECT;
			break;
		case TOK_FUNCTION:
			convert = FUNCTION;
			break;
		case TOK_POINTER:
			convert = POINTER;
			break;
		case TOK_TYPE_NAME:
			convert = TYPE_NAME;
			break;
		default:
			return logErrorN(_EXPECTED_BASE_CAST_, currentToken);
	}

	nextToken();
	auto ret = std::make_shared<CastToNode>(convert, a, currentToken);
	return parseTrailingNode(ret, true);
}

std::shared_ptr<Node> NodeParser::parseTypeNode()
{
	nextToken();
	string typestr = "";
	while (currentToken.getType() != NULL_TOK && currentToken.getType() == TOK_IDF) {
		typestr += currentToken.getValueString();
		nextToken();
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_INNER)
			break;
		typestr += ".";
		nextToken();
	}
	return std::make_shared<ContainerNode>(Symbol(static_cast<type_sll>(MAIN_HASH.hashString(typestr))), currentToken);
}

std::shared_ptr<Node> NodeParser::parseUntilNode(std::shared_ptr<Node> a, bool inclusive)
{
	nextToken();

	auto b = parseEquNode();
	if (!b)
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

	if (currentToken.getType() != NULL_TOK && currentToken.getType() == ':') {
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

	while (currentToken.getType() != NULL_TOK && bOperators.find(currentToken.getValueString()) != bOperators.end()) {
		string opStr = currentToken.getValueString();
		int prec = bOperators.at(opStr);

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

					if (!(std::abs(bOperators.at(oldOp) < std::abs(prec) || (std::abs(bOperators.at(oldOp)) == std::abs(prec) && bOperators.at(oldOp) < 0)))) {
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
				pastPrec = bOperators.at(reinterpret_cast<BinOpNode *>(current.get())->getOp());
			}
		} else {
			return logErrorN(_EXPECTED_RH_, currentToken);
		}
	}

	return current;
}

std::shared_ptr<Node> NodeParser::parseBaseNode()
{
	if (currentToken.getType() == NULL_TOK)
		logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	std::shared_ptr<Node> ret;
	switch (currentToken.getType()) {
		case TOK_NUM:
			return parseNumNode();
		case TOK_IDF:
			return parseIDNode();
		case TOK_STR_LIT:
			ret = std::make_shared<ContainerNode>(Symbol(currentToken.getValueString()), currentToken);
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
	string opStr = currentToken.getValueString();
	auto marker = currentToken;
	nextToken();
	if (uOperators.find(opStr) != uOperators.end()) {
		if (auto a = parseEquNode())
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
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != ']') {
		int delim = ',';

		if (i > 0) {
			if (currentToken.getType() != ',' && currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",` `;").str(), currentToken);
			delim = currentToken.getType();
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
		if (currentToken.getType() == NULL_TOK)
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
	std::vector<std::pair<hash_ull, std::shared_ptr<Node>>> args;
	int i = 0;
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != '}') {
		if (i > 0) {
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (!(currentToken.getType() == TOK_STR_LIT || currentToken.getType() == TOK_IDF || currentToken.getType() == TOK_NUM || currentToken.getType() == TOK_TRUE || currentToken.getType() == TOK_FALSE))
			return logErrorN(_VALUE_KEY_PAIR_ERROR_, currentToken);
		auto key = currentToken.getValueString();
		nextToken();
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ':')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ":").str(), currentToken);
		nextToken();
		if (auto b = parseEquNode()) {
			std::pair<hash_ull, std::shared_ptr<Node>> p = { MAIN_HASH.hashString(key), b };
			args.push_back(p);
		} else
			return nullptr;
		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	}
	nextToken();
	return std::make_shared<MapNode>(args, currentToken);
}

std::shared_ptr<Node> NodeParser::parseSwitchNode()
{
	nextToken();
	std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> cases;
	auto switchs = parseEquNode();
	if (!switchs)
		return logErrorN((boost::format(_EXPECTED_AFTER_) % "switch").str(), currentToken);
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_IN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "in").str(), currentToken);
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();

	while (currentToken.getType() != NULL_TOK && currentToken.getType() != '}') {
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_CASE)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "case").str(), currentToken);
		nextToken();

		auto c = parseUnitNode();
		if (!c)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_DO)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "do").str(), currentToken);
		nextToken();

		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{') {
			auto eq = parseEquNode();

			if (!eq)
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();

			cases[c] = eq;
		} else {
			nextToken();
			std::vector<std::shared_ptr<Node>> body;
			while (currentToken.getType() != NULL_TOK) {
				if (currentToken.getType() == '}')
					break;
				if (auto e = parseExprNode())
					body.push_back(e);
				else
					return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			}
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
			nextToken();

			cases[c] = std::make_shared<VectorNode>(body, true, currentToken);
		}

		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	}
	nextToken();
	auto ret = std::make_shared<SwitchNode>(switchs, cases, currentToken);

	if (currentToken.getType() != NULL_TOK && currentToken.getType() == TOK_ELSE) {
		nextToken();
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
		nextToken();
		std::vector<std::shared_ptr<Node>> elses;
		while (currentToken.getType() != NULL_TOK) {
			if (currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				elses.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.getType() != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();
		ret->setElse(std::make_shared<VectorNode>(elses, true, currentToken));
	}

	return ret;
}

std::shared_ptr<Node> NodeParser::parseUnitNode()
{
	if (currentToken.getType() == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	std::shared_ptr<Node> ret = nullptr;
	switch (currentToken.getType()) {
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
			return parseCallBuiltNode();
		case TOK_NEW:
			return parseNewNode();
		case TOK_OPR:
			return parseUnOpNode();
		case '(':
			nextToken();
			if (ret = parseEquNode()) {
				if (currentToken.getType() == NULL_TOK || currentToken.getType() != ')')
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
	while (currentToken.getType() == TOK_INNER) {
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
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != ':')
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
	if (currentToken.getType() == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	if (auto ret = parseUnitNode()) {
		if (currentToken.getType() != NULL_TOK) {
			switch (currentToken.getType()) {
				case TOK_OPR:
					ret = parseBinOpNode(ret);
					break;
				default:
					break;
			}
			if (currentToken.getType() != NULL_TOK && currentToken.getType() == '?')
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
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_THEN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "then").str(), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK) {
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	std::shared_ptr<IfElseNode> ret;

	if (body.size() != 1)
		ret = std::make_shared<IfElseNode>(ifs, std::make_shared<VectorNode>(body, true, currentToken), marker);
	else
		ret = std::make_shared<IfElseNode>(ifs, (body[0]), marker);

	if (currentToken.getType() == NULL_TOK)
		return ret;

	switch (currentToken.getType()) {
		case TOK_ELSE:
		{
			nextToken();
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
			nextToken();
			std::vector<std::shared_ptr<Node>> elses;
			while (currentToken.getType() != NULL_TOK) {
				if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
					break;

				if (auto e = parseExprNode())
					elses.push_back(e);
				else
					return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			}
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
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
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> tbody;
	while (currentToken.getType() != NULL_TOK) {
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			tbody.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_CATCH)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "catch").str(), currentToken);
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_IDF)
		return logErrorN(_EXPECTED_IDF_, currentToken);
	hash_ull key = MAIN_HASH.hashString(currentToken.getValueString());
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_THEN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "then").str(), currentToken);
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> cbody;
	while (currentToken.getType() != NULL_TOK) {
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			cbody.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
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
	if (currentToken.getType() != TOK_DO)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "do").str(), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK) {
		if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_shared<WhileNode>(ifs, body, marker);
}

std::shared_ptr<Node> NodeParser::parseForNode()
{

	nextToken();
	auto id = MAIN_HASH.hashString(currentToken.getValueString());
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_IN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "in").str(), currentToken);
	nextToken();
	auto fors = parseEquNode();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_DO)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "do").str(), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK) {
		if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_shared<ForNode>(id, fors, body, marker);
}

std::shared_ptr<Node> NodeParser::parseExternNode()
{

	nextToken();
	string fname = currentToken.getValueString();
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_IN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "in").str(), currentToken);
	nextToken();

	string libname = currentToken.getValueString();

	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
	nextToken();

	rlib::loadFunction(currentFile.parent_path(), libname, fname, &currentToken);
	return std::make_shared<ContainerNode>(Symbol(), currentToken);
}

std::shared_ptr<Node> NodeParser::parseClassNode()
{

	auto type = currentToken.getType();
	nextToken();
	auto key = MAIN_HASH.hashString(currentToken.getValueString());
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_CLASS)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "class").str(), currentToken);
	nextToken();
	std::shared_ptr<Node> extends = nullptr;
	if (currentToken.getType() == NULL_TOK || currentToken.getType() == ':') {
		nextToken();
		extends = parseUnitNode();
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::shared_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK) {
		if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_shared<ClassNode>(key, type, body, extends, currentToken);
}

std::shared_ptr<Node> NodeParser::parseLoadNode()
{

	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_STR_LIT)
		return logErrorN(_EXPECTED_FILE_, currentToken);
	string filename = currentToken.getValueString();
	nextToken();

	auto path = rdir::findFile(currentFile.parent_path(), filename, &currentToken);

	if (std::find(rdir::loaded.begin(), rdir::loaded.end(), path) != rdir::loaded.end()) {
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		nextToken();
		return std::make_shared<ContainerNode>(Symbol(), currentToken);
	}

	rdir::loaded.push_back(path);

	string content = "";
	string line;
	std::ifstream myfile(path.string());
	if (myfile.is_open()) {
		while (getline(myfile, line))
			content += line + "\n";
		myfile.close();
	}

	auto tokens = Ruota::lexer.lexString(content, path.filename().string());
	NodeParser np(tokens, this->bOperators, this->uOperators, path);
	auto n = np.parse();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
	nextToken();

	return n;
}

std::shared_ptr<Node> NodeParser::parseExprNode()
{

	if (currentToken.getType() == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	switch (currentToken.getType()) {
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
			while (currentToken.getType() != NULL_TOK) {
				if (i++ > 0) {
					if (currentToken.getType() != ',')
						return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
					nextToken();
				}
				if (currentToken.getType() != TOK_IDF)
					return logErrorN(_EXPECTED_IDF_, currentToken);
				v.push_back(MAIN_HASH.hashString(currentToken.getValueString()));
				nextToken();
				if (currentToken.getType() == ';') {
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
				if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
				nextToken();
				return std::make_shared<ThrowNode>(ret, marker);
			}
			return nullptr;
		}
		case TOK_BREAK:
			nextToken();
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();
			return std::make_shared<BreakNode>(currentToken);
		case TOK_RETURN:
			nextToken();
			if (auto ret = parseEquNode()) {
				if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
				nextToken();
				return std::make_shared<ReturnNode>(ret, currentToken);
			}
			return nullptr;
		case TOK_REFER:
			nextToken();
			if (auto ret = parseEquNode()) {
				if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
					return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
				nextToken();
				return std::make_shared<ReferNode>(ret, currentToken);
			}
			return nullptr;
		default:
			if (auto ret = parseEquNode()) {
				if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
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

std::shared_ptr<Node> NodeParser::logErrorN(const string &s, const Token t)
{
	if (t.getType() == NULL_TOK)
		throw RuotaError(s, tokens.at(index - 1));
	else
		throw RuotaError(s, t);
	return nullptr;
}

std::pair<Signature, std::vector<std::pair<LexerTokenType, hash_ull>>> NodeParser::logErrorSN(const string &s, const Token t)
{
	logErrorN(s, t);
	return { Signature({}), {{NULL_TOK, -1}} };
}