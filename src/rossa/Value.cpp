#include "../../bin/include/Rossa.h"

using namespace rossa;

Value::Value()
	: type{ NIL }
{}

Value::Value(const type_sll &valueType)
	: type{ TYPE_NAME }
	, valueType{ valueType }
{}

Value::Value(const bool &valueBool)
	: type{ BOOLEAN_D }
	, valueBool{ valueBool }
{}

Value::Value(const std::shared_ptr<void> &valuePointer)
	: type{ POINTER }
	, valuePointer{ valuePointer }
{}

Value::Value(const std::shared_ptr<Scope> &valueObject)
	: type{ OBJECT }
	, valueObject{ valueObject }
{}

Value::Value(const sig_t &ftype, const std::shared_ptr<const Function> &function)
	: type{ FUNCTION }
	, valueFunction{ {function->getArgSize(), {{ftype, function}}} }
{}

Value::Value(const std::shared_ptr<const Function> &function)
	: type{ FUNCTION }
	, valueVARGFunction{ function }
{}

Value::Value(const RNumber &valueNumber)
	: type{ NUMBER }
	, valueNumber{ valueNumber }
{}

Value::Value(const sym_vec_t &valueVector)
	: type{ ARRAY }
	, valueVector(valueVector)
{}

Value::Value(const sym_map_t &valueDictionary)
	: type{ DICTIONARY }
	, valueDictionary{ valueDictionary }
{}

Value::Value(const std::string &valueString)
	: type{ STRING }
	, valueString{ valueString }
{}

void Value::clearData()
{
	switch (type) {
		case FUNCTION:
			valueFunction.clear();
			valueVARGFunction = nullptr;
			break;
		case ARRAY:
			valueVector.clear();
			break;
		case DICTIONARY:
			valueDictionary.clear();
			break;
		case POINTER:
			valuePointer = nullptr;
			break;
		case OBJECT:
			valueObject = nullptr;
			break;
		default:
			return;
	}
}