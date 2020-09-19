#include "Ruota.hpp"

Datum::Datum() : type(NIL) {}

Datum::Datum(double valueNumber) : valueNumber(valueNumber), type(NUMBER) {}

Datum::Datum(bool valueBool) : valueBool(valueBool), type(BOOLEAN_D) {}

Datum::Datum(std::shared_ptr<Function> f) : type(FUNCTION)
{
	addFunction(f);
}

Datum::Datum(const std::string &valueString) : valueString(valueString), type(STRING) {}

Datum::Datum(const std::vector<DatumID> &valueVector) : valueVector(valueVector), type(VECTOR) {}

Datum::~Datum() {}

std::map<unsigned long, std::shared_ptr<Function>> Datum::getFunctions() const
{
	return this->valueFunction;
}

void Datum::setFunctions(std::map<unsigned long, std::shared_ptr<Function>> valueFunction)
{
	this->valueFunction = valueFunction;
}

const double Datum::getNumber() const
{
	if (type != NUMBER)
		throw std::runtime_error("Value is not of type Number");
	return valueNumber;
}
const bool Datum::getBool() const
{
	if (type != BOOLEAN_D)
		throw std::runtime_error("Value is not of type Bool");
	return valueBool;
}

const std::string &Datum::getString() const
{
	if (type != STRING)
		throw std::runtime_error("Value is not of type String");
	return valueString;
}

std::shared_ptr<Function> Datum::getFunction(unsigned long argSize) const
{
	if (type != FUNCTION)
		throw std::runtime_error("Value is not of type Function");
	if (valueFunction.find(argSize) == valueFunction.end())
		throw std::runtime_error("Function does not exist");
	return valueFunction.at(argSize);
}
void Datum::setBool(bool valueBool)
{
	this->valueBool = valueBool;
}
void Datum::setNumber(double valueNumber)
{
	this->valueNumber = valueNumber;
}
void Datum::setString(const std::string &valueString)
{
	this->valueString = valueString;
}
void Datum::addFunction(std::shared_ptr<Function> f)
{
	this->valueFunction[f->getArgSize()] = f;
}
const std::vector<DatumID> &Datum::getVector() const
{
	if (type != VECTOR)
		throw std::runtime_error("Value is not of type Vector");
	return valueVector;
}
void Datum::setVector(const std::vector<DatumID> &valueVector)
{
	this->valueVector.clear();
	for (int i = 0; i < valueVector.size(); i++)
	{
		auto d = Ruota::manager->newDatum(false);
		Ruota::manager->set(d, valueVector[i]);
		this->valueVector.push_back(d);
	}
}
const DatumID &Datum::indexVector(unsigned long index) const
{
	if (type != VECTOR)
		throw std::runtime_error("Value is not of type Vector");
	return this->valueVector[index];
}

const unsigned long Datum::vectorSize() const
{
	if (type != VECTOR)
		throw std::runtime_error("Value is not of type Vector");
	return this->valueVector.size();
}

void Datum::clearData()
{
	this->valueFunction.clear();
	this->valueVector.clear();
}

const D_TYPE Datum::getType() const { return type; }
void Datum::setType(D_TYPE type) { this->type = type; }