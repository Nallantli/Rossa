#include "Ruota.h"

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
	string path = "";
	if (parent != NULL)
		name_trace = parent->name_trace;
	if (key != 0) {
		name_trace.push_back(key);

		size_t i = 0;
		for (auto &p : name_trace) {
			if (i++ > 0)
				path += ".";
			path += MAIN_HASH.deHash(p);
		}
	}

	hashed_key = MAIN_HASH.hashString(path);
}

const Symbol Scope::instantiate(const std::vector<Symbol> &params, const Token *token) const
{
	if (type != STRUCT_O)
		throw RuotaError(_FAILURE_INSTANTIATE_OBJECT_, *token);

	auto o = std::make_shared<Scope>(parent, INSTANCE_O, body, hashed_key, extensions);
	o->body->evaluate(o.get());
	auto d = Symbol(o);
	o->getVariable(Ruota::HASH_INIT, token).call(params, token);
	return d;
}

const hash_ull Scope::getHashedKey() const
{
	return hashed_key;
}

const Symbol &Scope::getVariable(const hash_ull &key, const Token *token)
{
	if (values.find(key) != values.end())
		return values[key];
	if (parent != NULL)
		return parent->getVariable(key, token);

	throw RuotaError((boost::format(_UNDECLARED_VARIABLE_ERROR_) % MAIN_HASH.deHash(key)).str(), *token);
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

const Symbol Scope::getThis(const Token *token)
{
	if (type != SCOPE_O)
		return Symbol(shared_from_this());
	if (parent != NULL)
		return parent->getThis(token);

	throw RuotaError((boost::format(_UNDECLARED_VARIABLE_ERROR_) % "this").str(), *token);
}

bool Scope::hasValue(const hash_ull &key) const
{
	return values.find(key) != values.end();
}

Scope::~Scope()
{
	if (hasValue(Ruota::HASH_DELETER)) {
		values[Ruota::HASH_DELETER].call({}, NULL);
	}
}