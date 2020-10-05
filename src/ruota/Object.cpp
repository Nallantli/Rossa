#include "Ruota.h"

Object::Object(Scope *parent, ObjectType type, std::shared_ptr<Instruction> body, const std::string &key) : body(body), type(type), key(key)
{
	this->internal = std::make_shared<Scope>(parent, key);
}

Scope *Object::getScope() const
{
	return this->internal.get();
}

const Symbol Object::instantiate(const std::vector<Symbol> &params, const Token *token) const
{
	if (type != STRUCT_O)
		throw RuotaError(_FAILURE_INSTANTIATE_OBJECT_, *token);

	auto o = std::make_shared<Object>(internal->getParent(), INSTANCE_O, body, key);
	o->body->evaluate(o->getScope());
	auto d = Symbol(o);
	o->getScope()->getVariable(Ruota::HASH_INIT, token).call(params, &d, token);
	return d;
}

const ObjectType Object::getType() const
{
	return this->type;
}

const std::string &Object::getName() const
{
	return this->internal->getName();
}

bool Object::hasValue(hashcode_t key) const
{
	return internal->hasValue(key);
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