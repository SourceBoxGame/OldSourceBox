#ifndef QSCIRPTSTRUCTS_H
#define QSCIRPTSTRUCTS_H
#ifdef _WIN32
#pragma once
#endif
#include "utlvector.h"
#include "qscript_cstructs.h"

struct QModule
{
	const char* name;
	CUtlVector<QFunction*>* functions;
	CUtlVector<QClass*>* classes;
};

struct QClassCreatorMethod
{
    const char* name;
    int params_count;
    QType* params;
    bool is_private;
    bool is_scripting;
    union
    {
        QCFunc native_func;
        QCallback* scripting_func;
    };
};

struct QClassCreator
{
    QClass* parent;
    const char* name;
    CUtlVector<QClassCreatorMethod*> methods;
    CUtlVector<QVar*> vars;
};

#endif