#include "Ruota.hpp"

std::shared_ptr<Datum> DataManager::getDatum(unsigned long index)
{
	return tape[index];
}

unsigned long DataManager::newDatum(std::shared_ptr<Datum> d)
{
	unsigned long pos;
	if (openSpots.empty())
	{
		tape.push_back(d);
		pos = tape.size() - 1;
	}
	else
	{
		pos = openSpots.front();
		openSpots.pop_front();
		tape[pos] = d;
	}
	return pos;
}

void DataManager::set(std::shared_ptr<Datum> dA, std::shared_ptr<Datum> dB)
{
	dA->setType(dB->getType());
	dA->clearData();
	switch (dA->getType())
	{
	case NUMBER:
		dA->setNumber(dB->getNumber());
		break;
	case BOOLEAN_D:
		dA->setBool(dB->getBool());
		break;
	case STRING:
		dA->setString(dB->getString());
		break;
	case FUNCTION:
		dA->setFunctions(dB->getFunctions());
		break;
	case VECTOR:
		dA->setVector(dB->getVector());
		break;
	default:
		break;
	}
}

DataManager::DataManager() {}

void DataManager::setFree(const DatumID &id)
{
	if (tape[id.id] != NULL)
	{
		//delete tape[id->id];
		tape[id.id] = NULL;
		openSpots.push_back(id.id);
	}
}
DatumID DataManager::newDatum(bool constant)
{
	auto ptr = std::make_shared<Datum>();
	if (constant)
		return DatumID(ptr);
	return DatumID(newDatum(ptr), false);
}
DatumID DataManager::newDatum(bool constant, double d)
{
	auto ptr = std::make_shared<Datum>(d);
	if (constant)
		return DatumID(ptr);
	return DatumID(newDatum(ptr), false);
}
DatumID DataManager::newDatum(bool constant, bool b)
{
	auto ptr = std::make_shared<Datum>(b);
	if (constant)
		return DatumID(ptr);
	return DatumID(newDatum(ptr), false);
}
DatumID DataManager::newDatum(bool constant, const std::vector<DatumID> &v)
{
	auto ptr = std::make_shared<Datum>(v);
	if (constant)
		return DatumID(ptr);
	return DatumID(newDatum(ptr), false);
}
DatumID DataManager::newDatum(bool constant, std::shared_ptr<Function> valueFunction)
{
	auto ptr = std::make_shared<Datum>(valueFunction);
	if (constant)
		return DatumID(ptr);
	return DatumID(newDatum(ptr), false);
}
DatumID DataManager::newDatum(bool constant, const std::string &valueString)
{
	auto ptr = std::make_shared<Datum>(valueString);
	if (constant)
		return DatumID(ptr);
	return DatumID(newDatum(ptr), false);
}

const double DataManager::getNumber(const DatumID &a) const
{
	if (a.constant)
		return a.d->getNumber();
	return tape[a.id]->getNumber();
}

const std::vector<DatumID> &DataManager::getVector(const DatumID &a) const
{
	if (a.constant)
		return a.d->getVector();
	return tape[a.id]->getVector();
}

const std::string &DataManager::getString(const DatumID &a) const
{
	if (a.constant)
		return a.d->getString();
	return tape[a.id]->getString();
}

const bool DataManager::getBool(const DatumID &a) const
{
	if (a.constant)
		return a.d->getBool();
	return tape[a.id]->getBool();
}
const D_TYPE DataManager::getType(const DatumID &a) const
{
	if (a.constant)
		return a.d->getType();
	return tape[a.id]->getType();
}
void DataManager::setNumber(const DatumID &a, double d)
{
	if (a.constant)
		return a.d->setNumber(d);
	tape[a.id]->setNumber(d);
}
void DataManager::setVector(const DatumID &a, const std::vector<DatumID> &valueVector)
{
	if (a.constant)
		return a.d->setVector(valueVector);
	tape[a.id]->setVector(valueVector);
}
void DataManager::setString(const DatumID &a, const std::string &valueString)
{
	if (a.constant)
		return a.d->setString(valueString);
	tape[a.id]->setString(valueString);
}
const DatumID &DataManager::indexVector(const DatumID &a, unsigned long index) const
{
	if (a.constant)
		return a.d->indexVector(index);
	return tape[a.id]->indexVector(index);
}
const unsigned long DataManager::vectorSize(const DatumID &a) const
{
	if (a.constant)
		return a.d->vectorSize();
	return tape[a.id]->vectorSize();
}

void DataManager::set(DatumID a, const DatumID &b)
{
	std::shared_ptr<Datum> dB;
	if (b.constant)
		dB = b.d;
	else
		dB = getDatum(b.id);

	std::shared_ptr<Datum> dA;
	if (a.constant)
		dA = a.d;
	else
		dA = getDatum(a.id);

	set(dA, dB);
}

const std::string DataManager::toString(const DatumID &d)
{
	if (d.constant)
		return toString(d.d);
	auto data = getDatum(d.id);
	if (data == NULL)
		return "<deleted>";
	return toString(data);
}

const std::string DataManager::toString(std::shared_ptr<Datum> d)
{
	switch (d->getType())
	{
	case NIL:
		return "nil";
	case NUMBER:
		return std::to_string(d->getNumber());
	case STRING:
		return d->getString();
	case FUNCTION:
		return "LAMBDA";
	case BOOLEAN_D:
		return d->getBool() ? "true" : "false";
	case VECTOR:
	{
		std::string ret = "[";
		unsigned int i = 0;
		for (auto &d2 : d->getVector())
		{
			if (i > 0)
				ret += ", ";
			ret += toString(d2);
			i++;
		}
		return ret + "]";
	}
	default:
		return "undefined";
	}
}

DatumID DataManager::call(const DatumID &a, const std::vector<DatumID> &params)
{
	auto f = a.constant ? a.d->getFunction(params.size()) : getDatum(a.id)->getFunction(params.size());

	return f->evaluate(params);
}

void DataManager::addFunctions(const DatumID &a, const DatumID &b)
{
	auto fs = b.constant ? b.d->getFunctions() : getDatum(b.id)->getFunctions();
	std::shared_ptr<Datum> dA = a.constant ? a.d : getDatum(a.id);
	for (auto &f : fs)
		dA->addFunction(f.second);
}

DatumID DataManager::add(const DatumID &a, const DatumID &b)
{
	if (getType(a) != getType(b))
		throw std::runtime_error("Cannot add values of different type");
	switch (getType(a))
	{
	case NUMBER:
	{
		auto valA = getNumber(a);
		auto valB = getNumber(b);
		return newDatum(true, valA + valB);
	}
	case VECTOR:
	{
		auto valA = getVector(a);
		auto valB = getVector(b);
		valA.insert(valA.end(), std::make_move_iterator(valB.begin()), std::make_move_iterator(valB.end()));
		return newDatum(true, valA);
	}
	default:
		throw std::runtime_error("Operator `+` is undefined for value type");
	}
}

DatumID DataManager::sub(const DatumID &a, const DatumID &b)
{
	auto valA = getNumber(a);
	auto valB = getNumber(b);
	return newDatum(true, valA - valB);
}

DatumID DataManager::mul(const DatumID &a, const DatumID &b)
{
	auto valA = getNumber(a);
	auto valB = getNumber(b);
	return newDatum(true, valA * valB);
}

DatumID DataManager::div(const DatumID &a, const DatumID &b)
{
	auto valA = getNumber(a);
	auto valB = getNumber(b);
	return newDatum(true, valA / valB);
}

DatumID DataManager::mod(const DatumID &a, const DatumID &b)
{
	int valA = getNumber(a);
	int valB = getNumber(b);
	return newDatum(true, (double)(valA % valB));
}

DatumID DataManager::less(const DatumID &a, const DatumID &b)
{
	int valA = getNumber(a);
	int valB = getNumber(b);
	return newDatum(true, valA < valB);
}

DatumID DataManager::more(const DatumID &a, const DatumID &b)
{
	int valA = getNumber(a);
	int valB = getNumber(b);
	return newDatum(true, valA > valB);
}

DatumID DataManager::eless(const DatumID &a, const DatumID &b)
{
	int valA = getNumber(a);
	int valB = getNumber(b);
	return newDatum(true, valA <= valB);
}

DatumID DataManager::emore(const DatumID &a, const DatumID &b)
{
	int valA = getNumber(a);
	int valB = getNumber(b);
	return newDatum(true, valA >= valB);
}

bool DataManager::equals(const DatumID &a, const DatumID &b)
{
	if (getType(a) != getType(b))
		return false;
	switch (getType(a))
	{
	case NIL:
		return true;
	case NUMBER:
		return getNumber(a) == getNumber(b);
	case BOOLEAN_D:
		return getBool(a) == getBool(b);
	case VECTOR:
		if (vectorSize(a) != vectorSize(b))
			return false;
		for (int i = 0; i < vectorSize(a); i++)
		{
			if (!equals(indexVector(a, i), indexVector(b, i)))
				return false;
		}
	default:
		return false;
	}
}

bool DataManager::nequals(const DatumID &a, const DatumID &b)
{
	return !equals(a, b);
}

bool DataManager::dand(const DatumID &a, const DatumID &b)
{
	return getBool(a) && getBool(b);
}

bool DataManager::dor(const DatumID &a, const DatumID &b)
{
	return getBool(a) || getBool(b);
}