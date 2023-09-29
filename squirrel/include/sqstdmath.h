/*  see copyright notice in squirrel.h */
#ifndef _SQSTD_MATH_H_
#define _SQSTD_MATH_H_
#include "../../squirrelinterface/squirrelinterface.hpp"
#include "qscript_language.h"
#include "qscript_defs.h"
#include "qscript_structs.h"
#include "qscript/qscript.h"
#ifdef __cplusplus
extern "C" {
#endif


SQUIRREL_API SQRESULT sqstd_register_mathlib(HSQUIRRELVM v);

#define DECL_FUNC_MATH(funcname, sqname, SQ) INIT_SQUIRREL_FUNCTION(math_##funcname, sqname, SQ)

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQSTD_MATH_H_*/
