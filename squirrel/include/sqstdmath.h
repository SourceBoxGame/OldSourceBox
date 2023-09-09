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

#define DECL_FUNC_MATH(funcname, sqname, sqargs, sqnative, SQ)\
	QFunction* funcname##_func = new QFunction();\
    funcname##_func->name = sqname;\
    funcname##_func->args = sqargs;\
    funcname##_func->func = (QCFunc)math_##funcname##;\
    funcname##_func->native = sqnative;\
\
    INIT_SQUIRREL_FUNCTION(funcname, SQ)

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQSTD_MATH_H_*/
