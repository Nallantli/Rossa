#include "NodeParser.h"
#include "Library.h"
#include <fstream>

NodeParser::NodeParser(
	std::vector<Token> tokens,
	std::map<std::string, signed int> bOperators,
	std::map<std::string, signed int> uOperators,
	boost::filesystem::path currentFile) : tokens(tokens),
										   bOperators(bOperators),
										   uOperators(uOperators),
										   currentFile(currentFile) {}

void NodeParser::nextToken()
{
	if (index < this->tokens.size())
		currentToken = this->tokens.at(index++);
	else
		currentToken = Token();
}

std::unique_ptr<Node> NodeParser::parseNumNode()
{
	auto n = std::make_unique<ContainerNode>(Symbol(currentToken.getValueNumber()), currentToken);
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseBoolNode()
{
	auto n = std::make_unique<ContainerNode>(Symbol(currentToken.getValueString() == "true"), currentToken);
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseIDNode()
{
	auto n = std::make_unique<IDNode>(MAIN_HASH.hashString(currentToken.getValueString()), currentToken);
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseBIDNode()
{
	if (currentToken.getType() == TOK_IDF)
	{
		return parseIDNode();
	}
	auto n = std::make_unique<BIDNode>(currentToken.getValueString(), currentToken);
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseEntryNode()
{
	std::vector<std::unique_ptr<Node>> v;
	while (currentToken.getType() != NULL_TOK)
	{
		if (auto n = parseExprNode())
			v.push_back(std::move(n));
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	return std::make_unique<VectorNode>(std::move(v), false, currentToken);
}

std::unique_ptr<Node> NodeParser::parseTrailingNode(std::unique_ptr<Node> ret, bool allowInner)
{
	if (currentToken.getType() == NULL_TOK)
		return ret;

	switch (currentToken.getType())
	{
	case TOK_UNTILT:
		return parseUntilNode(std::move(ret), true);
	case TOK_UNTILF:
		return parseUntilNode(std::move(ret), false);
	case TOK_CAST:
		return parseCastToNode(std::move(ret));
	case TOK_INNER:
		if (allowInner)
			return parseInsNode(std::move(ret));
		else
			return ret;
	case '(':
		return parseCallNode(std::move(ret));
	case '[':
		return parseIndexNode(std::move(ret));
	default:
		return ret;
	}
}

std::unique_ptr<Node> NodeParser::parseCallBuiltNode()
{
	LexerTokenType t = static_cast<LexerTokenType>(currentToken.getType());
	std::string temp = currentToken.getValueString();
	auto marker = currentToken;
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '(')
		return std::make_unique<IDNode>(MAIN_HASH.hashString(temp), marker);
	nextToken();
	auto arg = parseEquNode();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != ')')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	nextToken();
	auto ret = std::make_unique<CallBuiltNode>(t, std::move(arg), marker);
	return parseTrailingNode(std::move(ret), true);
}

std::unique_ptr<Node> NodeParser::parseCallNode(std::unique_ptr<Node> a)
{
	auto marker = tokens.at(index - 2);
	nextToken();
	std::vector<std::unique_ptr<Node>> args;
	int i = 0;
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != ')')
	{
		if (i > 0)
		{
			if (currentToken.getType() != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(std::move(b));
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}

	nextToken();

	std::unique_ptr<Node> ret;
	if (a->getType() == INS_NODE)
	{
		auto a_a = reinterpret_cast<InsNode *>(a.get())->getCallee();
		auto a_b = reinterpret_cast<InsNode *>(a.get())->getArg();
		if (a_b->getType() == BID_NODE)
		{
			auto nkey = reinterpret_cast<BIDNode *>(a_b.get());
			std::string key = nkey->getKey();
			/*if (!args.empty())
				return logErrorN("Built in functions take a single argument", currentToken);*/

			if (key == "length")
				ret = std::make_unique<CallBuiltNode>(TOK_LENGTH, std::move(a_a), marker);
			if (key == "size")
				ret = std::make_unique<CallBuiltNode>(TOK_SIZE, std::move(a_a), marker);
			if (key == "alloc")
				ret = std::make_unique<CallBuiltNode>(TOK_ALLOC, std::move(a_a), marker);
			if (key == "charn")
				ret = std::make_unique<CallBuiltNode>(TOK_CHARN, std::move(a_a), marker);
			if (key == "chars")
				ret = std::make_unique<CallBuiltNode>(TOK_CHARS, std::move(a_a), marker);

			ret = parseTrailingNode(std::move(ret), true);
		}
		else
		{
			ret = std::make_unique<CallNode>(std::make_unique<InsNode>(std::move(a_a), std::move(a_b), currentToken), std::move(args), marker);
		}
		return parseTrailingNode(std::move(ret), true);
	}

	ret = std::make_unique<CallNode>(std::move(a), std::move(args), marker);
	return parseTrailingNode(std::move(ret), true);
}

std::unique_ptr<Node> NodeParser::parseExternCallNode()
{
	nextToken();
	std::string libname = currentToken.getValueString();
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_INNER)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ".").str(), currentToken);
	nextToken();
	std::string fname = currentToken.getValueString();

	auto marker = currentToken;

	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '(')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "(").str(), currentToken);
	nextToken();

	std::vector<std::unique_ptr<Node>> args;
	int i = 0;
	while (currentToken.getType() != ')')
	{
		if (i > 0)
		{
			if (currentToken.getType() != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(std::move(b));
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}
	nextToken();

	auto f = rlib::loaded.at(libname + "$" + fname);

	return std::make_unique<ExternCallNode>(libname + "$" + fname, std::move(args), marker);
}

std::pair<Signature, std::vector<std::pair<LexerTokenType, hashcode_t>>> NodeParser::parseSigNode(ValueType start)
{
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '(')
		return logErrorSN((boost::format(_EXPECTED_ERROR_) % "(").str(), currentToken);
	nextToken();

	std::vector<std::pair<LexerTokenType, hashcode_t>> args;
	std::vector<ValueType> types;

	int i = 0;
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != ')')
	{
		if (i > 0)
		{
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

		std::string arg;

		if (currentToken.getType() != NULL_TOK && currentToken.getType() == TOK_IDF)
			arg = currentToken.getValueString();
		else
			return logErrorSN("Expected variable identifier", currentToken);
		nextToken();

		ValueType ftype = NIL;
		if (currentToken.getType() != NULL_TOK && currentToken.getType() == ':')
		{
			nextToken();
			switch (currentToken.getType())
			{
			case TOK_BOOLEAN:
				ftype = BOOLEAN_D;
				break;
			case TOK_NUMBER:
				ftype = NUMBER;
				break;
			case TOK_VECTOR:
				ftype = VECTOR;
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
				return logErrorSN(_EXPECTED_BASE_TYPE_, currentToken);
			}
			nextToken();
		}

		args.push_back({static_cast<LexerTokenType>(type), MAIN_HASH.hashString(arg)});
		types.push_back(ftype);

		if (currentToken.getType() == NULL_TOK)
			return logErrorSN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
	}
	nextToken();

	if (start != NIL && !types.empty())
		types[0] = start;

	return {Signature(types), args};
}

std::unique_ptr<Node> NodeParser::parseDefineNode()
{
	nextToken();

	ValueType ftype = NIL;
	if (currentToken.getType() != TOK_IDF && currentToken.getType() != '~' && currentToken.getType() != TOK_LENGTH && currentToken.getType() != TOK_SIZE && currentToken.getType() != TOK_ALLOC && currentToken.getType() != TOK_CHARN && currentToken.getType() != TOK_CHARS)
	{
		switch (currentToken.getType())
		{
		case TOK_BOOLEAN:
			ftype = BOOLEAN_D;
			break;
		case TOK_NUMBER:
			ftype = NUMBER;
			break;
		case TOK_VECTOR:
			ftype = VECTOR;
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

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_IDF && currentToken.getType() != '~' && currentToken.getType() != TOK_LENGTH && currentToken.getType() != TOK_SIZE && currentToken.getType() != TOK_ALLOC && currentToken.getType() != TOK_CHARN && currentToken.getType() != TOK_CHARS)
		return logErrorN(_EXPECTED_FUNCTION_NAME_, currentToken);
	auto key = MAIN_HASH.hashString(currentToken.getValueString());
	nextToken();

	auto sig = parseSigNode(ftype);
	if (!sig.second.empty() && sig.second[0].first == 0)
		return logErrorN(_EXPECTED_FUNCTION_SIG_, currentToken);

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
	{
		auto body = parseEquNode();

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		nextToken();

		return std::make_unique<DefineNode>(key, sig.first, sig.second, std::move(body), currentToken);
	}
	else
	{
		std::vector<std::unique_ptr<Node>> vbody;

		nextToken();
		while (currentToken.getType() != NULL_TOK)
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				vbody.push_back(std::move(e));
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();

		auto body = std::make_unique<VectorNode>(std::move(vbody), true, currentToken);
		return std::make_unique<DefineNode>(key, sig.first, sig.second, std::move(body), currentToken);
	}
}

std::unique_ptr<Node> NodeParser::parseNewNode()
{
	nextToken();

	auto body = parseUnitNode();
	if (body->getType() != CALL_NODE)
		return logErrorN(_EXPECTED_OBJECT_NAME_, currentToken);
	auto object = reinterpret_cast<CallNode *>(body.get())->getCallee();
	auto params = std::make_unique<VectorNode>(std::move(reinterpret_cast<CallNode *>(body.get())->getArgs()), false, currentToken);

	return std::make_unique<NewNode>(std::move(object), std::move(params), currentToken);
}

std::unique_ptr<Node> NodeParser::parseLambdaNode()
{
	nextToken();

	auto sig = parseSigNode(NIL);
	if (!sig.second.empty() && sig.second[0].first == 0)
		return logErrorN(_EXPECTED_FUNCTION_SIG_, currentToken);

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
	{
		auto body = parseEquNode();

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		return std::make_unique<DefineNode>(0, sig.first, sig.second, std::move(body), currentToken);
	}
	else
	{
		std::vector<std::unique_ptr<Node>> vbody;

		nextToken();
		while (currentToken.getType() != NULL_TOK)
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				vbody.push_back(std::move(e));
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();

		auto body = std::make_unique<VectorNode>(std::move(vbody), true, currentToken);
		return std::make_unique<DefineNode>(0, sig.first, sig.second, std::move(body), currentToken);
	}
}

std::unique_ptr<Node> NodeParser::parseIndexNode(std::unique_ptr<Node> a)
{
	auto marker = tokens.at(index - 2);
	nextToken();
	if (auto b = parseEquNode())
	{
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ']')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "]").str(), currentToken);
		nextToken();
		auto ret = std::make_unique<IndexNode>(std::move(a), std::move(b), marker);
		return parseTrailingNode(std::move(ret), true);
	}
	return nullptr;
}

std::unique_ptr<Node> NodeParser::parseCastToNode(std::unique_ptr<Node> a)
{
	nextToken();

	ValueType convert;
	switch (currentToken.getType())
	{
	case TOK_NIL_NAME:
		convert = NIL;
		break;
	case TOK_BOOLEAN:
		convert = BOOLEAN_D;
		break;
	case TOK_NUMBER:
		convert = NUMBER;
		break;
	case TOK_VECTOR:
		convert = VECTOR;
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
	auto ret = std::make_unique<CastToNode>(convert, std::move(a), currentToken);
	return parseTrailingNode(std::move(ret), true);
}

std::unique_ptr<Node> NodeParser::parseTypeNode()
{
	nextToken();
	std::string typestr = "";
	while (currentToken.getType() != NULL_TOK && currentToken.getType() == TOK_IDF)
	{
		typestr += currentToken.getValueString();
		nextToken();
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_INNER)
			break;
		typestr += ".";
		nextToken();
	}
	return std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(MAIN_HASH.hashString(typestr))), currentToken);
}

std::unique_ptr<Node> NodeParser::parseUntilNode(std::unique_ptr<Node> a, bool inclusive)
{
	nextToken();

	auto b = parseEquNode();
	if (!b)
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

	if (currentToken.getType() != NULL_TOK && currentToken.getType() == ':')
	{
		nextToken();
		auto step = parseEquNode();
		if (!step)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		return std::make_unique<UntilNode>(std::move(a), std::move(b), std::move(step), inclusive, currentToken);
	}

	return std::make_unique<UntilNode>(std::move(a), std::move(b), std::make_unique<ContainerNode>(Symbol(CNumber::Long(1)), currentToken), inclusive, currentToken);
}

std::unique_ptr<Node> NodeParser::parseBinOpNode(std::unique_ptr<Node> a)
{
	std::unique_ptr<Node> current = std::move(a);
	int pastPrec = 999;

	while (currentToken.getType() != NULL_TOK && bOperators.find(currentToken.getValueString()) != bOperators.end())
	{
		std::string opStr = currentToken.getValueString();
		int prec = bOperators[opStr];

		auto marker = currentToken;

		nextToken();

		if (auto b = parseUnitNode())
		{
			if (std::abs(prec) < std::abs(pastPrec) || (std::abs(prec) == std::abs(pastPrec) && prec > 0))
			{
				current = std::make_unique<BinOpNode>(
					opStr,
					std::move(current),
					std::move(b),
					marker);
				pastPrec = prec;
			}
			else
			{
				auto oldToken = current->getToken();
				auto oldOp = reinterpret_cast<BinOpNode *>(current.get())->getOp();
				auto current_a = reinterpret_cast<BinOpNode *>(current.get())->getA();
				auto current_b = reinterpret_cast<BinOpNode *>(current.get())->getB();
				current = std::make_unique<BinOpNode>(
					oldOp,
					std::move(current_a),
					std::make_unique<BinOpNode>(
						opStr,
						std::move(current_b),
						std::move(b), marker),
					oldToken);
				pastPrec = bOperators[oldOp];
			}
		}
		else
		{
			return logErrorN(_EXPECTED_RH_, currentToken);
		}
	}

	return current;
}

std::unique_ptr<Node> NodeParser::parseBaseNode()
{
	if (currentToken.getType() == NULL_TOK)
		logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	std::unique_ptr<Node> ret;
	switch (currentToken.getType())
	{
	case TOK_NUM:
		return parseNumNode();
	case TOK_IDF:
		return parseIDNode();
	case TOK_VAR:
		nextToken();
		ret = std::make_unique<VarNode>(MAIN_HASH.hashString(currentToken.getValueString()), currentToken);
		nextToken();
		return ret;
	case TOK_STR_LIT:
		ret = std::make_unique<ContainerNode>(Symbol(currentToken.getValueString()), currentToken);
		nextToken();
		return ret;
	case TOK_NIL:
		ret = std::make_unique<ContainerNode>(Symbol(), currentToken);
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
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(NIL)), currentToken);
		nextToken();
		return ret;
	case TOK_NUMBER:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(NUMBER)), currentToken);
		nextToken();
		return ret;
	case TOK_STRING:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(STRING)), currentToken);
		nextToken();
		return ret;
	case TOK_BOOLEAN:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(BOOLEAN_D)), currentToken);
		nextToken();
		return ret;
	case TOK_VECTOR:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(VECTOR)), currentToken);
		nextToken();
		return ret;
	case TOK_DICTIONARY:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(DICTIONARY)), currentToken);
		nextToken();
		return ret;
	case TOK_OBJECT:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(OBJECT)), currentToken);
		nextToken();
		return ret;
	case TOK_FUNCTION:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(FUNCTION)), currentToken);
		nextToken();
		return ret;
	case TOK_TYPE_NAME:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(TYPE_NAME)), currentToken);
		nextToken();
		return ret;
	case TOK_POINTER:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<object_type_t>(POINTER)), currentToken);
		nextToken();
		return ret;
	default:
		return nullptr;
	}
}

std::unique_ptr<Node> NodeParser::parseUnOpNode()
{
	std::string opStr = currentToken.getValueString();
	auto marker = currentToken;
	nextToken();
	if (uOperators.find(opStr) != uOperators.end())
	{
		if (auto a = parseEquNode())
			return std::make_unique<UnOpNode>(opStr, std::move(a), marker);
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	else
	{
		return logErrorN(_NOT_UNARY_OP_, marker);
	}
}

std::unique_ptr<Node> NodeParser::parseVectorNode()
{
	nextToken();
	std::vector<std::unique_ptr<Node>> args;
	int i = 0;
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != ']')
	{
		if (i > 0)
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ',')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ",").str(), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(std::move(b));
		else
			return nullptr;
		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "]").str(), currentToken);
	}
	nextToken();
	return std::make_unique<VectorNode>(std::move(args), false, currentToken);
}

std::unique_ptr<Node> NodeParser::parseMapNode()
{
	nextToken();
	std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>> args;
	int i = 0;
	while (currentToken.getType() != NULL_TOK && currentToken.getType() != '}')
	{
		if (i > 0)
		{
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
		if (auto b = parseEquNode())
		{
			std::pair<hashcode_t, std::unique_ptr<Node>> p = {MAIN_HASH.hashString(key), std::move(b)};
			args.push_back(std::move(p));
		}
		else
			return nullptr;
		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	}
	nextToken();
	return std::make_unique<MapNode>(std::move(args), currentToken);
}

std::unique_ptr<Node> NodeParser::parseSwitchNode()
{
	nextToken();
	std::map<std::unique_ptr<Node>, std::unique_ptr<Node>> cases;
	auto switchs = parseEquNode();
	if (!switchs)
		return logErrorN((boost::format(_EXPECTED_AFTER_) % "switch").str(), currentToken);
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_IN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "in").str(), currentToken);
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();

	while (currentToken.getType() != NULL_TOK && currentToken.getType() != '}')
	{
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_CASE)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "case").str(), currentToken);
		nextToken();

		auto c = parseUnitNode();
		if (!c)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_DO)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "do").str(), currentToken);
		nextToken();

		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		{
			auto eq = parseEquNode();

			if (!eq)
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();

			cases[std::move(c)] = std::move(eq);
		}
		else
		{
			nextToken();
			std::vector<std::unique_ptr<Node>> body;
			while (currentToken.getType() != NULL_TOK)
			{
				if (currentToken.getType() == '}')
					break;
				if (auto e = parseExprNode())
					body.push_back(std::move(e));
				else
					return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			}
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
			nextToken();

			cases[std::move(c)] = std::make_unique<VectorNode>(std::move(body), true, currentToken);
		}

		if (currentToken.getType() == NULL_TOK)
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	}
	nextToken();
	auto ret = std::make_unique<SwitchNode>(std::move(switchs), std::move(cases), currentToken);

	if (currentToken.getType() != NULL_TOK && currentToken.getType() == TOK_ELSE)
	{
		nextToken();
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
		nextToken();
		std::vector<std::unique_ptr<Node>> elses;
		while (currentToken.getType() != NULL_TOK)
		{
			if (currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				elses.push_back(std::move(e));
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.getType() != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();
		ret->setElse(std::make_unique<VectorNode>(std::move(elses), true, currentToken));
	}

	return ret;
}

std::unique_ptr<Node> NodeParser::parseUnitNode()
{
	if (currentToken.getType() == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	std::unique_ptr<Node> ret = nullptr;
	switch (currentToken.getType())
	{
	case ';':
	case 0:
		return nullptr;
	case TOK_NUM:
	case TOK_IDF:
	case TOK_VAR:
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
	case TOK_VECTOR:
	case TOK_DICTIONARY:
	case TOK_OBJECT:
	case TOK_FUNCTION:
	case TOK_TYPE_NAME:
	case TOK_POINTER:
	case TOK_LAMBDA:
		ret = parseBaseNode();
		return parseTrailingNode(std::move(ret), true);
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
		if (ret = parseEquNode())
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ')')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ")").str(), currentToken);
			nextToken();
			return parseTrailingNode(std::move(ret), true);
		}
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	default:
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
}

std::unique_ptr<Node> NodeParser::parseInsNode(std::unique_ptr<Node> ret)
{
	while (currentToken.getType() == TOK_INNER)
	{
		nextToken();
		ret = std::make_unique<InsNode>(std::move(ret), parseBIDNode(), currentToken);
	}

	return parseTrailingNode(std::move(ret), false);
}

std::unique_ptr<Node> NodeParser::parseThenNode(std::unique_ptr<Node> a)
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
	auto ret = std::make_unique<IfElseNode>(std::move(a), std::move(b), currentToken);
	ret->setElse(std::move(c));
	return ret;
}

std::unique_ptr<Node> NodeParser::parseEquNode()
{
	if (currentToken.getType() == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	if (auto ret = parseUnitNode())
	{
		if (currentToken.getType() != NULL_TOK)
		{
			switch (currentToken.getType())
			{
			case TOK_OPR:
				ret = parseBinOpNode(std::move(ret));
				break;
			default:
				break;
			}
			if (currentToken.getType() != NULL_TOK && currentToken.getType() == '?')
				return parseThenNode(std::move(ret));
		}

		return ret;
	}

	return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
}

std::unique_ptr<Node> NodeParser::parseIfElseNode()
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
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	std::unique_ptr<IfElseNode> ret;

	if (body.size() != 1)
		ret = std::make_unique<IfElseNode>(std::move(ifs), std::make_unique<VectorNode>(std::move(body), true, currentToken), marker);
	else
		ret = std::make_unique<IfElseNode>(std::move(ifs), std::move(body[0]), marker);

	if (currentToken.getType() == NULL_TOK)
		return ret;

	switch (currentToken.getType())
	{
	case TOK_ELSE:
	{
		nextToken();
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
		nextToken();
		std::vector<std::unique_ptr<Node>> elses;
		while (currentToken.getType() != NULL_TOK)
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				elses.push_back(std::move(e));
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
		nextToken();
		if (elses.size() != 1)
			ret->setElse(std::make_unique<VectorNode>(std::move(elses), true, currentToken));
		else
			ret->setElse(std::move(elses[0]));
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

std::unique_ptr<Node> NodeParser::parseTryCatchNode()
{
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::unique_ptr<Node>> tbody;
	while (currentToken.getType() != NULL_TOK)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			tbody.push_back(std::move(e));
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
	hashcode_t key = MAIN_HASH.hashString(currentToken.getValueString());
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_THEN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "then").str(), currentToken);
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::unique_ptr<Node>> cbody;
	while (currentToken.getType() != NULL_TOK)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			cbody.push_back(std::move(e));
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_unique<TryCatchNode>(std::make_unique<VectorNode>(std::move(tbody), true, currentToken), std::make_unique<VectorNode>(std::move(cbody), true, currentToken), key, currentToken);
}

std::unique_ptr<Node> NodeParser::parseWhileNode()
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
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK)
	{
		if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_unique<WhileNode>(std::move(ifs), std::move(body), marker);
}

std::unique_ptr<Node> NodeParser::parseForNode()
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
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK)
	{
		if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_unique<ForNode>(id, std::move(fors), std::move(body), marker);
}

std::unique_ptr<Node> NodeParser::parseExternNode()
{

	nextToken();
	std::string fname = currentToken.getValueString();
	nextToken();

	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_IN)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "in").str(), currentToken);
	nextToken();

	std::string libname = currentToken.getValueString();

	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
	nextToken();

	rlib::loadFunction(currentFile.parent_path(), libname, fname, &currentToken);
	return std::make_unique<ContainerNode>(Symbol(), currentToken);
}

std::unique_ptr<Node> NodeParser::parseClassNode()
{

	auto type = currentToken.getType();
	nextToken();
	auto key = MAIN_HASH.hashString(currentToken.getValueString());
	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_CLASS)
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "class").str(), currentToken);
	nextToken();
	std::unique_ptr<Node> extends = nullptr;
	if (currentToken.getType() == NULL_TOK || currentToken.getType() == ':')
	{
		nextToken();
		extends = parseUnitNode();
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '{')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "{").str(), currentToken);
	nextToken();
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != NULL_TOK)
	{
		if (currentToken.getType() == NULL_TOK || currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != '}')
		return logErrorN((boost::format(_EXPECTED_ERROR_) % "}").str(), currentToken);
	nextToken();

	return std::make_unique<ClassNode>(key, type, std::move(body), std::move(extends), currentToken);
}

std::unique_ptr<Node> NodeParser::parseLoadNode()
{

	nextToken();
	if (currentToken.getType() == NULL_TOK || currentToken.getType() != TOK_STR_LIT)
		return logErrorN(_EXPECTED_FILE_, currentToken);
	std::string filename = currentToken.getValueString();
	nextToken();

	auto path = rdir::findFile(currentFile.parent_path(), filename, &currentToken);

	if (std::find(rdir::loaded.begin(), rdir::loaded.end(), path) != rdir::loaded.end())
	{
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		nextToken();
		return std::make_unique<ContainerNode>(Symbol(), currentToken);
	}

	rdir::loaded.push_back(path);

	std::string content = "";
	std::string line;
	std::ifstream myfile(path.string());
	if (myfile.is_open())
	{
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

std::unique_ptr<Node> NodeParser::parseExprNode()
{

	if (currentToken.getType() == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	switch (currentToken.getType())
	{
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
	case TOK_THROW:
	{
		auto marker = currentToken;
		nextToken();
		if (auto ret = parseEquNode())
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();
			return std::make_unique<ThrowNode>(std::move(ret), marker);
		}
		return nullptr;
	}
	case TOK_BREAK:
		nextToken();
		if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
			return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
		nextToken();
		return std::make_unique<BreakNode>(currentToken);
	case TOK_RETURN:
		nextToken();
		if (auto ret = parseEquNode())
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();
			return std::make_unique<ReturnNode>(std::move(ret), currentToken);
		}
		return nullptr;
	case TOK_REFER:
		nextToken();
		if (auto ret = parseEquNode())
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();
			return std::make_unique<ReferNode>(std::move(ret), currentToken);
		}
		return nullptr;
	default:
		if (auto ret = parseEquNode())
		{
			if (currentToken.getType() == NULL_TOK || currentToken.getType() != ';')
				return logErrorN((boost::format(_EXPECTED_ERROR_) % ";").str(), currentToken);
			nextToken();
			return ret;
		}
		return nullptr;
	}
}

std::unique_ptr<Node> NodeParser::parse()
{
	nextToken();
	return parseEntryNode();
}

std::shared_ptr<Instruction> NodeParser::genParser(std::unique_ptr<Node> n)
{
	return n->genParser();
}

std::unique_ptr<Node> NodeParser::logErrorN(const std::string &s, const Token t)
{
	if (t.getType() == NULL_TOK)
		throw RuotaError(s, tokens.at(index - 1));
	else
		throw RuotaError(s, t);
	return nullptr;
}

std::pair<Signature, std::vector<std::pair<LexerTokenType, hashcode_t>>> NodeParser::logErrorSN(const std::string &s, const Token t)
{
	logErrorN(s, t);
	return {Signature({}), {{NULL_TOK, -1}}};
}