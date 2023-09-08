#ifndef QSCRIPTDEFS_H
#define QSCRIPTDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"

DECLARE_POINTER_HANDLE(QScriptModule);
DECLARE_POINTER_HANDLE(QScriptFunction);

#ifndef QSCIRPTCSTRUCTS_H
DECLARE_POINTER_HANDLE(QScriptReturn);
DECLARE_POINTER_HANDLE(QScriptArgs);
typedef QScriptReturn(*QCFunc)(QScriptArgs);
#endif

DECLARE_POINTER_HANDLE(QScriptCallback);
DECLARE_POINTER_HANDLE(QScriptObject);
DECLARE_POINTER_HANDLE(QScriptObjectCreator);

#endif