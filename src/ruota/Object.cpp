#include "Ruota.h"

Object::Object(Scope &parent, OBJECT_TYPE type, std::shared_ptr<Instruction> body, const std::string &key) : body(body), type(type), key(key)
{
	this->internal = std::make_shared<Scope>(parent, key);
}

std::shared_ptr<Scope> Object::getScope()
{
	return this->internal;
}

SYM Object::instantiate(std::vector<SYM> params) const
{
	if (type != STRUCT_O)
		throw std::runtime_error("Cannot instantiate a non-struct Object");

	auto o = std::make_shared<Object>(*internal->getParent(), STATIC_O, body, key);
	o->body->evaluate(*o->getScope());
	auto f = manager::getFunction(o->getScope()->getVariable("init"), params.size());
	auto d = manager::newValue(o);

	f->evaluate(params, d);
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

bool Object::hasValue(const std::string &key) const
{
	return internal->hasValue(key);
}