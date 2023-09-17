#ifndef QSCRIPTDEFS_H
#define QSCRIPTDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"

DECLARE_POINTER_HANDLE(QScriptModule);
DECLARE_POINTER_HANDLE(QScriptCallback);

#ifndef QSCIRPTCSTRUCTS_H
DECLARE_POINTER_HANDLE(QScriptReturn);
DECLARE_POINTER_HANDLE(QScriptArgs);
DECLARE_POINTER_HANDLE(QScriptFunction);
DECLARE_POINTER_HANDLE(QScriptObject);
typedef struct QReturn QReturn;
typedef QReturn(*QCFunc)(QScriptArgs);
enum QType
{
    QType_None = 0,
    QType_Int,
    QType_Float,
    QType_String,
    QType_Bool,
    QType_Object,
    QType_Function,


    MAXQTYPE
};

union QValue
{
    int value_int;
    float value_float;
    const char* value_string;
    char* value_modifiable_string;
    bool value_bool;
    QScriptFunction value_function;
    QScriptObject value_object;
};
struct QReturn
{
    enum QType type;
    QValue value;
};
#endif

DECLARE_POINTER_HANDLE(QScriptObjectCreator);
DECLARE_POINTER_HANDLE(QScriptClass);
DECLARE_POINTER_HANDLE(QScriptClassCreator);

typedef struct
{
    QCFunc func;
    const char* name;
    enum QType ret;
    const char* types;

} QModuleDefFunc;




#endif