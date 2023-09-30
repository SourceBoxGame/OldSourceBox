
#ifndef CSQUIRRELINTERFACE_H
#define CSQUIRRELTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "appframework/IAppSystem.h"
#include "qscript_language.h"
#include "qscript_defs.h"
#include "qscript_structs.h"
#include "qscript/qscript.h"
#include "../squirrel/include/squirrel.h"
#include "../squirrel/include/sqstdstring.h"
#include "sqvm.h"
#include "utlvector.h"
#include "convar.h"
#include "tier1.h"
#include "filesystem.h"

#define VM_STATIC_STACKSIZE 1024

#define INIT_SQUIRREL_FUNCTION(funcname,sqname, SQ)\
    sq_pushstring(SQ, sqname, -1);\
    sq_newclosure(SQ, funcname, 0);\
    sq_setnativeclosurename(SQ, -1, sqname);\
    sq_newslot(SQ, -3, false);


#define DECLARE_SQUIRREL_FUNCTION(funcname, sqname, SQ) INIT_SQUIRREL_FUNCTION(funcname,sqname, SQ)


#define DECLARE_SQUIRREL_VARIABLE_INTEGER(var, sqname, SQ)\
        sq_pushstring(SQ, _SC(sqname), -1);\
        sq_pushinteger(SQ, var);\
        sq_newslot(SQ, -3, SQFalse);

#define DECLARE_SQUIRREL_VARIABLE_FLOAT(var, sqname, SQ)\
        sq_pushstring(SQ, _SC(sqname), -1);\
        sq_pushfloat(SQ, var);\
        sq_newslot(SQ, -3, SQFalse);


#define DECLARE_SQUIRREL_VARIABLE_STRING(var, sqname, SQ)\
        sq_pushstring(SQ, _SC(sqname), -1);\
        sq_pushstring(SQ, _SC(var), -1);\
        sq_newslot(SQ, -3, SQFalse);


void write_callstack(HSQUIRRELVM SQ, SQInteger level);



#endif
