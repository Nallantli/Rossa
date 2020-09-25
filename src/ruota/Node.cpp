#include "Node.h"

Token NodeParser::nextToken()
{
	if (index < this->tokens.size())
		currentToken = this->tokens[index++];
	else
		currentToken = Token();
	return currentToken;
}

std::unique_ptr<Node> NodeParser::parseNumNode()
{
	auto n = std::make_unique<NumNode>(currentToken.getValueNumber());
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseBoolNode()
{
	auto n = std::make_unique<BoolNode>(currentToken.getValueString() == "true");
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseIDNode()
{
	auto n = std::make_unique<IDNode>(hash.hashString(currentToken.getValueString()));
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseEntryNode()
{
	std::vector<std::unique_ptr<Node>> v;
	while (currentToken.getType() != 0)
	{
		if (auto n = parseExprNode())
			v.push_back(std::move(n));
		else
			return logErrorN("Expected interior code for global entry", currentToken);
	}
	return std::make_unique<VectorNode>(std::move(v));
}

std::unique_ptr<Node> NodeParser::parseTrailingNode(std::unique_ptr<Node> ret, bool allowInner)
{
	switch (currentToken.getType())
	{
	case TOK_UNTIL:
		return parseUntilNode(std::move(ret));
	case TOK_CAST:
		return parseCastToNode(std::move(ret));
	case '.':
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
	LEX_TOKEN_TYPE t = (LEX_TOKEN_TYPE)currentToken.getType();
	nextToken();
	if (currentToken.getType() != '(')
		return logErrorN("Expected `(`", currentToken);
	nextToken();
	auto arg = parseEquNode();
	if (currentToken.getType() != ')')
		return logErrorN("Expected `)`", currentToken);
	nextToken();
	auto ret = std::make_unique<CallBuiltNode>(t, std::move(arg));
	return parseTrailingNode(std::move(ret), true);
}

std::unique_ptr<Node> NodeParser::parseCallNode(std::unique_ptr<Node> a)
{
	nextToken();
	std::vector<std::unique_ptr<Node>> args;
	int i = 0;
	while (currentToken.getType() != ')')
	{
		if (i > 0)
		{
			if (currentToken.getType() != ',')
				return logErrorN("Expected `,`", currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(std::move(b));
		else
			return logErrorN("Expected Function parameter", currentToken);
		if (currentToken.getType() == 0)
			return logErrorN("Expected `)`", currentToken);
	}
	nextToken();
	auto ret = std::make_unique<CallNode>(std::move(a), std::move(args));
	return parseTrailingNode(std::move(ret), true);
}

std::unique_ptr<Node> NodeParser::parseExternCallNode()
{
	nextToken();
	std::string libname = currentToken.getValueString();
	nextToken();
	if (currentToken.getType() != '.')
		return logErrorN("Expected `.`", currentToken);
	nextToken();
	std::string fname = currentToken.getValueString();
	nextToken();

	if (currentToken.getType() != '(')
		return logErrorN("Expected `(`", currentToken);
	nextToken();

	std::vector<std::unique_ptr<Node>> args;
	int i = 0;
	while (currentToken.getType() != ')')
	{
		if (i > 0)
		{
			if (currentToken.getType() != ',')
				return logErrorN("Expected `,`", currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(std::move(b));
		else
			return logErrorN("Expected Function parameter", currentToken);
		if (currentToken.getType() == 0)
			return logErrorN("Expected `)`", currentToken);
	}
	nextToken();

	auto f = rlib::loaded.at(libname + "$" + fname);

	return std::make_unique<ExternCallNode>(libname + "$" + fname, std::move(args));
}

std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> NodeParser::parseSigNode()
{
	if (currentToken.getType() != '(')
		return logErrorSN("Expected `(`", currentToken);
	nextToken();

	std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> args;
	int i = 0;
	while (currentToken.getType() != ')')
	{
		if (i > 0)
		{
			if (currentToken.getType() != ',')
				return logErrorSN("Expected `,`", currentToken);
			nextToken();
		}
		i++;

		int type = currentToken.getType();
		if (!(type == TOK_FINAL || type == TOK_REF))
			type = TOK_NIL;
		else
			nextToken();

		std::string arg;

		if (currentToken.getType() == TOK_IDF)
			arg = currentToken.getValueString();
		else
			return logErrorSN("Expected variable identifier", currentToken);
		nextToken();

		args.push_back({(LEX_TOKEN_TYPE)type, hash.hashString(arg)});

		if (currentToken.getType() == 0)
			return logErrorSN("Expected `)`", currentToken);
	}
	nextToken();
	return args;
}

std::unique_ptr<Node> NodeParser::parseDefineNode()
{
	nextToken();

	D_TYPE ftype = NIL;
	if (currentToken.getType() != TOK_IDF && currentToken.getType() != TOK_LENGTH && currentToken.getType() != TOK_SIZE && currentToken.getType() != TOK_ALLOC)
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
			return logErrorN("Expected base type for prototypic Function declaration", currentToken);
		}
		nextToken();
		if (currentToken.getType() != TOK_DEF_TYPE)
			return logErrorN("Expected `::`", currentToken);
		nextToken();
	}
	if (currentToken.getType() != TOK_IDF && currentToken.getType() != TOK_LENGTH && currentToken.getType() != TOK_SIZE && currentToken.getType() != TOK_ALLOC)
		return logErrorN("Expected Function name", currentToken);
	auto key = hash.hashString(currentToken.getValueString());
	nextToken();

	auto args = parseSigNode();
	if (!args.empty() && args[0].first == 0)
		return logErrorN("Expected Function signature", currentToken);

	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();

	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN("Expected Function body", currentToken);
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();
	return std::make_unique<DefineNode>(key, ftype, args, std::move(body));
}

std::unique_ptr<Node> NodeParser::parseNewNode()
{
	nextToken();

	auto body = parseUnitNode();
	if (body->getType() != CALL_NODE)
		return logErrorN("Expected Object declaration", currentToken);
	auto object = ((CallNode *)body.get())->getCallee();
	auto params = std::make_unique<VectorNode>(std::move(((CallNode *)body.get())->getArgs()));

	return std::make_unique<NewNode>(std::move(object), std::move(params));
}

std::unique_ptr<Node> NodeParser::parseLambdaNode()
{
	nextToken();

	auto args = parseSigNode();
	if (!args.empty() && args[0].first == 0)
		return logErrorN("Expected Function signature", currentToken);

	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();

	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN("Expected Function body", currentToken);
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();
	return std::make_unique<DefineNode>(0, NIL, args, std::move(body));
}

std::unique_ptr<Node> NodeParser::parseIndexNode(std::unique_ptr<Node> a)
{
	nextToken();
	if (auto b = parseEquNode())
	{
		if (currentToken.getType() != ']')
			return logErrorN("Expected `]`", currentToken);
		nextToken();
		auto ret = std::make_unique<IndexNode>(std::move(a), std::move(b));
		return parseTrailingNode(std::move(ret), true);
	}
	return nullptr;
}

std::unique_ptr<Node> NodeParser::parseCastToNode(std::unique_ptr<Node> a)
{
	nextToken();

	D_TYPE convert;
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
		return logErrorN("Cannot cast to non-base value", currentToken);
	}

	nextToken();
	auto ret = std::make_unique<CastToNode>(convert, std::move(a));
	return parseTrailingNode(std::move(ret), true);
}

std::unique_ptr<Node> NodeParser::parseUntilNode(std::unique_ptr<Node> a)
{
	nextToken();

	auto b = parseEquNode();
	if (!b)
		return logErrorN("Expected expression", currentToken);

	return std::make_unique<UntilNode>(std::move(a), std::move(b));
}

std::unique_ptr<Node> NodeParser::parseBinOpNode(std::unique_ptr<Node> a)
{
	std::unique_ptr<Node> current = std::move(a);
	int pastPrec = 999;

	while (bOperators.find(currentToken.getValueString()) != bOperators.end())
	{
		std::string opStr = currentToken.getValueString();
		int prec = bOperators[opStr];
		nextToken();

		if (auto b = parseUnitNode())
		{
			if (prec <= pastPrec)
			{
				current = std::make_unique<BinOpNode>(
					opStr,
					std::move(current),
					std::move(b));
				pastPrec = prec;
			}
			else
			{
				auto oldOp = ((BinOpNode *)current.get())->getOp();
				auto current_a = ((BinOpNode *)current.get())->getA();
				auto current_b = ((BinOpNode *)current.get())->getB();
				current = std::make_unique<BinOpNode>(
					oldOp,
					std::move(current_a),
					std::make_unique<BinOpNode>(
						opStr,
						std::move(current_b),
						std::move(b)));
				pastPrec = bOperators[oldOp];
			}
		}
		else
		{
			return logErrorN("Expected right-hand expression", currentToken);
		}
	}

	return current;
}

std::unique_ptr<Node> NodeParser::parseBaseNode()
{
	std::unique_ptr<Node> ret;
	switch (currentToken.getType())
	{
	case TOK_NUM:
		return parseNumNode();
	case TOK_IDF:
		return parseIDNode();
	case TOK_VAR:
		nextToken();
		ret = std::make_unique<VarNode>(hash.hashString(currentToken.getValueString()));
		nextToken();
		return ret;
	case TOK_STR_LIT:
		ret = std::make_unique<StringNode>(currentToken.getValueString());
		nextToken();
		return ret;
	case TOK_NIL:
		ret = std::make_unique<NilNode>();
		nextToken();
		return ret;
	case TOK_TRUE:
	case TOK_FALSE:
		return parseBoolNode();
	case '@':
		return parseLambdaNode();
	case '[':
		return parseVectorNode();
	case '{':
		return parseMapNode();
	case TOK_NIL_NAME:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(NIL)));
		nextToken();
		return ret;
	case TOK_NUMBER:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(NUMBER)));
		nextToken();
		return ret;
	case TOK_STRING:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(STRING)));
		nextToken();
		return ret;
	case TOK_BOOLEAN:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(BOOLEAN_D)));
		nextToken();
		return ret;
	case TOK_VECTOR:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(VECTOR)));
		nextToken();
		return ret;
	case TOK_DICTIONARY:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(DICTIONARY)));
		nextToken();
		return ret;
	case TOK_OBJECT:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(OBJECT)));
		nextToken();
		return ret;
	case TOK_FUNCTION:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(FUNCTION)));
		nextToken();
		return ret;
	case TOK_TYPE_NAME:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(TYPE_NAME)));
		nextToken();
		return ret;
	case TOK_POINTER:
		ret = std::make_unique<ContainerNode>(Symbol(static_cast<signed long long>(POINTER)));
		nextToken();
		return ret;
	default:
		return nullptr;
	}
}

std::unique_ptr<Node> NodeParser::parseUnOpNode()
{
	std::string opStr = currentToken.getValueString();
	nextToken();
	if (uOperators.find(opStr) != uOperators.end())
	{
		if (auto a = parseEquNode())
		{
			return std::make_unique<UnOpNode>(opStr, std::move(a));
		}
		return logErrorN("Unable to interpret expression", currentToken);
	}
	else
	{
		return logErrorN("Unknown unary operator", currentToken);
	}
}

std::unique_ptr<Node> NodeParser::parseVectorNode()
{
	nextToken();
	std::vector<std::unique_ptr<Node>> args;
	int i = 0;
	while (currentToken.getType() != ']')
	{
		if (i > 0)
		{
			if (currentToken.getType() != ',')
				return logErrorN("Expected `,`", currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode())
			args.push_back(std::move(b));
		else
			return nullptr;
		if (currentToken.getType() == 0)
			return logErrorN("Expected `]`", currentToken);
	}
	nextToken();
	return std::make_unique<VectorNode>(std::move(args));
}

std::unique_ptr<Node> NodeParser::parseMapNode()
{
	nextToken();
	std::vector<std::pair<hashcode_t, std::unique_ptr<Node>>> args;
	int i = 0;
	while (currentToken.getType() != '}')
	{
		if (i > 0)
		{
			if (currentToken.getType() != ',')
				return logErrorN("Expected `,`", currentToken);
			nextToken();
		}
		i++;
		if (!(currentToken.getType() == TOK_STR_LIT || currentToken.getType() == TOK_IDF || currentToken.getType() == TOK_NUM || currentToken.getType() == TOK_TRUE || currentToken.getType() == TOK_FALSE))
			return logErrorN("Value cannot be used to generate key-value pair", currentToken);
		auto key = currentToken.getValueString();
		nextToken();
		if (currentToken.getType() != ':')
			return logErrorN("Expected `:`", currentToken);
		nextToken();
		if (auto b = parseEquNode())
		{
			std::pair<hashcode_t, std::unique_ptr<Node>> p = {hash.hashString(key), std::move(b)};
			args.push_back(std::move(p));
		}
		else
			return nullptr;
		if (currentToken.getType() == 0)
			return logErrorN("Expected `}`", currentToken);
	}
	nextToken();
	return std::make_unique<MapNode>(std::move(args));
}

std::unique_ptr<Node> NodeParser::parseUnitNode()
{
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
		ret = parseBaseNode();
		return parseTrailingNode(std::move(ret), true);
	case TOK_EXTERN_CALL:
		return parseExternCallNode();
	case TOK_LENGTH:
	case TOK_SIZE:
	case TOK_ALLOC:
		return parseCallBuiltNode();
	case TOK_NEW:
		return parseNewNode();
	case TOK_OPR:
		return parseUnOpNode();
	case '(':
		nextToken();
		if (ret = parseEquNode())
		{
			if (currentToken.getType() != ')')
				return logErrorN("Expected `)`", currentToken);
			nextToken();
			return parseTrailingNode(std::move(ret), true);
		}
		return logErrorN("Expected interior code", currentToken);
	default:
		if (ret = parseEquNode())
		{
			return parseTrailingNode(std::move(ret), false);
		}
		return logErrorN("Expected expression", currentToken);
	}
}

std::unique_ptr<Node> NodeParser::parseInsNode(std::unique_ptr<Node> ret)
{
	while (currentToken.getType() == '.')
	{
		nextToken();
		ret = std::make_unique<InsNode>(std::move(ret), parseIDNode());
	}

	return parseTrailingNode(std::move(ret), false);
}

std::unique_ptr<Node> NodeParser::parseEquNode()
{
	if (auto ret = parseUnitNode())
	{
		switch (currentToken.getType())
		{
		case TOK_OPR:
			return parseBinOpNode(std::move(ret));
		default:
			return ret;
		}
	}
	return logErrorN("Expected expression", currentToken);
}

std::unique_ptr<Node> NodeParser::parseIfElseNode()
{
	nextToken();
	auto ifs = parseEquNode();
	if (!ifs)
		return nullptr;
	if (currentToken.getType() != TOK_THEN)
		return logErrorN("Expected `then`", currentToken);
	nextToken();
	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN("Expected interior code for `then` statement", currentToken);
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();

	auto ret = std::make_unique<IfElseNode>(std::move(ifs), std::move(body));

	switch (currentToken.getType())
	{
	case TOK_ELSE:
	{
		nextToken();
		if (currentToken.getType() != '{')
			return logErrorN("Expected `{`", currentToken);
		nextToken();
		std::vector<std::unique_ptr<Node>> elses;
		while (currentToken.getType() != 0)
		{
			if (currentToken.getType() == '}')
				break;

			if (auto e = parseExprNode())
				elses.push_back(std::move(e));
			else
				return logErrorN("Expected interior code for `else` statement", currentToken);
		}
		if (currentToken.getType() != '}')
			return logErrorN("Expected `}`", currentToken);
		nextToken();
		ret->setElse(std::make_unique<VectorNode>(std::move(elses)));
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

std::unique_ptr<Node> NodeParser::parseWhileNode()
{
	nextToken();
	auto ifs = parseEquNode();
	if (!ifs)
		return nullptr;
	if (currentToken.getType() != TOK_DO)
		return logErrorN("Expected `do`", currentToken);
	nextToken();
	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN("Expected interior code for `do` statement", currentToken);
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();

	return std::make_unique<WhileNode>(std::move(ifs), std::move(body));
}

std::unique_ptr<Node> NodeParser::parseForNode()
{
	nextToken();
	auto id = hash.hashString(currentToken.getValueString());
	nextToken();
	if (currentToken.getType() != TOK_IN)
		return logErrorN("Expected `in`", currentToken);
	nextToken();
	auto fors = parseEquNode();
	if (currentToken.getType() != TOK_DO)
		return logErrorN("Expected `do`", currentToken);
	nextToken();
	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN("Expected interior code for `do` statement", currentToken);
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();

	return std::make_unique<ForNode>(id, std::move(fors), std::move(body));
}

std::unique_ptr<Node> NodeParser::parseExternNode()
{
	nextToken();
	std::string fname = currentToken.getValueString();
	nextToken();

	if (currentToken.getType() != TOK_IN)
		return logErrorN("Expected `in`", currentToken);
	nextToken();

	std::string libname = currentToken.getValueString();

	nextToken();
	if (currentToken.getType() != ';')
		return logErrorN("Expected `;`", currentToken);
	nextToken();

	rlib::loadFunction(currentDir, libname, fname);
	return std::make_unique<NilNode>();
}

std::unique_ptr<Node> NodeParser::parseClassNode()
{
	auto type = currentToken.getType();
	nextToken();
	auto key = hash.hashString(currentToken.getValueString());
	nextToken();
	if (currentToken.getType() != TOK_CLASS)
		return logErrorN("Expected `class`", currentToken);
	nextToken();
	std::unique_ptr<Node> extends = nullptr;
	if (currentToken.getType() == ':')
	{
		nextToken();
		extends = parseUnitNode();
	}
	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();
	std::vector<std::unique_ptr<Node>> body;
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;

		if (auto e = parseExprNode())
			body.push_back(std::move(e));
		else
			return logErrorN("Expected interior code for class declaration", currentToken);
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();

	return std::make_unique<ClassNode>(key, type, std::move(body), std::move(extends));
}

std::unique_ptr<Node> NodeParser::parseLoadNode()
{
	nextToken();
	if (currentToken.getType() != TOK_STR_LIT)
		return logErrorN("Expected filepath after `load`", currentToken);
	std::string filename = currentToken.getValueString();
	nextToken();

	auto path = rdir::findFile(currentDir, filename);

	if (std::find(rdir::loaded.begin(), rdir::loaded.end(), path) != rdir::loaded.end())
	{
		if (currentToken.getType() != ';')
			return logErrorN("Expected `;`", currentToken);
		nextToken();
		return std::make_unique<NilNode>();
	}

	rdir::loaded.push_back(path);

	std::string content = "";
	std::string line;
	std::ifstream myfile(path.string());
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			content += line + "\n";
		}
		myfile.close();
	}

	auto tokens = Ruota::lexer.lexString(content);
	NodeParser np(tokens, this->bOperators, this->uOperators, path.parent_path());
	auto n = np.parse();

	if (currentToken.getType() != ';')
		return logErrorN("Expected `;`", currentToken);
	nextToken();

	return n;
}

std::unique_ptr<Node> NodeParser::parseExprNode()
{
	switch (currentToken.getType())
	{
	case ';':
		return logErrorN("Expected expression", currentToken);
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
	case TOK_STRUCT:
	case TOK_STATIC:
	case TOK_VIRTUAL:
		return parseClassNode();
	case TOK_BREAK:
		nextToken();
		if (currentToken.getType() != ';')
			return logErrorN("Expected `;`", currentToken);
		nextToken();
		return std::make_unique<BreakNode>();
	case TOK_RETURN:
		nextToken();
		if (auto ret = parseEquNode())
		{
			if (currentToken.getType() != ';')
				return logErrorN("Expected `;`", currentToken);
			nextToken();
			return std::make_unique<ReturnNode>(std::move(ret));
		}
		return nullptr;
	case TOK_REFER:
		nextToken();
		if (auto ret = parseEquNode())
		{
			if (currentToken.getType() != ';')
				return logErrorN("Expected `;`", currentToken);
			nextToken();
			return std::make_unique<ReferNode>(std::move(ret));
		}
		return nullptr;
	default:
		if (auto ret = parseEquNode())
		{
			if (currentToken.getType() != ';')
				return logErrorN("Expected `;`", currentToken);
			nextToken();
			return ret;
		}
		return nullptr;
	}
}

Instruction *VectorNode::genParser() const
{
	std::vector<Instruction *> ins;
	for (auto &n : args)
		ins.push_back(n->genParser());
	return new Sequence(ins);
}

Instruction *NumNode::genParser() const
{
	return new Container(Symbol(numberValue).setMutable(false));
}

Instruction *NilNode::genParser() const
{
	return new Container(Symbol().setMutable(false));
}

Instruction *StringNode::genParser() const
{
	return new Container(Symbol(stringValue).setMutable(false));
}

Instruction *BoolNode::genParser() const
{
	return new Container(Symbol(boolValue).setMutable(false));
}

Instruction *IDNode::genParser() const
{
	return new VariableI(key);
}

Instruction *VarNode::genParser() const
{
	return new DeclareI(key);
}

Instruction *DefineNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto fbody = std::make_shared<ScopeI>(is);
	return new DefineI(key, ftype, params, fbody);
}

Instruction *CallNode::genParser() const
{
	auto fcallee = callee->genParser();
	std::vector<Instruction *> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return new CallI(fcallee, new Sequence(fargs));
}

Instruction *IndexNode::genParser() const
{
	return new IndexI(callee->genParser(), arg->genParser());
}

Instruction *BreakNode::genParser() const
{
	return new Container(Symbol(ID_BREAK).setMutable(false));
}

Instruction *BinOpNode::genParser() const
{
	if (op == "+")
		return new AddI(a->genParser(), b->genParser());
	if (op == "-")
		return new SubI(a->genParser(), b->genParser());
	if (op == "*")
		return new MulI(a->genParser(), b->genParser());
	if (op == "/")
		return new DivI(a->genParser(), b->genParser());
	if (op == "%")
		return new ModI(a->genParser(), b->genParser());
	if (op == "**")
		return new PowI(a->genParser(), b->genParser());

	if (op == "+=")
		return new SetI(a->genParser(), new AddI(a->genParser(), b->genParser()));
	if (op == "-=")
		return new SetI(a->genParser(), new SubI(a->genParser(), b->genParser()));
	if (op == "*=")
		return new SetI(a->genParser(), new MulI(a->genParser(), b->genParser()));
	if (op == "/=")
		return new SetI(a->genParser(), new DivI(a->genParser(), b->genParser()));
	if (op == "%=")
		return new SetI(a->genParser(), new ModI(a->genParser(), b->genParser()));
	if (op == "**=")
		return new SetI(a->genParser(), new PowI(a->genParser(), b->genParser()));

	if (op == "=")
		return new SetI(a->genParser(), b->genParser());

	if (op == "<")
		return new LessI(a->genParser(), b->genParser());
	if (op == ">")
		return new MoreI(a->genParser(), b->genParser());
	if (op == "<=")
		return new ELessI(a->genParser(), b->genParser());
	if (op == ">=")
		return new EMoreI(a->genParser(), b->genParser());
	if (op == "==")
		return new Equals(a->genParser(), b->genParser());
	if (op == "!=")
		return new NEquals(a->genParser(), b->genParser());
	if (op == "&&")
		return new AndI(a->genParser(), b->genParser());
	if (op == "||")
		return new OrI(a->genParser(), b->genParser());

	throw std::runtime_error("Unknown binary operator: " + op);
}

Instruction *UnOpNode::genParser() const
{
	if (op == "+")
		return a->genParser();
	if (op == "-")
		return new SubI(std::make_unique<NumNode>((long_int_t)0)->genParser(), a->genParser());
	if (op == "!")
		return new Equals(std::make_unique<BoolNode>(false)->genParser(), a->genParser());

	throw std::runtime_error("Unknown unary operator: " + op);
}

Instruction *CallBuiltNode::genParser() const
{
	switch (t)
	{
	case TOK_SIZE:
		return new SizeI(arg->genParser());
	case TOK_LENGTH:
		return new LengthI(arg->genParser());
	case TOK_ALLOC:
		return new AllocI(arg->genParser());
	default:
		break;
	}

	throw std::runtime_error("Unknown built in function: " + std::to_string(t));
}

Instruction *InsNode::genParser() const
{
	return new InnerI(callee->genParser(), arg->genParser());
}

Instruction *IfElseNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = new ScopeI(is);
	if (elses)
		return new IfElseI(ifs->genParser(), bodyI, elses->genParser());
	return new IfElseI(ifs->genParser(), bodyI, NULL);
}

Instruction *MapNode::genParser() const
{
	std::map<hashcode_t, Instruction *> is;
	for (auto &e : this->args)
	{
		is[e.first] = e.second->genParser();
	}
	return new MapI(is);
}

Instruction *NewNode::genParser() const
{
	auto paramsI = params->genParser();
	return new NewI(object->genParser(), paramsI);
}

Instruction *ClassNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = std::make_shared<ScopeI>(is);

	OBJECT_TYPE ot;
	switch (type)
	{
	case TOK_STRUCT:
		ot = STRUCT_O;
		break;
	case TOK_STATIC:
		ot = STATIC_O;
		break;
	case TOK_VIRTUAL:
		ot = VIRTUAL_O;
		break;
	default:
		throw std::runtime_error("Invalid Object type");
	}

	if (extends == nullptr)
		return new ClassI(key, ot, bodyI, NULL);
	else
		return new ClassI(key, ot, bodyI, extends->genParser());
}

Instruction *WhileNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = new ScopeI(is);

	return new WhileI(whiles->genParser(), bodyI);
}

Instruction *ExternCallNode::genParser() const
{
	std::vector<Instruction *> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return new ExternI(id, new Sequence(fargs));
}

Instruction *ForNode::genParser() const
{
	std::vector<Instruction *> is;
	for (auto &e : this->body)
		is.push_back(e->genParser());
	auto bodyI = new ScopeI(is);

	return new ForI(id, fors->genParser(), bodyI);
}

std::unique_ptr<Node> NodeParser::parse()
{
	nextToken();
	return parseEntryNode();
}

Instruction *NodeParser::genParser(std::unique_ptr<Node> n)
{
	return n->genParser();
}

Instruction *ReturnNode::genParser() const
{
	return new ReturnI(a->genParser());
}

Instruction *ReferNode::genParser() const
{
	return new ReferI(a->genParser());
}

Instruction *CastToNode::genParser() const
{
	return new CastToI(a->genParser(), convert);
}

Instruction *UntilNode::genParser() const
{
	return new UntilI(a->genParser(), b->genParser());
}

Instruction *ContainerNode::genParser() const
{
	return new Container(s);
}

std::unique_ptr<Node> NodeParser::logErrorN(const std::string &s, Token t)
{
	//std::string o = s + "\n\t" + t.getLine() + "\n\t";
	std::cout << s << "\n"
			  << t.getLine() << "\n";
	for (size_t i = 0; i < t.getDist() - t.getValueString().size(); i++)
		std::cout << " ";
	std::cout << "^";
	if (t.getValueString().size() > 0)
		for (size_t i = 0; i < t.getValueString().size() - 1; i++)
			std::cout << "~";
	std::cout << "\n";
	return nullptr;
}

std::vector<std::pair<LEX_TOKEN_TYPE, hashcode_t>> NodeParser::logErrorSN(const std::string &s, Token t)
{
	logErrorN(s, t);
	return {{(LEX_TOKEN_TYPE)0, -1}};
}