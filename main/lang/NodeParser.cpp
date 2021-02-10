#include "Rossa.h"
#include <fstream>

NodeParser::NodeParser(
	const std::vector<token_t> &tokens,
	const std::filesystem::path &currentFile) : tokens(tokens),
	currentFile(currentFile)
{}

void NodeParser::nextToken()
{
	if (index < this->tokens.size())
		currentToken = this->tokens.at(index++);
	else
		currentToken = token_t();
}

node_ptr_t NodeParser::parseNumNode(ns_vec_t &scopes)
{
	const node_ptr_t n = std::make_shared<ContainerNode>(scopes, sym_t::Number(currentToken.valueNumber), currentToken);
	nextToken();
	return n;
}

node_ptr_t NodeParser::parseBoolNode(ns_vec_t &scopes)
{
	auto n = std::make_shared<ContainerNode>(scopes, sym_t::Boolean(currentToken.valueString == KEYWORD_TRUE), currentToken);
	nextToken();
	return n;
}

node_ptr_t NodeParser::parseIDNode(ns_vec_t &scopes)
{
	auto n = std::make_shared<IDNode>(scopes, ROSSA_HASH(currentToken.valueString), currentToken);
	nextToken();
	return n;
}

node_ptr_t NodeParser::parseBIDNode(ns_vec_t &scopes)
{
	if (currentToken.type == TOK_IDF) {
		return parseIDNode(scopes);
	}
	auto n = std::make_shared<BIDNode>(scopes, currentToken.valueString, currentToken);
	nextToken();
	return n;
}

node_ptr_t NodeParser::parseEntryNode(ns_vec_t &scopes)
{
	node_vec_t v;
	while (currentToken.type != NULL_TOK)
		v.push_back(parseExprNode(scopes));
	return std::make_shared<VectorNode>(scopes, v, false, currentToken);
}

node_ptr_t NodeParser::parseTrailingNode(ns_vec_t &scopes, const node_ptr_t &ret, const bool &allowInner)
{
	if (currentToken.type == NULL_TOK)
		return ret;

	switch (currentToken.type) {
		case TOK_INNER:
			if (allowInner)
				return parseInsNode(scopes, ret);
			else
				return ret;
		case '(':
			return parseCallNode(scopes, ret);
		case '[':
			return parseIndexNode(scopes, ret);
		default:
			return ret;
	}
}

node_ptr_t NodeParser::parseCallBuiltNode(ns_vec_t &scopes)
{
	LexerTokenType t = static_cast<LexerTokenType>(currentToken.type);
	std::string temp = currentToken.valueString;
	auto marker = currentToken;
	nextToken();
	if (currentToken.type != '(')
		return std::make_shared<IDNode>(scopes, ROSSA_HASH(temp), marker);
	nextToken();
	auto arg = parseEquNode(scopes);
	if (currentToken.type != ')')
		return logErrorN(format::format(_EXPECTED_ERROR_, { ")" }), currentToken);
	nextToken();
	auto ret = std::make_shared<CallBuiltNode>(scopes, t, arg, marker);
	return parseTrailingNode(scopes, ret, true);
}

node_ptr_t NodeParser::parseCallNode(ns_vec_t &scopes, const node_ptr_t &a)
{
	auto marker = tokens.at(index - 2);
	nextToken();
	node_vec_t args;
	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != ')') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode(scopes))
			args.push_back(b);
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.type == NULL_TOK)
			return logErrorN(format::format(_EXPECTED_ERROR_, { ")" }), currentToken);
	}

	nextToken();

	node_ptr_t ret;
	if (a->getType() == Node::type_t::INS_NODE) {
		auto a_a = reinterpret_cast<InsNode *>(a.get())->getCallee();
		auto a_b = reinterpret_cast<InsNode *>(a.get())->getArg();
		if (a_b->getType() == Node::type_t::BID_NODE) {
			auto nkey = reinterpret_cast<BIDNode *>(a_b.get());
			std::string key = nkey->getKey();
			/*if (!args.empty())
				return logErrorN("Built in functions take a single argument", currentToken);*/

			if (key == KEYWORD_LENGTH)
				ret = std::make_shared<CallBuiltNode>(scopes, TOK_LENGTH, a_a, marker);
			if (key == KEYWORD_ALLOC)
				ret = std::make_shared<CallBuiltNode>(scopes, TOK_ALLOC, a_a, marker);
			if (key == KEYWORD_PARSE)
				ret = std::make_shared<CallBuiltNode>(scopes, TOK_PARSE, a_a, marker);
			if (key == KEYWORD_CHAR_N)
				ret = std::make_shared<CallBuiltNode>(scopes, TOK_CHARN, a_a, marker);
			if (key == KEYWORD_CHAR_S)
				ret = std::make_shared<CallBuiltNode>(scopes, TOK_CHARS, a_a, marker);

			ret = parseTrailingNode(scopes, ret, true);
		} else {
			ret = std::make_shared<CallNode>(scopes, std::make_shared<InsNode>(scopes, a_a, a_b, currentToken), args, marker);
		}
		return parseTrailingNode(scopes, ret, true);
	}

	ret = std::make_shared<CallNode>(scopes, a, args, marker);
	return parseTrailingNode(scopes, ret, true);
}

node_ptr_t NodeParser::parseExternCallNode(ns_vec_t &scopes)
{
	nextToken();
	std::string libname = currentToken.valueString;
	nextToken();
	if (currentToken.type != TOK_INNER)
		return logErrorN(format::format(_EXPECTED_ERROR_, { "." }), currentToken);
	nextToken();
	std::string fname = currentToken.valueString;

	auto marker = currentToken;

	nextToken();

	if (currentToken.type != '(')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "(" }), currentToken);
	nextToken();

	node_vec_t args;
	int i = 0;
	while (currentToken.type != ')') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode(scopes))
			args.push_back(b);
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.type == NULL_TOK)
			return logErrorN(format::format(_EXPECTED_ERROR_, { ")" }), currentToken);
	}
	nextToken();

	return std::make_shared<ExternCallNode>(scopes, libname, fname, args, marker);
}

node_ptr_t NodeParser::parseCallOpNode(ns_vec_t &scopes)
{
	nextToken();
	size_t id = currentToken.valueNumber.getLong();
	auto marker = currentToken;
	nextToken();

	if (currentToken.type != '(')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "(" }), currentToken);
	nextToken();

	node_vec_t args;
	int i = 0;
	while (currentToken.type != ')') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
			nextToken();
		}
		i++;
		if (auto b = parseEquNode(scopes))
			args.push_back(b);
		else
			return logErrorN(_EXPECTED_FUNCTION_PARAM_, currentToken);
		if (currentToken.type == NULL_TOK)
			return logErrorN(format::format(_EXPECTED_ERROR_, { ")" }), currentToken);
	}
	nextToken();
	return std::make_shared<CallOpNode>(scopes, id, args, marker);
}


param_t NodeParser::parseParamTypeNode(ns_vec_t &scopes, const aug_type_t &base)
{
	if (base[0] < 0 && base[0] != Value::type_t::FUNCTION)
		return logErrorPT(format::format(_FUNCTION_PARAM_ERROR_, { getTypeString(base) }), currentToken);

	param_t pt(base);
	nextToken();

	int i = 0;
	while (currentToken.valueString != ">") {
		if (i++ > 0) {
			if (currentToken.type != ',')
				return logErrorPT(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
			nextToken();
		}
		aug_type_t qbase = { Value::type_t::ANY };
		switch (currentToken.type) {
			case TOK_BOOLEAN:
				qbase = { Value::type_t::BOOLEAN_D };
				nextToken();
				break;
			case TOK_NUMBER:
				qbase = { Value::type_t::NUMBER };
				nextToken();
				break;
			case TOK_ARRAY:
				qbase = { Value::type_t::ARRAY };
				nextToken();
				break;
			case TOK_STRING:
				qbase = { Value::type_t::STRING };
				nextToken();
				break;
			case TOK_DICTIONARY:
				qbase = { Value::type_t::DICTIONARY };
				nextToken();
				break;
			case TOK_OBJECT:
				qbase = { Value::type_t::OBJECT };
				nextToken();
				break;
			case TOK_FUNCTION:
				qbase = { Value::type_t::FUNCTION };
				nextToken();
				break;
			case TOK_POINTER:
				qbase = { Value::type_t::POINTER };
				nextToken();
				break;
			case TOK_ANY:
				nextToken();
				break;
			case TOK_TYPE_NAME:
				qbase = { Value::type_t::TYPE_NAME };
				nextToken();
				break;
			case TOK_IDF:
			{
				qbase = {};
				while (currentToken.type == TOK_IDF) {
					qbase.push_back(ROSSA_HASH(currentToken.valueString));
					nextToken();
					if (currentToken.type != TOK_INNER)
						break;
					nextToken();
				}
				break;
			}
			default:
				return logErrorPT(_EXPECTED_BASE_TYPE_, currentToken);
		}
		if (currentToken.valueString == "<") {
			pt.addQualifier(parseParamTypeNode(scopes, qbase));
		} else {
			pt.addQualifier(param_t(qbase));
		}
	}
	nextToken();
	return pt;
}

std::pair<fsig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> NodeParser::parseSigNode(ns_vec_t &scopes)
{
	if (currentToken.type != '(')
		return logErrorSN(format::format(_EXPECTED_ERROR_, { "(" }), currentToken);
	nextToken();

	if (currentToken.type == TOK_VAR_ARGS) {
		nextToken();
		if (currentToken.type != ')')
			return logErrorSN(format::format(_EXPECTED_ERROR_, { ")" }), currentToken);
		nextToken();
		return { fsig_t(), {{static_cast<LexerTokenType>(1), static_cast<hash_ull>(0)}} };
	}

	std::vector<std::pair<LexerTokenType, hash_ull>> args;
	param_vec_t types;

	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != ')') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorSN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
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

		param_t pt({ Value::type_t::ANY });
		if (currentToken.type == ':') {
			aug_type_t ftype = { Value::type_t::ANY };
			nextToken();
			switch (currentToken.type) {
				case TOK_BOOLEAN:
					ftype = { Value::type_t::BOOLEAN_D };
					nextToken();
					break;
				case TOK_NUMBER:
					ftype = { Value::type_t::NUMBER };
					nextToken();
					break;
				case TOK_ARRAY:
					ftype = { Value::type_t::ARRAY };
					nextToken();
					break;
				case TOK_STRING:
					ftype = { Value::type_t::STRING };
					nextToken();
					break;
				case TOK_DICTIONARY:
					ftype = { Value::type_t::DICTIONARY };
					nextToken();
					break;
				case TOK_OBJECT:
					ftype = { Value::type_t::OBJECT };
					nextToken();
					break;
				case TOK_FUNCTION:
					ftype = { Value::type_t::FUNCTION };
					nextToken();
					break;
				case TOK_POINTER:
					ftype = { Value::type_t::POINTER };
					nextToken();
					break;
				case TOK_ANY:
					nextToken();
					break;
				case TOK_TYPE_NAME:
					ftype = { Value::type_t::TYPE_NAME };
					nextToken();
					break;
				case TOK_IDF:
				{
					ftype = {};
					while (currentToken.type == TOK_IDF) {
						ftype.push_back(ROSSA_HASH(currentToken.valueString));
						nextToken();
						if (currentToken.type != TOK_INNER)
							break;
						nextToken();
					}
					break;
				}
				default:
					return logErrorSN(_EXPECTED_BASE_TYPE_, currentToken);
			}
			if (currentToken.valueString == "<") {
				pt = parseParamTypeNode(scopes, ftype);
			} else {
				pt = param_t(ftype);
			}
		}

		args.push_back({ static_cast<LexerTokenType>(type), ROSSA_HASH(arg) });
		types.push_back(pt);

		if (currentToken.type == NULL_TOK)
			return logErrorSN(format::format(_EXPECTED_ERROR_, { ")" }), currentToken);
	}
	nextToken();

	return { types, args };
}

node_ptr_t NodeParser::parseDefineNode(ns_vec_t &scopes)
{
	nextToken();

	std::vector<hash_ull> captures;

	if (currentToken.type == NULL_TOK || (currentToken.type != TOK_IDF && currentToken.type != TOK_LENGTH && currentToken.type != TOK_ALLOC && currentToken.type != TOK_CHARN && currentToken.type != TOK_CHARS && currentToken.type != TOK_PARSE))
		return logErrorN(_EXPECTED_FUNCTION_NAME_, currentToken);
	auto key = ROSSA_HASH(currentToken.valueString);
	scopes.push_back({ key });
	nextToken();

	bool isVargs = false;
	auto sig = parseSigNode(scopes);
	if (!sig.second.empty() && sig.second[0].first == 0)
		return logErrorN(_EXPECTED_FUNCTION_SIG_, currentToken);
	if (!sig.second.empty() && sig.second[0].first == 1)
		isVargs = true;

	if (currentToken.type != '{') {
		auto body = parseEquNode(scopes);

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		if (currentToken.type != ';')
			return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
		nextToken();

		scopes.pop_back();
		if (isVargs)
			return std::make_shared<VargDefineNode>(scopes, key, body, captures, currentToken);
		else
			return std::make_shared<DefineNode>(scopes, key, sig.first, sig.second, body, captures, currentToken);
	} else {
		node_vec_t vbody;

		nextToken();
		while (currentToken.type != NULL_TOK) {
			if (currentToken.type == NULL_TOK || currentToken.type == '}')
				break;

			if (auto e = parseExprNode(scopes))
				vbody.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.type != '}')
			return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
		nextToken();

		auto body = std::make_shared<VectorNode>(scopes, vbody, true, currentToken);
		scopes.pop_back();
		if (isVargs)
			return std::make_shared<VargDefineNode>(scopes, key, body, captures, currentToken);
		else
			return std::make_shared<DefineNode>(scopes, key, sig.first, sig.second, body, captures, currentToken);
	}
}

node_ptr_t NodeParser::parseNewNode(ns_vec_t &scopes)
{
	nextToken();

	auto body = parseUnitNode(scopes);
	if (body->getType() != Node::type_t::CALL_NODE)
		return logErrorN(_EXPECTED_OBJECT_NAME_, currentToken);
	auto object = reinterpret_cast<CallNode *>(body.get())->getCallee();
	auto params = std::make_shared<VectorNode>(scopes, (reinterpret_cast<CallNode *>(body.get())->getArgs()), false, currentToken);

	return std::make_shared<NewNode>(scopes, object, params, currentToken);
}

node_ptr_t NodeParser::parseLambdaNode(ns_vec_t &scopes)
{
	nextToken();
	scopes.push_back({ ROSSA_HASH("<LAM@" + std::to_string(scope_i++) + ">") });

	bool isVargs = false;
	auto sig = parseSigNode(scopes);
	if (!sig.second.empty() && sig.second[0].first == 0)
		return logErrorN(_EXPECTED_FUNCTION_SIG_, currentToken);
	if (!sig.second.empty() && sig.second[0].first == 1)
		isVargs = true;

	int i = 0;
	std::vector<hash_ull> captures;
	if (currentToken.type == '[') {
		nextToken();
		while (currentToken.type != ']') {
			if (i++ > 0) {
				if (currentToken.type != ',')
					return logErrorN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
				nextToken();
			}

			std::string arg;

			if (currentToken.type == TOK_IDF)
				arg = currentToken.valueString;
			else
				return logErrorN("Expected variable identifier", currentToken);
			nextToken();

			captures.push_back(ROSSA_HASH(arg));
		}
		nextToken();
	}

	if (currentToken.type != '{') {
		auto body = parseEquNode(scopes);

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		scopes.pop_back();
		if (isVargs)
			return std::make_shared<VargDefineNode>(scopes, 0, body, captures, currentToken);
		else
			return std::make_shared<DefineNode>(scopes, 0, sig.first, sig.second, body, captures, currentToken);
	} else {
		node_vec_t vbody;

		nextToken();
		while (currentToken.type != NULL_TOK) {
			if (currentToken.type == NULL_TOK || currentToken.type == '}')
				break;

			if (auto e = parseExprNode(scopes))
				vbody.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.type != '}')
			return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
		nextToken();

		auto body = std::make_shared<VectorNode>(scopes, vbody, true, currentToken);

		scopes.pop_back();
		if (isVargs)
			return std::make_shared<VargDefineNode>(scopes, 0, body, captures, currentToken);
		else
			return std::make_shared<DefineNode>(scopes, 0, sig.first, sig.second, body, captures, currentToken);
	}
}

node_ptr_t NodeParser::parseNPLambdaNode(ns_vec_t &scopes)
{
	nextToken();

	fsig_t sig;
	std::vector<std::pair<LexerTokenType, hash_ull>> p;
	std::vector<hash_ull> c;

	scopes.push_back({ ROSSA_HASH("<LM2@" + std::to_string(scope_i++) + ">") });
	if (currentToken.type != '{') {
		auto body = parseEquNode(scopes);

		if (!body)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		scopes.pop_back();
		return std::make_shared<DefineNode>(scopes, 0, sig, p, body, c, currentToken);
	} else {
		node_vec_t vbody;

		nextToken();
		while (currentToken.type != NULL_TOK) {
			if (currentToken.type == NULL_TOK || currentToken.type == '}')
				break;

			if (auto e = parseExprNode(scopes))
				vbody.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.type != '}')
			return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
		nextToken();

		scopes.pop_back();
		auto body = std::make_shared<VectorNode>(scopes, vbody, true, currentToken);
		return std::make_shared<DefineNode>(scopes, 0, sig, p, body, c, currentToken);
	}
}

node_ptr_t NodeParser::parseIndexNode(ns_vec_t &scopes, const node_ptr_t &a)
{
	auto marker = tokens.at(index - 2);
	nextToken();
	if (auto b = parseEquNode(scopes)) {
		if (currentToken.type != ']')
			return logErrorN(format::format(_EXPECTED_ERROR_, { "]" }), currentToken);
		nextToken();
		auto ret = std::make_shared<BinOpNode>(scopes, "[]", a, b, marker);
		return parseTrailingNode(scopes, ret, true);
	}
	return nullptr;
}

node_ptr_t NodeParser::parseTypeNode(ns_vec_t &scopes)
{
	nextToken();
	aug_type_t ftype = {};
	while (currentToken.type == TOK_IDF) {
		ftype.push_back(ROSSA_HASH(currentToken.valueString));
		nextToken();
		if (currentToken.type != TOK_INNER)
			break;
		nextToken();
	}
	return std::make_shared<ContainerNode>(scopes, sym_t::TypeName(ftype), currentToken);
}

node_ptr_t NodeParser::parseUntilNode(ns_vec_t &scopes, const node_ptr_t &a, const bool &inclusive)
{
	nextToken();

	auto b = parseEquNode(scopes);
	if (!b)
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

	if (currentToken.type == ':') {
		nextToken();
		auto step = parseEquNode(scopes);
		if (!step)
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

		return std::make_shared<UntilNode>(scopes, a, b, step, inclusive, currentToken);
	}

	return std::make_shared<UntilNode>(scopes, a, b, nullptr, inclusive, currentToken);
}

node_ptr_t NodeParser::parseBinOpNode(ns_vec_t &scopes, const node_ptr_t &a)
{
	node_ptr_t current = a;
	int pastPrec = 999;

	while (currentToken.type != NULL_TOK && Rossa::bOperators.find(currentToken.valueString) != Rossa::bOperators.end()) {
		std::string opStr = currentToken.valueString;

		int prec = Rossa::bOperators.at(opStr);

		auto marker = currentToken;

		nextToken();

		if (auto b = parseUnitNode(scopes)) {
			if (std::abs(prec) < std::abs(pastPrec) || (std::abs(prec) == std::abs(pastPrec) && prec > 0)) {
				current = std::make_shared<BinOpNode>(scopes,
					opStr,
					current,
					b,
					marker);
				pastPrec = prec;
			} else {
				node_ptr_t parent = nullptr;
				auto n = current;
				while (n->getType() == Node::type_t::BIN_OP_NODE) {
					auto bon = reinterpret_cast<BinOpNode *>(n.get());
					auto oldOp = bon->getOp();
					auto bon_b = bon->getB();

					if (!(std::abs(Rossa::bOperators.at(oldOp) < std::abs(prec) || (std::abs(Rossa::bOperators.at(oldOp)) == std::abs(prec) && Rossa::bOperators.at(oldOp) < 0)))) {
						reinterpret_cast<BinOpNode *>(parent.get())->setB(std::make_shared<BinOpNode>(scopes,
							opStr,
							n,
							b,
							marker
							));
						break;
					}
					if (bon_b->getType() != Node::type_t::BIN_OP_NODE) {
						bon->setB(std::make_shared<BinOpNode>(scopes,
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

node_ptr_t NodeParser::parseBaseNode(ns_vec_t &scopes)
{
	if (currentToken.type == NULL_TOK)
		logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	node_ptr_t ret;
	switch (currentToken.type) {
		case TOK_NUM:
			return parseNumNode(scopes);
		case TOK_IDF:
			return parseIDNode(scopes);
		case TOK_STR_LIT:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::String(currentToken.valueString), currentToken);
			nextToken();
			return ret;
		case TOK_NIL:
			ret = std::make_shared<ContainerNode>(scopes, sym_t(), currentToken);
			nextToken();
			return ret;
		case TOK_TRUE:
		case TOK_FALSE:
			return parseBoolNode(scopes);
		case '@':
			return parseTypeNode(scopes);
		case TOK_NO_PARAM_LAMBDA:
			return parseNPLambdaNode(scopes);
		case TOK_LAMBDA:
			return parseLambdaNode(scopes);
		case '[':
			return parseVectorNode(scopes);
		case '{':
			return parseMapNode(scopes);
		case TOK_NIL_NAME:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::NIL }), currentToken);
			nextToken();
			return ret;
		case TOK_NUMBER:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::NUMBER }), currentToken);
			nextToken();
			return ret;
		case TOK_STRING:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::STRING }), currentToken);
			nextToken();
			return ret;
		case TOK_BOOLEAN:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::BOOLEAN_D }), currentToken);
			nextToken();
			return ret;
		case TOK_ARRAY:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::ARRAY }), currentToken);
			nextToken();
			return ret;
		case TOK_DICTIONARY:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::DICTIONARY }), currentToken);
			nextToken();
			return ret;
		case TOK_OBJECT:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::OBJECT }), currentToken);
			nextToken();
			return ret;
		case TOK_FUNCTION:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::FUNCTION }), currentToken);
			nextToken();
			return ret;
		case TOK_TYPE_NAME:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::TYPE_NAME }), currentToken);
			nextToken();
			return ret;
		case TOK_POINTER:
			ret = std::make_shared<ContainerNode>(scopes, sym_t::TypeName({ Value::type_t::POINTER }), currentToken);
			nextToken();
			return ret;
		default:
			return nullptr;
	}
}

node_ptr_t NodeParser::parseUnOpNode(ns_vec_t &scopes)
{
	std::string opStr = currentToken.valueString;
	auto marker = currentToken;
	nextToken();
	if (Rossa::uOperators.find(opStr) != Rossa::uOperators.end()) {
		if (auto a = parseUnitNode(scopes))
			return std::make_shared<UnOpNode>(scopes, opStr, a, marker);
		return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	} else {
		return logErrorN(_NOT_UNARY_OP_, marker);
	}
}

node_ptr_t NodeParser::parseVectorNode(ns_vec_t &scopes)
{
	nextToken();
	node_vec_t args;
	node_vec_t curr;
	bool flag = false;
	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != ']') {
		int delim = ',';

		if (i > 0) {
			if (currentToken.type != ',' && currentToken.type != ';')
				return logErrorN(format::format(_EXPECTED_ERROR_, { ",` `;" }), currentToken);
			delim = currentToken.type;
			nextToken();
		}
		i++;

		if (delim == ';') {
			flag = true;
			args.push_back(std::make_shared<VectorNode>(scopes, curr, false, currentToken));
			curr.clear();
		}

		if (auto b = parseEquNode(scopes))
			curr.push_back((b));
		else
			return nullptr;
		if (currentToken.type == NULL_TOK)
			return logErrorN(format::format(_EXPECTED_ERROR_, { "]" }), currentToken);
	}
	nextToken();
	if (flag) {
		args.push_back(std::make_shared<VectorNode>(scopes, curr, false, currentToken));
		return std::make_shared<VectorNode>(scopes, args, false, currentToken);
	} else {
		return std::make_shared<VectorNode>(scopes, curr, false, currentToken);
	}
}

node_ptr_t NodeParser::parseMapNode(ns_vec_t &scopes)
{
	nextToken();
	std::vector<std::pair<std::string, node_ptr_t>> args;
	int i = 0;
	while (currentToken.type != NULL_TOK && currentToken.type != '}') {
		if (i > 0) {
			if (currentToken.type != ',')
				return logErrorN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
			nextToken();
		}
		i++;
		if (currentToken.type != TOK_STR_LIT)
			return logErrorN(_VALUE_KEY_PAIR_ERROR_, currentToken);
		auto key = currentToken.valueString;
		nextToken();
		if (currentToken.type != ':')
			return logErrorN(format::format(_EXPECTED_ERROR_, { ":" }), currentToken);
		nextToken();
		if (auto b = parseEquNode(scopes)) {
			std::pair<std::string, node_ptr_t> p = { key, b };
			args.push_back(p);
		} else
			return nullptr;
		if (currentToken.type == NULL_TOK)
			return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	}
	nextToken();
	return std::make_shared<MapNode>(scopes, args, currentToken);
}

node_ptr_t NodeParser::parseSwitchNode(ns_vec_t &scopes)
{
	nextToken();
	std::map<node_ptr_t, size_t> cases;
	node_vec_t gotos;

	scopes.push_back({ ROSSA_HASH("<SWI@" + std::to_string(scope_i++) + ">") });
	auto switchs = parseEquNode(scopes);
	if (!switchs)
		return logErrorN(format::format(_EXPECTED_AFTER_, { KEYWORD_SWITCH }), currentToken);
	if (currentToken.type != TOK_OF)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_OF }), currentToken);
	nextToken();
	if (currentToken.type != '{')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
	nextToken();

	while (currentToken.type != NULL_TOK && currentToken.type != '}') {
		if (currentToken.type != TOK_CASE)
			return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_CASE }), currentToken);
		nextToken();

		node_vec_t keys;

		size_t i = 0;
		while (currentToken.type != NULL_TOK) {
			if (i++ > 0) {
				if (currentToken.type != ',')
					return logErrorN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
				nextToken();
			}
			auto c = parseUnitNode(scopes);
			if (!c)
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			keys.push_back(c);
			if (currentToken.type == TOK_DO)
				break;
		}

		if (currentToken.type != TOK_DO)
			return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_DO }), currentToken);
		nextToken();

		if (currentToken.type != '{') {
			auto eq = parseEquNode(scopes);

			if (!eq)
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);

			if (currentToken.type != ';')
				return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
			nextToken();

			gotos.push_back(eq);
		} else {
			nextToken();
			node_vec_t body;
			while (currentToken.type != NULL_TOK) {
				if (currentToken.type == '}')
					break;
				if (auto e = parseExprNode(scopes))
					body.push_back(e);
				else
					return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			}
			if (currentToken.type != '}')
				return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
			nextToken();

			if (body.size() == 1)
				gotos.push_back(body[0]);
			else
				gotos.push_back(std::make_shared<VectorNode>(scopes, body, true, currentToken));
		}

		for (auto &c : keys)
			cases[c] = gotos.size();

		if (currentToken.type == NULL_TOK)
			return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	}
	nextToken();
	auto ret = std::make_shared<SwitchNode>(scopes, switchs, cases, gotos, currentToken);

	if (currentToken.type == TOK_ELSE) {
		nextToken();
		if (currentToken.type != '{')
			return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
		nextToken();
		node_vec_t elses;
		while (currentToken.type != NULL_TOK) {
			if (currentToken.type == '}')
				break;

			if (auto e = parseExprNode(scopes))
				elses.push_back(e);
			else
				return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		}
		if (currentToken.type != '}')
			return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
		nextToken();
		ret->setElse(std::make_shared<VectorNode>(scopes, elses, true, currentToken));
	}

	scopes.pop_back();
	return ret;
}

node_ptr_t NodeParser::parseUnitNode(ns_vec_t &scopes)
{
	if (currentToken.type == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	node_ptr_t ret = nullptr;
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
		case TOK_NO_PARAM_LAMBDA:
		case TOK_LAMBDA:
			ret = parseBaseNode(scopes);
			return parseTrailingNode(scopes, ret, true);
		case TOK_EXTERN_CALL:
			return parseExternCallNode(scopes);
		case TOK_CALL_OP:
			return parseCallOpNode(scopes);
		case TOK_LENGTH:
		case TOK_ALLOC:
		case TOK_CHARN:
		case TOK_CHARS:
		case TOK_PARSE:
			return parseCallBuiltNode(scopes);
		case TOK_NEW:
			return parseNewNode(scopes);
		case TOK_OPR:
			return parseUnOpNode(scopes);
		case '(':
			nextToken();
			if (ret = parseEquNode(scopes)) {
				if (currentToken.type != ')')
					return logErrorN(format::format(_EXPECTED_ERROR_, { ")" }), currentToken);
				ret = std::make_shared<ParenNode>(scopes, ret, currentToken);
				nextToken();
				return parseTrailingNode(scopes, ret, true);
			}
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
		default:
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
}

node_ptr_t NodeParser::parseInsNode(ns_vec_t &scopes, const node_ptr_t &ret)
{
	auto curr = ret;
	while (currentToken.type == TOK_INNER) {
		nextToken();
		curr = std::make_shared<InsNode>(scopes, curr, parseBIDNode(scopes), currentToken);
	}

	return parseTrailingNode(scopes, curr, false);
}

node_ptr_t NodeParser::parseThenNode(ns_vec_t &scopes, const node_ptr_t &a)
{
	scopes.push_back({ ROSSA_HASH("<IFI@" + std::to_string(scope_i++) + ">") });
	nextToken();
	auto b = parseEquNode(scopes);
	if (!b)
		return logErrorN(format::format(_EXPECTED_AFTER_, { "?" }), currentToken);
	if (currentToken.type != ':')
		return logErrorN(format::format(_EXPECTED_ERROR_, { ":" }), currentToken);
	nextToken();
	auto c = parseEquNode(scopes);
	if (!c) {
		scopes.pop_back();
		return logErrorN(format::format(_EXPECTED_AFTER_, { ":" }), currentToken);
	}
	auto ret = std::make_shared<IfElseNode>(scopes, a, b, currentToken);
	ret->setElse(c);
	scopes.pop_back();
	return ret;
}

node_ptr_t NodeParser::parseEquNode(ns_vec_t &scopes)
{
	if (currentToken.type == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	if (auto ret = parseUnitNode(scopes)) {
		if (currentToken.type != NULL_TOK) {
			switch (currentToken.type) {
				case TOK_UNTILT:
					return parseUntilNode(scopes, ret, true);
				case TOK_UNTILF:
					return parseUntilNode(scopes, ret, false);
				case TOK_OPR:
					ret = parseBinOpNode(scopes, ret);
					break;
				default:
					break;
			}
			if (currentToken.type == '?')
				return parseThenNode(scopes, ret);
		}

		return ret;
	}

	return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
}

node_ptr_t NodeParser::parseIfElseNode(ns_vec_t &scopes)
{
	scopes.push_back({ ROSSA_HASH("<IFS@" + std::to_string(scope_i++) + ">") });
	nextToken();
	auto ifs = parseEquNode(scopes);
	if (!ifs)
		return nullptr;
	if (currentToken.type != TOK_THEN)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_THEN }), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.type != '{')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
	nextToken();
	node_vec_t body;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode(scopes))
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	nextToken();

	std::shared_ptr<IfElseNode> ret;

	if (body.size() != 1)
		ret = std::make_shared<IfElseNode>(scopes, ifs, std::make_shared<VectorNode>(scopes, body, true, currentToken), marker);
	else
		ret = std::make_shared<IfElseNode>(scopes, ifs, (body[0]), marker);

	if (currentToken.type == NULL_TOK) {
		scopes.pop_back();
		return ret;
	}

	switch (currentToken.type) {
		case TOK_ELSE:
		{
			nextToken();
			if (currentToken.type != '{')
				return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
			nextToken();
			node_vec_t elses;
			while (currentToken.type != NULL_TOK) {
				if (currentToken.type == '}')
					break;

				if (auto e = parseExprNode(scopes))
					elses.push_back(e);
				else
					return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
			}
			if (currentToken.type != '}')
				return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
			nextToken();
			if (elses.size() != 1)
				ret->setElse(std::make_shared<VectorNode>(scopes, elses, true, currentToken));
			else
				ret->setElse(elses[0]);
			scopes.pop_back();
			return ret;
		}
		case TOK_ELSEIF:
		{
			ret->setElse(parseIfElseNode(scopes));
			scopes.pop_back();
			return ret;
		}
		default:
			scopes.pop_back();
			return ret;
	}
}

node_ptr_t NodeParser::parseTryCatchNode(ns_vec_t &scopes)
{
	scopes.push_back({ ROSSA_HASH("<TRY@" + std::to_string(scope_i++) + ">") });
	nextToken();
	if (currentToken.type != '{')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
	nextToken();
	node_vec_t tbody;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode(scopes))
			tbody.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	nextToken();

	if (currentToken.type != TOK_CATCH)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_CATCH }), currentToken);
	nextToken();

	if (currentToken.type != TOK_IDF)
		return logErrorN(_EXPECTED_IDF_, currentToken);
	hash_ull key = ROSSA_HASH(currentToken.valueString);
	nextToken();

	if (currentToken.type != TOK_THEN)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_THEN }), currentToken);
	nextToken();

	if (currentToken.type != '{')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
	nextToken();
	node_vec_t cbody;

	scopes.pop_back();
	scopes.push_back({ ROSSA_HASH("<CAT@" + std::to_string(scope_i++) + ">") });
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode(scopes))
			cbody.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	nextToken();

	scopes.pop_back();
	return std::make_shared<TryCatchNode>(scopes, std::make_shared<VectorNode>(scopes, tbody, true, currentToken), std::make_shared<VectorNode>(scopes, cbody, true, currentToken), key, currentToken);
}

node_ptr_t NodeParser::parseWhileNode(ns_vec_t &scopes)
{
	scopes.push_back({ ROSSA_HASH("<WHI@" + std::to_string(scope_i++) + ">") });
	nextToken();
	auto ifs = parseEquNode(scopes);
	if (!ifs)
		return nullptr;
	if (currentToken.type != TOK_DO)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_DO }), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.type != '{')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
	nextToken();
	node_vec_t body;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode(scopes))
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	nextToken();

	scopes.pop_back();
	return std::make_shared<WhileNode>(scopes, ifs, body, marker);
}

node_ptr_t NodeParser::parseForNode(ns_vec_t &scopes)
{
	scopes.push_back({ ROSSA_HASH("<FOR@" + std::to_string(scope_i++) + ">") });
	nextToken();
	auto id = ROSSA_HASH(currentToken.valueString);
	nextToken();
	if (currentToken.type != TOK_IN)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_IN }), currentToken);
	nextToken();
	auto fors = parseEquNode(scopes);
	if (currentToken.type != TOK_DO)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_DO }), currentToken);

	auto marker = currentToken;

	nextToken();
	if (currentToken.type != '{')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
	nextToken();
	node_vec_t body;
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode(scopes))
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	nextToken();

	scopes.pop_back();
	return std::make_shared<ForNode>(scopes, id, fors, body, marker);
}

node_ptr_t NodeParser::parseExternNode(ns_vec_t &scopes)
{

	nextToken();

	std::string libname = currentToken.valueString;

	nextToken();
	if (currentToken.type != ';')
		return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
	nextToken();

	lib::loadLibrary(currentFile.parent_path(), libname, &currentToken);
	return nullptr;
}

node_ptr_t NodeParser::parseClassNode(ns_vec_t &scopes)
{
	auto type = currentToken.type;
	nextToken();
	auto key = ROSSA_HASH(currentToken.valueString);
	nextToken();
	if (currentToken.type != TOK_CLASS)
		return logErrorN(format::format(_EXPECTED_ERROR_, { KEYWORD_CLASS }), currentToken);
	nextToken();
	node_ptr_t extends = nullptr;
	if (currentToken.type == ':') {
		nextToken();
		extends = parseUnitNode(scopes);
	}
	if (currentToken.type != '{')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "{" }), currentToken);
	nextToken();
	node_vec_t body;
	scopes.push_back({ key });
	while (currentToken.type != NULL_TOK) {
		if (currentToken.type == '}')
			break;

		if (auto e = parseExprNode(scopes))
			body.push_back(e);
		else
			return logErrorN(_FAILURE_PARSE_CODE_, currentToken);
	}
	if (currentToken.type != '}')
		return logErrorN(format::format(_EXPECTED_ERROR_, { "}" }), currentToken);
	nextToken();

	scopes.pop_back();
	return std::make_shared<ClassNode>(scopes, key, type, body, extends, currentToken);
}

node_ptr_t NodeParser::parseLoadNode(ns_vec_t &scopes)
{
	nextToken();
	if (currentToken.type != TOK_STR_LIT)
		return logErrorN(_EXPECTED_FILE_, currentToken);
	std::string filename = currentToken.valueString + ".ra";
	nextToken();

	auto path = dir::findFile(currentFile.parent_path(), filename, &currentToken);

	if (std::find(dir::loaded.begin(), dir::loaded.end(), path) != dir::loaded.end()) {
		if (currentToken.type != ';')
			return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
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
	auto n = np.parse(consts);

	if (currentToken.type != ';')
		return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
	nextToken();

	return n;
}

node_ptr_t NodeParser::parseExprNode(ns_vec_t &scopes)
{

	if (currentToken.type == NULL_TOK)
		return logErrorN(_UNEXPECTED_TERMINATION_, currentToken);

	switch (currentToken.type) {
		case ';':
			return logErrorN(_EXPECTED_EXPR_, currentToken);
		case TOK_LOAD:
			return parseLoadNode(scopes);
		case TOK_DEF:
			return parseDefineNode(scopes);
		case TOK_FOR:
			return parseForNode(scopes);
		case TOK_WHILE:
			return parseWhileNode(scopes);
		case TOK_IF:
			return parseIfElseNode(scopes);
		case TOK_EXTERN:
			return parseExternNode(scopes);
		case TOK_SWITCH:
			return parseSwitchNode(scopes);
		case TOK_TRY:
			return parseTryCatchNode(scopes);
		case TOK_STRUCT:
		case TOK_STATIC:
		case TOK_VIRTUAL:
			return parseClassNode(scopes);
		case TOK_CONST:
		{
			auto marker = currentToken;
			nextToken();

			if (currentToken.type != TOK_IDF)
				return logErrorN(_EXPECTED_IDF_, currentToken);
			hash_ull id = ROSSA_HASH(currentToken.valueString);
			nextToken();

			if (currentToken.valueString != "=")
				return logErrorN(format::format(_EXPECTED_ERROR_, { "=" }), currentToken);
			nextToken();

			try {
				scope_t newScope(static_cast<hash_ull>(0));
				trace_t trace;
				auto vn = parseEquNode(scopes)->fold(*consts)->genParser()->evaluate(&newScope, trace);
				//scopes.back().var_ids.push_back({ id, vn });
				std::vector<hash_ull> path;
				for (auto &k : scopes)
					path.push_back(k.id);
				path.push_back(id);
				consts->push_back({ path, vn });
				if (currentToken.type != ';')
					return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
				nextToken();
				return std::make_shared<ContainerNode>(scopes, vn, marker);
			} catch (const rossa_error &e) {
				return logErrorN(_CANNOT_MAKE_CONST_, marker);
			}
			return nullptr;
		}
		case TOK_VAR:
		{
			auto marker = currentToken;
			nextToken();
			int i = 0;
			std::vector<hash_ull> v;
			while (currentToken.type != NULL_TOK) {
				if (i++ > 0) {
					if (currentToken.type != ',')
						return logErrorN(format::format(_EXPECTED_ERROR_, { "," }), currentToken);
					nextToken();
				}
				if (currentToken.type != TOK_IDF)
					return logErrorN(_EXPECTED_IDF_, currentToken);
				v.push_back(ROSSA_HASH(currentToken.valueString));
				nextToken();
				if (currentToken.type == ';') {
					nextToken();
					return std::make_shared<VarNode>(scopes, v, marker);
				}
			}
			return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
		}
		case TOK_THROW:
		{
			auto marker = currentToken;
			nextToken();
			if (auto ret = parseEquNode(scopes)) {
				if (currentToken.type != ';')
					return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
				nextToken();
				return std::make_shared<ThrowNode>(scopes, ret, marker);
			}
			return nullptr;
		}
		case TOK_BREAK:
			nextToken();
			if (currentToken.type != ';')
				return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
			nextToken();
			return std::make_shared<BreakNode>(scopes, currentToken);
		case TOK_CONTINUE:
			nextToken();
			if (currentToken.type != ';')
				return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
			nextToken();
			return std::make_shared<ContinueNode>(scopes, currentToken);
		case TOK_RETURN:
			nextToken();
			if (auto ret = parseEquNode(scopes)) {
				if (currentToken.type != ';')
					return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
				nextToken();
				return std::make_shared<ReturnNode>(scopes, ret, currentToken);
			}
			return nullptr;
		case TOK_REFER:
			nextToken();
			if (auto ret = parseEquNode(scopes)) {
				if (currentToken.type != ';')
					return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
				nextToken();
				return std::make_shared<ReferNode>(scopes, ret, currentToken);
			}
			return nullptr;
		case TOK_DELETE:
			nextToken();
			if (auto ret = parseEquNode(scopes)) {
				if (currentToken.type != ';')
					return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
				nextToken();
				return std::make_shared<DeleteNode>(scopes, ret, currentToken);
			}
			return nullptr;
		default:
			if (auto ret = parseEquNode(scopes)) {
				if (currentToken.type != ';')
					return logErrorN(format::format(_EXPECTED_ERROR_, { ";" }), currentToken);
				nextToken();
				return ret;
			}
			return nullptr;
	}
}

node_ptr_t NodeParser::parse(std::vector<std::pair<std::vector<hash_ull>, sym_t>> *consts)
{
	this->consts = consts;
	nextToken();
	ns_vec_t init;
	return parseEntryNode(init);
}

i_ptr_t NodeParser::genParser(const node_ptr_t &n)
{
	return n->genParser();
}

node_ptr_t NodeParser::logErrorN(const std::string &s, const token_t &t)
{
	trace_t stack_trace;
	if (t.type == NULL_TOK)
		throw rossa_error(s, tokens.at(index - 1), stack_trace);
	else
		throw rossa_error(s, t, stack_trace);
	return nullptr;
}

std::pair<fsig_t, std::vector<std::pair<LexerTokenType, hash_ull>>> NodeParser::logErrorSN(const std::string &s, const token_t &t)
{
	logErrorN(s, t);
	return { fsig_t(), {{NULL_TOK, -1}} };
}

param_t NodeParser::logErrorPT(const std::string &s, const token_t &t)
{
	logErrorN(s, t);
	return param_t({ Value::type_t::NIL });
}