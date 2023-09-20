/* see copyright notice in squirrel.h */
#include <include/squirrel.h>
#include <math.h>
#include <stdlib.h>
#include <include/sqstdmath.h>
#include "../../squirrelinterface/squirrelinterface.hpp"

#define SINGLE_ARG_FUNC(_funcname) static SQInteger math_##_funcname(HSQUIRRELVM v){ \
    SQFloat f; \
    sq_getfloat(v,2,&f); \
    sq_pushfloat(v,(SQFloat)_funcname(f)); \
    return 1; \
}

#define _FUNC(_funcname) static SQInteger math_##_funcname(HSQUIRRELVM v){ \
    sq_pushfloat(v,(SQFloat)_funcname()); \
    return 1; \
}

#define SINGLE_ARG_VOID_FUNC(_funcname) static SQInteger math_##_funcname(HSQUIRRELVM v){ \
    SQFloat f; \
    sq_getfloat(v,2,&f); \
    _funcname(f); \
    return 0; \
}
#define TWO_ARGS_FUNC(_funcname) static SQInteger math_##_funcname(HSQUIRRELVM v){ \
    SQFloat p1,p2; \
    sq_getfloat(v,2,&p1); \
    sq_getfloat(v,3,&p2); \
    sq_pushfloat(v,(SQFloat)_funcname(p1,p2)); \
    return 1; \
}

static SQInteger math_abs(HSQUIRRELVM v)
{
    SQInteger n;
    sq_getinteger(v,2,&n);
    sq_pushinteger(v,(SQInteger)abs((int)n));
    return 1;
}

int sq_RandomInteger(int min, int max)
{
    return RandomInt(min, max);
}

int sq_srand()
{
    return RandomInt(0, RAND_MAX);
}


float sq_srandf()
{
    return RandomFloat(0, RAND_MAX);
}

void sq_randseed(float seed)
{
    RandomSeed(seed);
}

SQInteger sq_map(HSQUIRRELVM v)
{
    SQFloat p1, p2, p3, p4, p5; 
    sq_getfloat(v, 2, &p1); 
    sq_getfloat(v, 3, &p2); 
    sq_getfloat(v, 4, &p3); 
    sq_getfloat(v, 5, &p4); 
    sq_getfloat(v, 5, &p5); 

    sq_pushfloat(v, (SQFloat)RemapVal(p1, p2, p3, p4, p5));
    return 1; 
}

SINGLE_ARG_FUNC(sqrt)
SINGLE_ARG_FUNC(fabs)
SINGLE_ARG_FUNC(sin)
SINGLE_ARG_FUNC(cos)
SINGLE_ARG_FUNC(asin)
SINGLE_ARG_FUNC(acos)
SINGLE_ARG_FUNC(log)
SINGLE_ARG_FUNC(log10)
SINGLE_ARG_FUNC(tan)
SINGLE_ARG_FUNC(atan)
TWO_ARGS_FUNC(atan2)
TWO_ARGS_FUNC(pow)
SINGLE_ARG_FUNC(floor)
SINGLE_ARG_FUNC(ceil)
SINGLE_ARG_FUNC(exp)

TWO_ARGS_FUNC(sq_RandomInteger)

_FUNC(sq_srand)
_FUNC(sq_srandf)
_FUNC(rand)
//SINGLE_ARG_VOID_FUNC(sq_randseed)

/*#define _DECL_FUNC(name,nparams,tycheck) {_SC(#name),math_##name,nparams,tycheck}
static const SQRegFunction mathlib_funcs[] = {
    _DECL_FUNC(sqrt,2,_SC(".n")),
    _DECL_FUNC(sin,2,_SC(".n")),
    _DECL_FUNC(cos,2,_SC(".n")),
    _DECL_FUNC(asin,2,_SC(".n")),
    _DECL_FUNC(acos,2,_SC(".n")),
    _DECL_FUNC(log,2,_SC(".n")),
    _DECL_FUNC(log10,2,_SC(".n")),
    _DECL_FUNC(tan,2,_SC(".n")),
    _DECL_FUNC(atan,2,_SC(".n")),
    _DECL_FUNC(atan2,3,_SC(".nn")),
    _DECL_FUNC(pow,3,_SC(".nn")),
    _DECL_FUNC(floor,2,_SC(".n")),
    _DECL_FUNC(ceil,2,_SC(".n")),
    _DECL_FUNC(exp,2,_SC(".n")),
    _DECL_FUNC(srand,2,_SC(".n")),
    _DECL_FUNC(rand,1,NULL),
    _DECL_FUNC(fabs,2,_SC(".n")),
    _DECL_FUNC(abs,2,_SC(".n")),
    {NULL,(SQFUNCTION)0,0,NULL}
};
#undef _DECL_FUNC*/

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

SQRESULT sqstd_register_mathlib(HSQUIRRELVM v)
{
    SQInteger i = 0;
    /*while (mathlib_funcs[i].name != 0) {
        sq_pushstring(v,mathlib_funcs[i].name,-1);
        sq_newclosure(v,mathlib_funcs[i].f,0);
        sq_setparamscheck(v,mathlib_funcs[i].nparamscheck,mathlib_funcs[i].typemask);
        sq_setnativeclosurename(v,-1,mathlib_funcs[i].name);
        sq_newslot(v,-3,SQFalse);
        i++;
    }*/

    DECL_FUNC_MATH(sqrt, "sqrt", v)
    DECL_FUNC_MATH(sin, "sin", v)
    DECL_FUNC_MATH(cos, "cos", v)
    DECL_FUNC_MATH(asin, "asin", v)
    DECL_FUNC_MATH(acos, "acos", v)
    DECL_FUNC_MATH(log, "log", v)
    DECL_FUNC_MATH(log10, "log10", v)
    DECL_FUNC_MATH(tan, "tan", v)
    DECL_FUNC_MATH(atan, "atan", v)
    DECL_FUNC_MATH(atan2, "atan2ue", v)
    DECL_FUNC_MATH(pow, "pow", v)
    DECL_FUNC_MATH(floor, "floor", v)
    DECL_FUNC_MATH(ceil, "ceil", v)
    DECL_FUNC_MATH(exp, "exp", v)
    DECL_FUNC_MATH(sq_srand, "srand", v)
    DECL_FUNC_MATH(sq_srandf, "srandf", v)
    DECL_FUNC_MATH(rand, "rand", v)
    DECL_FUNC_MATH(fabs, "fabs", v)
    DECL_FUNC_MATH(abs, "abs", v)
    DECL_FUNC_MATH(sq_RandomInteger, "randi", v)
    //DECL_FUNC_MATH(sq_randseed, "randseed", v)

    INIT_SQUIRREL_FUNCTION(sq_map, "map", v)

    DECLARE_SQUIRREL_VARIABLE_INTEGER(RAND_MAX, "RAND_MAX", v)
    DECLARE_SQUIRREL_VARIABLE_FLOAT(M_PI, "PI", v)

    //sq_pushstring(v, _SC("RAND_MAX"), -1);
    //sq_pushinteger(v,RAND_MAX);
    //sq_newslot(v,-3,SQFalse);
    //sq_pushstring(v,_SC("PI"),-1);
    //sq_pushfloat(v,(SQFloat)M_PI);
    //sq_newslot(v,-3,SQFalse);
    return SQ_OK;
}

