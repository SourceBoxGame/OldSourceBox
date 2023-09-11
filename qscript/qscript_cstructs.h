#ifndef QSCIRPTCSTRUCTS_H
#define QSCIRPTCSTRUCTS_H
#ifdef _WIN32
#pragma once
#endif
#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif


#ifndef QSCRIPTDEFS_H
typedef struct {
	int unused;
}* QScriptArgs;
typedef struct {
	int unused;
}* QScriptReturn;
typedef QScriptReturn(*QCFunc)(QScriptArgs);
#endif

enum QType
{
    QType_None = 0,
    QType_Int,
    QType_Float,
    QType_String,
    QType_Bool,
    QType_Object,
    QType_Function
};

union QValue
{
    int value_int;
    float value_float;
    const char* value_string;
    bool value_bool;
};

typedef struct
{
    bool native;
    QCFunc func;
} QFunction;

typedef struct
{
    QType type;
    QValue val;
} QArg;

typedef struct
{
    int count;
    QArg* args;
} QArgs;

typedef struct
{
    void* lang;
    void* callback;
    void* object;
    void* env;
} QCallback;

typedef struct
{
    enum QType type;
    void* value;
} QReturn;

typedef struct
{
    int count;
    enum QType* types;
} QParams;

typedef struct 
{
    int count;
    const char** names;
    QParams* args;
} QInterface;

typedef struct 
{
    bool is_scripting;
    union 
    {
        QFunction* func_native;
        QCallback* func_scripting;
    };
} QObjectFunction;

typedef struct 
{
    int vars_count;
    enum QType* vars_types;
    const char** vars_names;
    int methods_count;
    QObjectFunction* methods;
    int sigs_count;
    QInterface* sigs;
} QClass;

typedef struct 
{
    QClass* cls;
    QValue vars[];
} QObject;

#ifdef __cplusplus
}
#endif
#endif