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
	CUtlVector<QObject*>* objs;
};


#endif