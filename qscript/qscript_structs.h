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
	if ((*dst)->type != QType_Object)
		return;
	for (int i = 0; i != (*dst)->value_tree->obj_count; i++)
	{
		QObject* child;
		CopyQObject(&child, src->value_tree->objs[i]);
		(*dst)->value_tree->objs[i] = child;
	}
	for (int i = 0; i != (*dst)->value_tree->method_count; i++)
	{
		QFunction* func = new QFunction();
		(*dst)->value_tree->methods[i] = func;
		memcpy(func, src->value_tree->methods[i],sizeof(QFunction));
	}
	for (int i = 0; i != (*dst)->value_tree->immutable_objs_count; i++)
	{
		QObject* child;
		CopyQObject(&child, src->value_tree->immutable_objs[i]);
		(*dst)->value_tree->immutable_objs[i] = child;
	}
	for (int i = 0; i != (*dst)->value_tree->immutable_methods_count; i++)
	{
		QFunction* func = new QFunction();
		(*dst)->value_tree->immutable_methods[i] = func;
		memcpy(func, src->value_tree->immutable_methods[i], sizeof(QFunction));
	}
}

#endif