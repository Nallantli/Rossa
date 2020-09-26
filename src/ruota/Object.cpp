#include "Ruota.h"

Object::Object(Scope &parent, OBJECT_TYPE type, std::shared_ptr<Instruction> body, const std::string &key) : body(body), type(type), key(key)
{
	this->internal = std::make_shared<Scope>(parent, key);
}

std::shared_ptr<Scope> Object::getScope() const
{
	return this->internal;
}

Symbol Object::instantiate(std::vector<Symbol> params) const
{
	if (type != STRUCT_O)
		throw std::runtime_error("Cannot instantiate a non-struct Object");

	auto o = std::make_shared<Object>(*internal->getParent(), STATIC_O, body, key);
	o->body->evaluate(*o->getScope());
	auto f = o->getScope()->getVariable(Ruota::HASH_INIT).getFunction(NIL, params.size());
	auto d = Symbol(o);

	f->evaluate(params, &d);
	return d;
}

OBJECT_TYPE Object::getType() const
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

std::shared_ptr<Instruction> Object::getBody() const
{
	return body;
}