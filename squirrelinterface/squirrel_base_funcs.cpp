#include "tier1/interface.h"
#include "appframework/IAppSystem.h"
#include "qscript_language.h"
#include "qscript_defs.h"
#include "qscript_structs.h"
#include "qscript/qscript.h"
#include "../squirrel/include/squirrel.h"
#include "../squirrel/include/sqstdstring.h"
#include "sqvm.h"
#include "Windows.h"
#include "utlvector.h"
#include "convar.h"
#include "tier1.h"
#include "filesystem.h"
#include "../squirrel/include/sqstdaux.h"
#include "../squirrel/include/sqstdmath.h"

#include "squirrelinterface.hpp"
#include "../squirrel/sqstate.h"
#include "squirrel_base_funcs.hpp"


//DECLARE_SQUIRREL_FUNCTION(warning, "warning", "s", false, SQ)
//DECLARE_SQUIRREL_FUNCTION(error, "error", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(_string_format, "format", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(Squirrel_assert, "assert", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(suspend, "suspend", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(callee, "callee", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(sq_typefunc, "type", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(compilestring, "compilestring", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(newthread, "newthread", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(getstackinfos, "getstackinfos", "s", true, SQ)

//sq_setprintfunc(SQ, emptyprintl, errfunc);
//sq_seterrorcallstackfunc(SQ, errCBfunc);

//DECLARE_SQUIRREL_FUNCTION(getroottable, "getroottable", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(getconsttable, "getconsttable", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(setroottable, "setroottable", "s", true, SQ)
//DECLARE_SQUIRREL_FUNCTION(setconsttable, "setconsttable", "s", true, SQ)

#ifndef NO_GARBAGE_COLLECTOR
    //DECLARE_SQUIRREL_FUNCTION(collectgarbage, "collectgarbage", "s", true, SQ)
    //DECLARE_SQUIRREL_FUNCTION(resurectureachable, "resurectureachable", "s", true, SQ)
#endif


SQInteger sq_print(HSQUIRRELVM SQ)
{
    const char* str;
    sq_getstring(SQ, 2, &str);
    Msg(str);
    return 0;
}

SQInteger sq_printl(HSQUIRRELVM SQ)
{
    const char* str;
    sq_getstring(SQ, 2, &str);
    Msg("%s \n", str);
    return 0;
}

SQInteger sq_warning(HSQUIRRELVM SQ)
{
    const char* str;
    sq_getstring(SQ, 2, &str);
    Warning("%s \n", str);
    return 0;
}

SQInteger sq_error(HSQUIRRELVM SQ)
{
    const SQChar* str;
    if (SQ_SUCCEEDED(sq_tostring(SQ, 2)))
    {
        if (SQ_SUCCEEDED(sq_getstring(SQ, -1, &str)))
        {

            if (_ss(SQ)->_errorfunc) _ss(SQ)->_errorfunc(SQ, _SC("%s"), str);

            return SQ_OK;
        }
    }

    return SQ_ERROR;
}

SQInteger sq_format(HSQUIRRELVM SQ)
{
    return _string_format(SQ);
}

SQInteger sq_assert(HSQUIRRELVM SQ)
{
    return Squirrel_assert(SQ);
}

SQInteger sq_suspend(HSQUIRRELVM SQ)
{
    return sq_suspendvm(SQ);
}

SQInteger sq_callee(HSQUIRRELVM SQ)
{
    return callee(SQ);
}

SQInteger sq_typefunc_func(HSQUIRRELVM SQ)
{
    return sq_typefunc(SQ);
}

SQInteger sq_compilestring(HSQUIRRELVM SQ)
{
    return compilestring(SQ);
}

void sq_printl_err(HSQUIRRELVM SQ, const SQChar* str, ...)
{
    va_list args;
    va_start(args, str);

    Msg(str, args);
    Msg("\n");

    va_end(args);
}

void sq_error_err(HSQUIRRELVM SQ, const SQChar* str, ...)
{

    va_list args;
    va_start(args, str);

    Warning("[Squirrel]: ");
    WarningV(str, args);
    Msg("\n");

    va_end(args);
}


void sq_error_callstack_err(HSQUIRRELVM SQ, const SQChar* str, ...)
{

    va_list args;
    va_start(args, str);

    WarningV(str, args);

    va_end(args);
}






