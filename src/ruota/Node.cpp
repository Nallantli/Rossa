#include "Node.hpp"

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
	auto n = std::make_unique<IDNode>(currentToken.getValueString());
	nextToken();
	return n;
}

std::unique_ptr<Node> NodeParser::parseEntryNode()
{
	std::unique_ptr<EntryNode> entry = std::make_unique<EntryNode>();
	while (currentToken.getType() != 0)
	{
		if (auto n = parseExprNode())
		{
			entry->addNode(std::move(n));
		}
		else
		{
			return nullptr;
		}
	}
	return entry;
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
			return nullptr;
		if (currentToken.getType() == 0)
			return logErrorN("Expected `)`", currentToken);
	}
	nextToken();
	auto ret = std::make_unique<CallNode>(std::move(a), std::move(args));
	switch (currentToken.getType())
	{
	case '.':
		return parseInsNode(std::move(ret));
	case '(':
		return parseCallNode(std::move(ret));
	case '[':
		return parseIndexNode(std::move(ret));
	default:
		return ret;
	}
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
			return nullptr;
		if (currentToken.getType() == 0)
			return logErrorN("Expected `)`", currentToken);
	}
	nextToken();

	auto f = rlib::loaded.at(libname + "$" + fname);

	return std::make_unique<ExternCallNode>(f, std::move(args));
}

std::unique_ptr<Node> NodeParser::parseSigNode()
{
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
		if (auto b = parseIDNode())
			args.push_back(std::move(b));
		else
			return nullptr;
		if (currentToken.getType() == 0)
			return logErrorN("Expected `)`", currentToken);
	}
	nextToken();
	return std::make_unique<VectorNode>(std::move(args));
}

std::unique_ptr<Node> NodeParser::parseDefineNode()
{
	nextToken();
	auto keyNode = parseIDNode();
	if (!keyNode)
		logErrorN("Expected identifier", currentToken);

	auto args = parseSigNode();

	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();
	auto body = std::make_unique<EntryNode>();
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;
		if (auto e = parseExprNode())
		{
			body->addNode(std::move(e));
		}
		else
		{
			return nullptr;
		}
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();
	return std::make_unique<DefineNode>(((IDNode *)(keyNode.get()))->getKey(), std::move(args), std::move(body));
}

std::unique_ptr<Node> NodeParser::parseLambdaNode()
{
	nextToken();

	auto args = parseSigNode();

	if (currentToken.getType() != '{')
		return logErrorN("Expected `{`", currentToken);
	nextToken();
	auto body = std::make_unique<EntryNode>();
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;
		if (auto e = parseExprNode())
		{
			body->addNode(std::move(e));
		}
		else
		{
			return nullptr;
		}
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();
	return std::make_unique<DefineNode>("", std::move(args), std::move(body));
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
		switch (currentToken.getType())
		{
		case '.':
			return parseInsNode(std::move(ret));
		case '(':
			return parseCallNode(std::move(ret));
		case '[':
			return parseIndexNode(std::move(ret));
		default:
			return ret;
		}
	}
	return nullptr;
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

		if (prec < pastPrec)
		{
			current = std::make_unique<BinOpNode>(
				opStr,
				std::move(current),
				parseUnitNode());
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
					parseUnitNode()));
		}
		pastPrec = prec;
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
		ret = std::make_unique<VarNode>(currentToken.getValueString());
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

std::unique_ptr<Node> NodeParser::parseUnitNode()
{
	std::unique_ptr<Node> ret = nullptr;
	switch (currentToken.getType())
	{
	case TOK_NUM:
	case TOK_IDF:
	case TOK_VAR:
	case TOK_TRUE:
	case TOK_FALSE:
	case TOK_NIL:
	case TOK_STR_LIT:
	case '@':
	case '[':
		ret = parseBaseNode();

		switch (currentToken.getType())
		{
		case '.':
			return parseInsNode(std::move(ret));
		case '(':
			return parseCallNode(std::move(ret));
		case '[':
			return parseIndexNode(std::move(ret));
		default:
			return ret;
		}
	case TOK_EXTERN_CALL:
		return parseExternCallNode();
	case TOK_OPR:
		return parseUnOpNode();
	case '(':
		nextToken();
		if (ret = parseEquNode())
		{
			if (currentToken.getType() != ')')
				return logErrorN("Expected `)`", currentToken);
			nextToken();

			switch (currentToken.getType())
			{
			case '.':
				return parseInsNode(std::move(ret));
			case '(':
				return parseCallNode(std::move(ret));
			case '[':
				return parseIndexNode(std::move(ret));
			default:
				return ret;
			}
		}
		else
		{
			return logErrorN("Expected interior code", currentToken);
		}
	default:
		ret = parseEquNode();

		switch (currentToken.getType())
		{
		case '(':
			return parseCallNode(std::move(ret));
		case '[':
			return parseIndexNode(std::move(ret));
		default:
			return ret;
		}
	}
}

std::unique_ptr<Node> NodeParser::parseInsNode(std::unique_ptr<Node> ret)
{
	while (currentToken.getType() == '.')
	{
		nextToken();
		ret = std::make_unique<InsNode>(std::move(ret), parseIDNode());
	}

	switch (currentToken.getType())
	{
	case '(':
		return parseCallNode(std::move(ret));
	case '[':
		return parseIndexNode(std::move(ret));
	default:
		return ret;
	}
}

std::unique_ptr<Node> NodeParser::parseEquNode()
{
	auto ret = parseUnitNode();

	switch (currentToken.getType())
	{
	case TOK_OPR:
		return parseBinOpNode(std::move(ret));
	default:
		return ret;
	}
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
	auto body = std::make_unique<EntryNode>();
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;
		if (auto e = parseExprNode())
		{
			body->addNode(std::move(e));
		}
		else
		{
			return nullptr;
		}
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
		auto elses = std::make_unique<EntryNode>();
		while (currentToken.getType() != 0)
		{
			if (currentToken.getType() == '}')
				break;
			if (auto e = parseExprNode())
			{
				elses->addNode(std::move(e));
			}
			else
			{
				return nullptr;
			}
		}
		if (currentToken.getType() != '}')
			return logErrorN("Expected `}`", currentToken);
		nextToken();
		ret->setElse(std::move(elses));
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
	auto body = std::make_unique<EntryNode>();
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;
		if (auto e = parseExprNode())
		{
			body->addNode(std::move(e));
		}
		else
		{
			return nullptr;
		}
	}
	if (currentToken.getType() != '}')
		return logErrorN("Expected `}`", currentToken);
	nextToken();

	return std::make_unique<WhileNode>(std::move(ifs), std::move(body));
}

std::unique_ptr<Node> NodeParser::parseForNode()
{
	nextToken();
	auto id = currentToken.getValueString();
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
	auto body = std::make_unique<EntryNode>();
	while (currentToken.getType() != 0)
	{
		if (currentToken.getType() == '}')
			break;
		if (auto e = parseExprNode())
		{
			body->addNode(std::move(e));
		}
		else
		{
			return nullptr;
		}
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

	rlib::loadFunction(libname, fname);
	return std::make_unique<NilNode>();
}

std::unique_ptr<Node> NodeParser::parseExprNode()
{
	switch (currentToken.getType())
	{
	case ';':
		return logErrorN("Expected expression", currentToken);
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

Instruction *EntryNode::genParser() const
{
	std::vector<Instruction *> ins;
	for (auto &n : children)
		ins.push_back(n->genParser());
	return new Sequence(true, ins);
}

Instruction *VectorNode::genParser() const
{
	std::vector<Instruction *> ins;
	for (auto &n : args)
		ins.push_back(n->genParser());
	return new Sequence(false, ins);
}

Instruction *NumNode::genParser() const
{
	return new Container(manager::newValue(numberValue));
}

Instruction *NilNode::genParser() const
{
	return new Container(manager::newValue());
}

Instruction *StringNode::genParser() const
{
	return new Container(manager::newValue(stringValue));
}

Instruction *BoolNode::genParser() const
{
	return new Container(manager::newValue(boolValue));
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
	std::shared_ptr<Instruction> fbody(this->body->genParser());
	std::vector<std::string> args;
	for (auto &e : ((VectorNode *)params.get())->getChildren())
		args.push_back(((IDNode *)e.get())->getKey());
	return new DefineI(key, args, fbody);
}

Instruction *CallNode::genParser() const
{
	auto fcallee = callee->genParser();
	std::vector<Instruction *> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return new CallI(fcallee, new Sequence(false, fargs));
}

Instruction *IndexNode::genParser() const
{
	return new IndexI(callee->genParser(), arg->genParser());
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
		return new SubI(std::make_unique<NumNode>(0)->genParser(), a->genParser());
	if (op == "!")
		return new Equals(std::make_unique<BoolNode>(false)->genParser(), a->genParser());

	throw std::runtime_error("Unknown unary operator: " + op);
}

Instruction *InsNode::genParser() const
{
	return new InnerI(callee->genParser(), arg->genParser());
}

Instruction *IfElseNode::genParser() const
{
	auto bodyI = (Sequence *)body->genParser();
	bodyI->setScoped(false);
	if (elses)
		return new IfElseI(ifs->genParser(), bodyI, elses->genParser());
	return new IfElseI(ifs->genParser(), bodyI, NULL);
}

Instruction *WhileNode::genParser() const
{
	auto bodyI = (Sequence *)body->genParser();
	bodyI->setScoped(false);
	return new WhileI(whiles->genParser(), bodyI);
}

Instruction *ExternCallNode::genParser() const
{
	std::vector<Instruction *> fargs;
	for (auto &c : args)
		fargs.push_back(c->genParser());
	return new ExternI(this->f, new Sequence(false, fargs));
}

Instruction *ForNode::genParser() const
{
	return new ForI(id, fors->genParser(), body->genParser());
}

std::unique_ptr<Node> NodeParser::parse()
{
	nextToken();
	return parseEntryNode();
}

Instruction *NodeParser::genParser(std::unique_ptr<Node> n)
{
	auto g = n->genParser();
	if (g->getType() == SEQUENCE)
		((Sequence *)g)->setScoped(false);
	return g;
}

Instruction *ReturnNode::genParser() const
{
	return new ReturnI(a->genParser());
}

std::unique_ptr<Node> NodeParser::logErrorN(const std::string &s, Token t)
{
	std::cout << s << "\n"
			  << t.getLine() << "\n"
			  << t.getValueString() << "\n"
			  << t.getDist() << "\n";
	return nullptr;
}