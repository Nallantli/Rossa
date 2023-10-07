#include "../main/rossa/rossa.h"
#include "../main/rossa/function/function.h"
#include "../main/rossa/parser/parser.h"
#include "../main/rossa/symbol/symbol.h"
#include "../main/mediator/mediator.h"

#include <thread>
#include <memory>

namespace lib_thread
{
    inline void threadWrapper(const ptr_function_t &f)
    {
        trace_t stack_trace;
        try
        {
            function_evaluate(f, {}, NULL, stack_trace);
        }
        catch (const rossa_error_t &e)
        {
            parser_t::printError(e);
        }
    }
};

ROSSA_EXT_SIG(_thread_init, args)
{
    auto f = COERCE_POINTER(args[0], function_t);
    auto t = std::make_shared<std::thread>(lib_thread::threadWrapper, f);
    return MAKE_POINTER(t);
}

ROSSA_EXT_SIG(_thread_join, args)
{
    auto t = COERCE_POINTER(args[0], std::thread);

    if (t->joinable())
        t->join();

    return mediator_t();
}

ROSSA_EXT_SIG(_thread_detach, args)
{
    auto t = COERCE_POINTER(args[0], std::thread);

    if (t->joinable())
        t->detach();

    return mediator_t();
}

EXPORT_FUNCTIONS(lib_thread)
{
    ADD_EXT(_thread_detach);
    ADD_EXT(_thread_init);
    ADD_EXT(_thread_join);
}