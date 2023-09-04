#ifndef QSCRIPTDEFS_H
#define QSCRIPTDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"

DECLARE_POINTER_HANDLE(QScriptModule);
DECLARE_POINTER_HANDLE(QScriptFunction);

#ifndef QSCIRPTCSTRUCTS_H
DECLARE_POINTER_HANDLE(QScriptArgs);
typedef void(*QCFunc)(QScriptArgs);
#endif

DECLARE_POINTER_HANDLE(QScriptCallback);

#endif