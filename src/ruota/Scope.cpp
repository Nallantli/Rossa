#include "Ruota.h"

using namespace ruota;

Scope::Scope() : parent(NULL), type(SCOPE_O), hashed_key(0), name_trace({})
{}

Scope::Scope(Scope *parent, const hash_ull &key) : parent(parent), type(SCOPE_O)
{
	traceName(key);
}

Scope::Scope(Scope *parent, const ObjectType &type, const std::shared_ptr<Instruction> &body, const hash_ull &key, const Scope *ex, const std::vector<type_sll> &extensions) : parent(parent), type(type), body(body)
{
	if (ex != NULL) {
		this->extensions = ex->extensions;
		this->extensions.push_back(ex->getHashedKey());
	} else {
		this->extensions = extensions;
	}
	traceName(key);
}

Scope::Scope(Scope *parent, const ObjectType &type, const std::shared_ptr<Instruction> &body, const hash_ull &hashed_key, const std::vector<type_sll> &extensions) : parent(parent), type(type), body(body), hashed_key(hashed_key), extensions(extensions)
{}

void Scope::traceName(const hash_ull &key)
{
	if (parent != NULL)
		name_trace = parent->name_trace;
	if (key != 0) {
		std::string path = "";
		name_trace.push_back(key);
		size_t i = 0;
		for (auto &p : name_trace) {
			if (i++ > 0)
				path += ".";
			path += RUOTA_DEHASH(p);
		}
		hashed_key = RUOTA_HASH(path);
	} else {
		hashed_key = Ruota::HASH_BLANK;
	}
}

const Symbol Scope::instantiate(const std::vector<Symbol> &params, const Token *token, std::vector<Function> &stack_trace) const
{
	if (type != STRUCT_O)
		throw RTError(_FAILURE_INSTANTIATE_OBJECT_, *token, stack_trace);

	auto o = std::make_shared<Scope>(parent, INSTANCE_O, body, hashed_key, extensions);
	o->body->evaluate(o.get(), stack_trace);
	auto d = Symbol(o);
	o->getVariable(Ruota::HASH_INIT, token, stack_trace).call(params, token, stack_trace);
	return d;
}

const hash_ull Scope::getHashedKey() const
{
	return hashed_key;
}

const Symbol &Scope::getVariable(const hash_ull &key, const Token *token, std::vector<Function> &stack_trace) const
{
	if (values.find(key) != values.end())
		return values.at(key);
	if (parent != NULL)
		return parent->getVariable(key, token, stack_trace);

	throw RTError((boost::format(_UNDECLARED_VARIABLE_ERROR_) % RUOTA_DEHASH(key)).str(), *token, stack_trace);
}

const Symbol &Scope::createVariable(const hash_ull &key, const Token *token)
{
	values[key] = Symbol();
	return values[key];
}

const Symbol &Scope::createVariable(const hash_ull &key, const Symbol &d, const Token *token)
{
	if (values.find(key) != values.end() && values[key].getValueType() == FUNCTION)
		values[key].addFunctions(&d, token);
	else
		values[key] = d;

	return values[key];
}

void Scope::clear()
{
	values.clear();
}

Scope *Scope::getParent() const
{
	return this->parent;
}

const bool Scope::extendsObject(const type_sll &ex) const
{
	return std::find(extensions.begin(), extensions.end(), ex) != extensions.end();
}

const ObjectType Scope::getType() const
{
	return type;
}

const std::shared_ptr<Instruction> Scope::getBody() const
{
	return body;
}
const Symbol Scope::getThis(const Token *token, std::vector<Function> &stack_trace)
{
	if (type != SCOPE_O)
		return Symbol(shared_from_this());
	if (parent != NULL)
		return parent->getThis(token, stack_trace);

	throw RTError((boost::format(_UNDECLARED_VARIABLE_ERROR_) % KEYWORD_THIS).str(), *token, stack_trace);
}

const bool Scope::hasValue(const hash_ull &key) const
{
	return values.find(key) != values.end();
}

Scope::~Scope()
{
	if (hasValue(Ruota::HASH_DELETER)) {
		std::vector<Function> stack_trace;
		values[Ruota::HASH_DELETER].call({}, NULL, stack_trace);
	}
}