#ifndef QSCIRPTSTRUCTS_H
#define QSCIRPTSTRUCTS_H
#ifdef _WIN32
#pragma once
#endif
#include "utlvector.h"
#include "UtlStringMap.h"
#include "qscript_cstructs.h"
#include "qscript/qscript.h"

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
    void* misc; //use for anything you need
};

enum QExport_Type
{
    QExport_Object,
    QExport_Class,
    QExport_Function
};

struct QExport
{
    const char* name;
    QExport_Type type;
    union
    {
        QObject* obj;
        QClass* cls;
        QFunction* func;
    };
};

struct QInstance
{
    CUtlVector<QExport*> exports;
    void* lang;
    void* env;
};



struct QMod
{
    const char* name;
    CUtlStringMap<QInstance*> instances;
};


#endif