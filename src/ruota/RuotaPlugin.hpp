#ifndef RUOTAPLUGIN_H
#define RUOTAPLUGIN_H

#include <vector>
#include <string>

enum P_D_TYPE
{
	P_NIL,
	P_NUMBER,
	P_STRING,
	P_VECTOR,
	P_BOOLEAN
};

struct pDatum
{
private:
	P_D_TYPE type;
	std::string valueString;
	double valueNumber;
	bool valueBool;
	std::vector<pDatum> valueVector;

public:
	pDatum() : type(P_NIL) {}
	pDatum(const std::string &valueString) : valueString(valueString), type(P_STRING) {}
	pDatum(const double &valueNumber) : valueNumber(valueNumber), type(P_NUMBER) {}
	pDatum(const bool &valueBool) : valueBool(valueBool), type(P_BOOLEAN) {}
	pDatum(std::vector<pDatum> valueVector) : valueVector(valueVector), type(P_VECTOR) {}

	std::string getString() const { return valueString; }
	double getNumber() const { return valueNumber; }
	bool getBool() const { return valueBool; }
	std::vector<pDatum> getVector() const { return valueVector; }
	P_D_TYPE getType() const { return type; }
};

#endif