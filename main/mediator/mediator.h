#ifndef MEDIATOR_H
#define MEDATIOR_H

#include <variant>
#include <vector>
#include <memory>
#include <map>

#include "../number/number.h"

class mediator_t;

typedef const mediator_t (*extf_t)(const std::vector<mediator_t> &);
#define ROSSA_EXT_SIG(name, args) inline const mediator_t name(const std::vector<mediator_t> &args)
#define ADD_EXT(name) fmap[#name] = name
typedef void (*export_fns_t)(std::map<std::string, extf_t> &);

#ifndef _WIN32
#include <limits.h>
#include <unistd.h>
#include <dlfcn.h>
#define colorASCII(c) "\033[" + std::to_string(c) + "m"
#define EXPORT_FUNCTIONS(name) extern "C" void name##_rossaExportFunctions(std::map<std::string, extf_t> &fmap)
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define EXPORT_FUNCTIONS(name) extern "C" __declspec(dllexport) void name##_rossaExportFunctions(std::map<std::string, extf_t> &fmap)
#endif

#define COERCE_STRING(v) (*reinterpret_cast<std::string *>((v).getValue().get()))
#define COERCE_BOOLEAN(v) (*reinterpret_cast<bool *>((v).getValue().get()))
#define COERCE_NUMBER(v) (*reinterpret_cast<number_t *>((v).getValue().get()))
#define COERCE_ARRAY(v) (*reinterpret_cast<std::vector<mediator_t> *>((v).getValue().get()))
#define COERCE_DICTIONARY(v) (*reinterpret_cast<std::map<const std::string, const mediator_t> *>((v).getValue().get()))
#define COERCE_POINTER(v, t) (std::static_pointer_cast<t>((v).getValue()))

#define MAKE_NUMBER(v) (mediator_t(MEDIATOR_NUMBER, std::make_shared<number_t>(v)))
#define MAKE_STRING(v) (mediator_t(MEDIATOR_STRING, std::make_shared<std::string>(v)))
#define MAKE_BOOLEAN(v) (mediator_t(MEDIATOR_BOOLEAN_D, std::make_shared<bool>(v)))
#define MAKE_DICTIONARY(v) (mediator_t(MEDIATOR_DICTIONARY, std::make_shared<std::map<const std::string, const mediator_t>>(v)))
#define MAKE_POINTER(v) (mediator_t(MEDIATOR_POINTER, v))

enum mediator_type_enum
{
    MEDIATOR_NIL = -1,
    MEDIATOR_NUMBER = -2,
    MEDIATOR_BOOLEAN_D = -3,
    MEDIATOR_STRING = -4,
    MEDIATOR_ARRAY = -5,
    MEDIATOR_FUNCTION = -6,
    MEDIATOR_DICTIONARY = -7,
    MEDIATOR_OBJECT = -8,
    MEDIATOR_TYPE_NAME = -9,
    MEDIATOR_POINTER = -10
};

class mediator_t
{
private:
    const mediator_type_enum type;
    std::shared_ptr<void> value;

public:
    mediator_t();
    mediator_t(const mediator_type_enum &, std::shared_ptr<void>);
    const mediator_type_enum getType() const;
    std::shared_ptr<void> getValue() const;
};

class library_error_t : public std::runtime_error
{
public:
    library_error_t(const std::string &);
};

#endif