

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

SQInteger sq_print(HSQUIRRELVM SQ);
SQInteger sq_printl(HSQUIRRELVM SQ);
SQInteger sq_warning(HSQUIRRELVM SQ);
SQInteger sq_error(HSQUIRRELVM SQ);
SQInteger sq_format(HSQUIRRELVM SQ);
SQInteger sq_assert(HSQUIRRELVM SQ);
SQInteger sq_suspend(HSQUIRRELVM SQ);
SQInteger sq_callee(HSQUIRRELVM SQ);
SQInteger sq_typefunc_func(HSQUIRRELVM SQ);
SQInteger sq_compilestring(HSQUIRRELVM SQ);
/*SQInteger sq_getstackinfos(HSQUIRRELVM SQ);
SQInteger sq_getroottable(HSQUIRRELVM SQ);
SQInteger sq_getconsttable(HSQUIRRELVM SQ);
SQInteger sq_setroottable(HSQUIRRELVM SQ);
SQInteger sq_setconsttable(HSQUIRRELVM SQ);*/

void sq_printl_err(HSQUIRRELVM SQ, const SQChar* str, ...);
void sq_error_err(HSQUIRRELVM SQ, const SQChar* str, ...);
void sq_error_callstack_err(HSQUIRRELVM SQ, const SQChar* str, ...);