#include "Ruota.h"

Object::Object(Scope &parent, OBJECT_TYPE type, std::shared_ptr<Instruction> body, const std::string &key) : body(body), type(type), key(key)
{
	this->internal = std::make_shared<Scope>(parent, key);
}

std::shared_ptr<Scope> Object::getScope() const
{
	return this->internal;
}

const Symbol Object::instantiate(std::vector<Symbol> params, Token * token) const
{
	if (type != STRUCT_O)
		throwError("Cannot instantiate a non-struct Object", token);

	auto o = std::make_shared<Object>(*internal->getParent(), INSTANCE_O, body, key);
	o->body->evaluate(*o->getScope());
	auto f = o->getScope()->getVariable(Ruota::HASH_INIT, token).getFunction(NIL, params.size(), token);
	auto d = Symbol(o);

	f->evaluate(params, &d, token);
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

Object::~Object()
{
	if (hasValue(Ruota::HASH_DELETER))
	{
		auto f = internal->getVariable(Ruota::HASH_DELETER, NULL).getFunction(NIL, 0, NULL);
		f->evaluate({}, NULL);
	}
}