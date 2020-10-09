#include "Ruota.h"

Object::Object(Scope *parent, const ObjectType &type, const std::shared_ptr<Instruction> &body, const hashcode_t &key, const Object *ex) : type(type), body(body), key(key), internal(std::make_shared<Scope>(parent, key))
{
	if (ex != NULL)
	{
		extensions = ex->extensions;
		extensions.push_back(ex->getHashedKey());
	}
}

Object::Object(Scope *parent, const ObjectType &type, const std::shared_ptr<Instruction> &body, const hashcode_t &key, const std::vector<object_type_t> &extensions) : type(type), body(body), key(key), extensions(extensions), internal(std::make_shared<Scope>(parent, key)) {}

Scope *Object::getScope() const
{
	return this->internal.get();
}

const Symbol Object::instantiate(const std::vector<Symbol> &params, const Token *token) const
{
	if (type != STRUCT_O)
		throw RuotaError(_FAILURE_INSTANTIATE_OBJECT_, *token);

	auto o = std::make_shared<Object>(internal->getParent(), INSTANCE_O, body, key, extensions);
	o->body->evaluate(o->getScope());
	auto d = Symbol(o);
	o->getScope()->getVariable(Ruota::HASH_INIT, token).call(params, &d, token);
	return d;
}

const ObjectType Object::getType() const
{
	return this->type;
}

const hashcode_t Object::getHashedKey() const
{
	return this->internal->getHashedKey();
}

bool Object::hasValue(const hashcode_t &key) const
{
	return internal->hasValue(key);
}

bool Object::extendsObject(const hashcode_t &ex) const
{
	return std::find(extensions.begin(), extensions.end(), ex) != extensions.end();
}

const std::shared_ptr<Instruction> Object::getBody() const
{
	return body;
}

Object::~Object()
{
	if (hasValue(Ruota::HASH_DELETER))
	{
		internal->getVariable(Ruota::HASH_DELETER, NULL).call({}, NULL, NULL);
	}
}