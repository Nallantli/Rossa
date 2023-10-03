#include "mediator.h"

mediator_t::mediator_t()
    : type(MEDIATOR_NIL), value(nullptr)
{
}

mediator_t::mediator_t(const mediator_type_enum &type, std::shared_ptr<void> value)
    : type(type), value(value)
{
}

const mediator_type_enum mediator_t::getType() const
{
    return this->type;
}

std::shared_ptr<void> mediator_t::getValue() const
{
    return this->value;
}

library_error_t::library_error_t(const std::string &error)
    : std::runtime_error(error)
{
}