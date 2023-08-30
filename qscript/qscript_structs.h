#ifndef QSCIRPTSTRUCTS_H
#define QSCIRPTSTRUCTS_H
#ifdef _WIN32
#pragma once
#endif
#include "utlvector.h"

struct QFunction
{
	const char* name;
	void* func;
};

struct QModule
{
	const char* name;
	CUtlVector<QFunction*>* functions;
};

#endif