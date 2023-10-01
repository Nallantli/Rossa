#ifndef SYMBOL_H
#define SYMBOL_H

#include "../rossa.h"

struct symbol_t
{
private:
	value_t *d;

	symbol_t(const std::shared_ptr<void> &);
	symbol_t(const parameter_t &);
	symbol_t(const number_t &);
	symbol_t(const bool &);
	symbol_t(const std::vector<symbol_t> &);
	symbol_t(const object_t &);
	symbol_t(const signature_t &, const ptr_function_t &);
	symbol_t(const ptr_function_t &);
	symbol_t(const std::string &);
	symbol_t(const std::map<const std::string, const symbol_t> &);

public:
	enum type_t
	{
		ID_RETURN,
		ID_BREAK,
		ID_CONTINUE,
		ID_CASUAL,
		ID_REFER
	} type;

	symbol_t(const type_t &);
	symbol_t(const symbol_t &);
	symbol_t();

	const unsigned int hash() const;

	static const symbol_t Pointer(const std::shared_ptr<void> &);
	static const symbol_t TypeName(const parameter_t &);
	static const symbol_t Number(const number_t &);
	static const symbol_t Boolean(const bool &);
	static const symbol_t Array(const std::vector<symbol_t> &);
	static const symbol_t Object(const object_t &);
	static const symbol_t FunctionSIG(const signature_t &, const ptr_function_t &);
	static const symbol_t FunctionVARG(const ptr_function_t &);
	static const symbol_t String(const std::string &);
	static const symbol_t Dictionary(const std::map<const std::string, const symbol_t> &);

	~symbol_t();

	void operator=(const symbol_t &);
	static const symbol_t allocate(const size_t &);
	static const symbol_t allocateAs(const size_t &, const symbol_t *, const token_t *, trace_t &);
	const type_t getSymbolType() const;
	void setSymbolType(const type_t &);
	const number_t &getNumber(const token_t *, trace_t &) const;
	void *getPointer(const token_t *, trace_t &) const;
	std::map<const std::string, const symbol_t> &getDictionary(const token_t *, trace_t &) const;
	const symbol_t &indexVector(const size_t &, const token_t *, trace_t &) const;
	const std::vector<symbol_t> &getVector(const token_t *, trace_t &) const;
	const std::string getString(const token_t *, trace_t &) const;
	const bool getBool(const token_t *, trace_t &) const;
	const bool hasVarg(const token_t *, trace_t &) const;
	object_t *getObject(const token_t *, trace_t &) const;
	const value_type_enum getValueType() const;
	const parameter_t getAugValueType() const;
	const parameter_t getTypeName(const token_t *, trace_t &) const;
	const ptr_function_t getFunction(const std::vector<symbol_t> &, const token_t *, trace_t &) const;
	const ptr_function_t &getVARGFunction(const token_t *, trace_t &) const;
	const symbol_t &indexDict(const std::string &) const;
	const bool hasDictionaryKey(const std::string &) const;
	const size_t vectorSize() const;
	const size_t dictionarySize(const token_t *, trace_t &) const;
	const std::string toString(const token_t *, trace_t &) const;
	const std::string toCodeString() const;
	const symbol_t call(const std::vector<symbol_t> &, const token_t *, trace_t &) const;
	void addFunctions(const symbol_t *, const token_t *) const;
	void nullify() const;
	void set(const symbol_t *, const token_t *, trace_t &) const;
	const bool equals(const symbol_t *, const token_t *, trace_t &) const;
	const bool nequals(const symbol_t *, const token_t *, trace_t &) const;
	const bool pureEquals(const symbol_t *, const token_t *, trace_t &) const;
	const bool pureNEquals(const symbol_t *, const token_t *, trace_t &) const;
	const bool operator==(const symbol_t &) const;
	const bool operator!=(const symbol_t &) const;
	const bool operator<(const symbol_t &) const;
	const std::map<const size_t, std::map<const signature_t, ptr_function_t>> &getFunctionOverloads(const token_t *, trace_t &) const;
	void shift() const;

	const symbol_t clone() const;
};

#endif