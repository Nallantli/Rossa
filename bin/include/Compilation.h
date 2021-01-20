#pragma once

#define _MSP_(a) "i_ptr_t(" + a + ")"

#define C_NONE(name) _MSP_(std::string("new " name "(t)"))
#define C_UNARY(name, a) _MSP_(format::format("new " name "({0}, t)", {a}))
#define C_BINARY(name, a, b) _MSP_(format::format("new " name "({0}, {1}, t)", {a, b}))
#define C_TRINARY(name, a, b, c) _MSP_(format::format("new " name "({0}, {1}, {2}, t)", {a, b, c}))
#define C_QUATERNARY(name, a, b, c, d) _MSP_(format::format("new " name "({0}, {1}, {2}, {3}, t)", {a, b, c, d}))
#define C_QUINARY(name, a, b, c, d, e) _MSP_(format::format("new " name "({0}, {1}, {2}, {3}, {4}, t)", {a, b, c, d, e}))