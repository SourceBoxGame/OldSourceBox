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

void CopyQObject(QObject **dst, QObject* src)
{
	*dst = new QObject();
	memcpy(*dst, src, sizeof(QObject));
	for (int i = 0; i != (*dst)->count; i++)
	{
		QObject* child;
		CopyQObject(&child, src->objs[i]);
		(*dst)->objs[i] = child;
	}
}

#endif