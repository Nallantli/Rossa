#ifndef RUOTA_H
#define RUOTA_H

#include "RuotaTypes.h"
#include "Lexer.h"

class Ruota;

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
class PowI;
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
class LengthI;
class SizeI;
class ClassI;
class NewI;
class TypeI;
class CastToI;
class AllocI;
class UntilI;

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
	SYM d;

public:
	Container(const SYM &d);
	SYM evaluate(Scope &) const override;
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
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class Sequence : public Instruction
{
protected:
	std::vector<Instruction *> children;
	bool scoped;

public:
	Sequence(bool, std::vector<Instruction *>);
	SYM evaluate(Scope &) const override;
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
	SYM evaluate(Scope &) const override;
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
	SYM evaluate(Scope &) const override;
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
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
	virtual ~ForI();
};

class VariableI : public CastingI
{
public:
	VariableI(const std::string &);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class DeclareI : public CastingI
{
public:
	DeclareI(const std::string &);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class IndexI : public BinaryI
{
public:
	IndexI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class InnerI : public BinaryI
{
public:
	InnerI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class CallI : public BinaryI
{
public:
	CallI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class AddI : public BinaryI
{
public:
	AddI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class SubI : public BinaryI
{
public:
	SubI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class MulI : public BinaryI
{
public:
	MulI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class DivI : public BinaryI
{
public:
	DivI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ModI : public BinaryI
{
public:
	ModI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class PowI : public BinaryI
{
public:
	PowI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class LessI : public BinaryI
{
public:
	LessI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class MoreI : public BinaryI
{
public:
	MoreI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ELessI : public BinaryI
{
public:
	ELessI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class EMoreI : public BinaryI
{
public:
	EMoreI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class Equals : public BinaryI
{
public:
	Equals(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class NEquals : public BinaryI
{
public:
	NEquals(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class AndI : public BinaryI
{
public:
	AndI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class OrI : public BinaryI
{
public:
	OrI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class SetI : public BinaryI
{
public:
	SetI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ReturnI : public UnaryI
{
public:
	ReturnI(Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ExternI : public UnaryI
{
protected:
	boost::function<SYM(std::vector<SYM>)> f;

public:
	ExternI(boost::function<SYM(std::vector<SYM>)>, Instruction *a);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class LengthI : public UnaryI
{
public:
	LengthI(Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class SizeI : public UnaryI
{
public:
	SizeI(Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class ClassI : public Instruction
{
protected:
	std::string key;
	OBJECT_TYPE type;
	std::shared_ptr<Instruction> body;

public:
	ClassI(const std::string &, OBJECT_TYPE, std::shared_ptr<Instruction>);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class NewI : public BinaryI
{
public:
	NewI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class TypeI : public UnaryI
{
public:
	TypeI(Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class CastToI : public UnaryI
{
protected:
	D_TYPE convert;

public:
	CastToI(Instruction *, D_TYPE);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class AllocI : public UnaryI
{
public:
	AllocI(Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class UntilI : public BinaryI
{
public:
	UntilI(Instruction *, Instruction *);
	SYM evaluate(Scope &) const override;
	const std::string toString() const override;
};

class Ruota
{
private:
	static const std::map<std::string, signed int> bOperators;
	static const std::map<std::string, signed int> uOperators;
	Scope main;

public:
	static Lexer lexer;
	//static std::unique_ptr<DataManager> manager;
	Ruota();
	SYM parseCode(const std::string &code);
};

namespace rdir
{
	static std::vector<boost::filesystem::path> loaded;

	static inline boost::filesystem::path findFile(boost::filesystem::path currentDir, const std::string &filename)
	{
		auto currentDirCheck = currentDir / filename;
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
	static std::map<std::string, boost::function<SYM(std::vector<SYM>)>> loaded;

	static inline void loadFunction(boost::filesystem::path currentDir, const std::string &rawlibname, const std::string &fname)
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

		loaded[search] = boost::dll::import<SYM(std::vector<SYM>)>(rdir::findFile(currentDir, libname), fname);
	}
} // namespace rlib

#endif