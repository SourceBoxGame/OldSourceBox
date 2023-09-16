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
    QType_Function
};

typedef union QValue
{
    int value_int;
    float value_float;
    const char* value_string;
    char* value_modifiable_string;
    bool value_bool;
};

struct QReturn
{
    enum QType type;
    union QValue value;
};
#endif



typedef struct
{
    int count;
    enum QType* types;
} QParams;

typedef struct
{
    const char* name;
    QParams params;
    enum QType type;
    QCFunc func;
} QModuleFunction;

typedef struct
{
    enum QType type;
    union QValue val;
} QArg;

typedef struct QObject QObject;

typedef struct
{
    QObject* self;
    int count;
    QArg args[];
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
    int count;
    const char** names;
    QParams* args;
} QInterface;

enum QFunctionType
{
    QFunction_Native,
    QFunction_Scripting,
    QFunction_Module,
    QFunction_Void,
};

typedef struct 
{
    int always_zero;
    enum QFunctionType type;
    union 
    {
        QCFunc func_native;
        QCallback* func_scripting;
        QModuleFunction* func_module;
        void* func_void;
    };
} QFunction;

typedef struct
{
    enum QType type;
    const char* name;
    int size;
    bool is_private;
} QVar;

typedef struct 
{
    const char* name;
    int vars_count;
    QVar* vars;
    int methods_count;
    QFunction* methods;
    int sigs_count;
    QInterface** sigs;
} QClass;

struct QObject
{
    QClass* cls;
    union QValue vars[];
};

static enum QType strtotype[256] = { QType_None };

#ifdef __cplusplus
}
#endif
#endif