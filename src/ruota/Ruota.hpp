#ifndef RUOTA_H
#define RUOTA_H

#include "RuotaPlugin.hpp"
#include "Lexer.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <stdexcept>
#include <memory>
#include <map>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

enum D_TYPE
{
	NIL,
	NUMBER,
	BOOLEAN_D,
	STRING,
	VECTOR,
	FUNCTION,
	DICTIONARY,
	OBJECT
};

enum I_TYPE
{
	CONTAINER,
	VARIABLE,
	SEQUENCE,
	DECLARE,
	INDEX,
	INNER,
	IFELSE,
	WHILE,
	DEFINE,
	RETURN,
	EXTERN,
	FOR,
	SET,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	LESS,
	MORE,
	ELESS,
	EMORE,
	EQUALS,
	NEQUALS,
	AND,
	OR
};

enum DID_TYPE
{
	ID_RETURN,
	ID_BREAK,
	ID_CASUAL
};

class Ruota;

class DataManager;
class Datum;
class Function;
class Scope;
class Instruction;
class UnaryI;
class CastingI;
class BinaryI;
class Container;
class DefineI;
class Sequence;
class IfElseI;
class WhileI;
class ForI;
class VariableI;
class DeclareI;
class IndexI;
class InnerI;
class AddI;
class SubI;
class MulI;
class DivI;
class ModI;
class LessI;
class MoreI;
class ELessI;
class EMoreI;
class Equals;
class NEquals;
class AndI;
class OrI;
class SetI;
class ReturnI;
class ExternI;

struct DatumID
{
	unsigned long id;
	bool constant;
	DID_TYPE type;
	std::shared_ptr<Datum> d;

	DatumID() : id(-1), constant(true), type(ID_CASUAL), d(std::make_shared<Datum>()) {}
	DatumID(unsigned long id, bool constant) : id(id), type(ID_CASUAL), constant(constant) {}
	DatumID(std::shared_ptr<Datum> d) : d(std::move(d)), constant(true), type(ID_CASUAL), id(-1) {}
};

class Datum
{
private:
	D_TYPE type;
	double valueNumber;
	bool valueBool;
	std::vector<DatumID> valueVector;
	std::string valueString;
	std::map<unsigned long, std::shared_ptr<Function>> valueFunction;

public:
	Datum();
	Datum(bool);
	Datum(std::shared_ptr<Function>);
	Datum(double);
	Datum(const std::vector<DatumID> &);
	Datum(const std::string &);
	const double getNumber() const;
	const std::string &getString() const;
	const bool getBool() const;
	std::shared_ptr<Function> getFunction(unsigned long) const;
	std::map<unsigned long, std::shared_ptr<Function>> getFunctions() const;
	void setFunctions(std::map<unsigned long, std::shared_ptr<Function>>);
	const std::vector<DatumID> &getVector() const;
	const unsigned long vectorSize() const;
	const DatumID &indexVector(unsigned long) const;
	const D_TYPE getType() const;
	void setNumber(double);
	void setString(const std::string &);
	void addFunction(std::shared_ptr<Function>);
	void setVector(const std::vector<DatumID> &);
	void setBool(bool);
	void setType(D_TYPE);
	void clearData();

	~Datum();
};

class DataManager
{
private:
	std::deque<unsigned long> openSpots;
	std::vector<std::shared_ptr<Datum>> tape;
	std::shared_ptr<Datum> getDatum(unsigned long);
	unsigned long newDatum(std::shared_ptr<Datum>);
	void set(std::shared_ptr<Datum>, std::shared_ptr<Datum>);

public:
	DataManager();
	void setFree(const DatumID &);
	DatumID newDatum(bool);
	DatumID newDatum(bool, double);
	DatumID newDatum(bool, bool);
	DatumID newDatum(bool, const std::vector<DatumID> &);
	DatumID newDatum(bool, std::shared_ptr<Function>);
	DatumID newDatum(bool, const std::string &);
	const double getNumber(const DatumID &) const;
	const std::string &getString(const DatumID &) const;
	const std::vector<DatumID> &getVector(const DatumID &) const;
	const bool getBool(const DatumID &) const;
	const D_TYPE getType(const DatumID &) const;
	DatumID call(const DatumID &, const std::vector<DatumID> &);
	void addFunctions(const DatumID &, const DatumID &);
	void setNumber(const DatumID &, double);
	void setString(const DatumID &, const std::string &);
	void setVector(const DatumID &, const std::vector<DatumID> &);
	const DatumID &indexVector(const DatumID &, unsigned long) const;
	const unsigned long vectorSize(const DatumID &) const;
	void set(DatumID, const DatumID &);
	const std::string toString(const DatumID &);
	const std::string toString(std::shared_ptr<Datum>);

	DatumID add(const DatumID &, const DatumID &);
	DatumID sub(const DatumID &, const DatumID &);
	DatumID mul(const DatumID &, const DatumID &);
	DatumID div(const DatumID &, const DatumID &);
	DatumID mod(const DatumID &, const DatumID &);
	DatumID less(const DatumID &, const DatumID &);
	DatumID more(const DatumID &, const DatumID &);
	DatumID eless(const DatumID &, const DatumID &);
	DatumID emore(const DatumID &, const DatumID &);
	bool equals(const DatumID &, const DatumID &);
	bool nequals(const DatumID &, const DatumID &);
	bool dand(const DatumID &, const DatumID &);
	bool dor(const DatumID &, const DatumID &);
};

class Scope
{
private:
	Scope *parent;
	std::map<std::string, DatumID> values;

public:
	Scope();
	Scope(Scope &);
	DatumID getVariable(const std::string &);
	DatumID createVariable(const std::string &);
	DatumID createVariable(const std::string &, const DatumID &);

	~Scope();
};

class Function
{
private:
	std::vector<std::string> paramNames;
	std::shared_ptr<Instruction> body;
	Scope *parent;

public:
	Function(Scope &, std::vector<std::string>, std::shared_ptr<Instruction>);
	DatumID evaluate(std::vector<DatumID>);
	const unsigned long getArgSize() const;
};

class Instruction
{
protected:
	I_TYPE type;

public:
	Instruction(I_TYPE);
	virtual DatumID evaluate(Scope &) const = 0;
	virtual const std::string toString() const = 0;
	const I_TYPE getType();
	virtual ~Instruction();
};

class UnaryI : public Instruction
{
protected:
	Instruction *a;

public:
	UnaryI(I_TYPE, Instruction *);
	Instruction *getA();
	virtual ~UnaryI();
};

class CastingI : public Instruction
{
protected:
	std::string key;

public:
	CastingI(I_TYPE, const std::string &);
	const std::string getKey();
};

class BinaryI : public UnaryI
{
protected:
	Instruction *b;

public:
	BinaryI(I_TYPE, Instruction *, Instruction *);
	Instruction *getB();
	virtual ~BinaryI();
};

class Container : public Instruction
{
protected:
	DatumID d;

public:
	Container(const DatumID &d);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class DefineI : public Instruction
{
protected:
	std::string key;
	std::vector<std::string> fargs;
	std::shared_ptr<Instruction> body;

public:
	DefineI(const std::string &, std::vector<std::string>, std::shared_ptr<Instruction>);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class Sequence : public Instruction
{
protected:
	std::vector<Instruction *> children;
	bool scoped;

public:
	Sequence(bool, std::vector<Instruction *>);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
	void setScoped(bool);
	virtual ~Sequence();
};

class IfElseI : public Instruction
{
protected:
	Instruction *ifs;
	Instruction *body;
	Instruction *elses;

public:
	IfElseI(Instruction *, Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
	virtual ~IfElseI();
};

class WhileI : public Instruction
{
protected:
	Instruction *whiles;
	Instruction *body;

public:
	WhileI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
	virtual ~WhileI();
};

class ForI : public Instruction
{
protected:
	std::string id;
	Instruction *fors;
	Instruction *body;

public:
	ForI(const std::string &, Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
	virtual ~ForI();
};

class VariableI : public CastingI
{
public:
	VariableI(const std::string &);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class DeclareI : public CastingI
{
public:
	DeclareI(const std::string &);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class CallI : public BinaryI
{
public:
	CallI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class AddI : public BinaryI
{
public:
	AddI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class SubI : public BinaryI
{
public:
	SubI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class MulI : public BinaryI
{
public:
	MulI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class DivI : public BinaryI
{
public:
	DivI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ModI : public BinaryI
{
public:
	ModI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class LessI : public BinaryI
{
public:
	LessI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class Equals : public BinaryI
{
public:
	Equals(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class NEquals : public BinaryI
{
public:
	NEquals(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class AndI : public BinaryI
{
public:
	AndI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class OrI : public BinaryI
{
public:
	OrI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class SetI : public BinaryI
{
public:
	SetI(Instruction *, Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(Instruction *);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ExternI : public UnaryI
{
protected:
	boost::function<pDatum(std::vector<pDatum>)> f;

public:
	ExternI(boost::function<pDatum(std::vector<pDatum>)>, Instruction *a);
	DatumID evaluate(Scope &) const override;
	const std::string toString() const override;
};

class Ruota
{
private:
	static const std::map<std::string, signed int> bOperators;
	static const std::map<std::string, signed int> uOperators;
	static Lexer lexer;
	Scope main;

public:
	static std::unique_ptr<DataManager> manager;
	Ruota();
	DatumID parseCode(const std::string &code);
};

namespace rdir
{
	static inline boost::filesystem::path findFile(const std::string &filename)
	{
		auto currentDirCheck = boost::filesystem::current_path() / filename;
		if (boost::filesystem::exists(currentDirCheck))
			return currentDirCheck;
		auto libDirCheck = boost::dll::program_location().parent_path() / "lib" / filename;
		if (boost::filesystem::exists(libDirCheck))
			return libDirCheck;
		throw std::runtime_error("File `" + filename + "` cannot be found");
	}
} // namespace rdir

namespace rlib
{
	static std::map<std::string, boost::function<pDatum(std::vector<pDatum>)>> loaded;

	static inline DatumID convertFromPDatum(const pDatum &pd)
	{
		switch (pd.getType())
		{
		case P_NIL:
			return DatumID();
		case P_STRING:
			return Ruota::manager->newDatum(true, pd.getString());
		case P_NUMBER:
			return Ruota::manager->newDatum(true, pd.getNumber());
		case P_BOOLEAN:
			return Ruota::manager->newDatum(true, pd.getBool());
		case P_VECTOR:
		{
			auto v = pd.getVector();
			std::vector<DatumID> nv;
			for (auto &e : v)
				nv.push_back(convertFromPDatum(e));
			return Ruota::manager->newDatum(true, nv);
		}
		default:
			throw std::runtime_error("Unknown external_call function return type: " + std::to_string(pd.getType()));
		}
	}

	static inline pDatum convertToPDatum(const DatumID &d)
	{
		switch (Ruota::manager->getType(d))
		{
		case NIL:
			return pDatum();
		case NUMBER:
			return pDatum(Ruota::manager->getNumber(d));
		case STRING:
			return pDatum(Ruota::manager->getString(d));
		case BOOLEAN_D:
			return pDatum(Ruota::manager->getBool(d));
		case VECTOR:
		{
			auto v = Ruota::manager->getVector(d);
			std::vector<pDatum> nv;
			for (auto &e : v)
			{
				nv.push_back(convertToPDatum(e));
			}
			return pDatum(nv);
		}
		}
	}

	static inline void loadFunction(const std::string &rawlibname, const std::string &fname)
	{
		std::string libname = rawlibname;

#ifdef __unix__
		libname += ".so";
#else
		libname += ".dll";
#endif

		std::string search = rawlibname + "$" + fname;
		if (loaded.find(search) != loaded.end())
			return;

		loaded[search] = boost::dll::import<pDatum(std::vector<pDatum>)>(rdir::findFile(libname), fname);
	}
} // namespace rlib

#endif