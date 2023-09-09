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

typedef struct 
{
	unsigned char native;
	const char* name;
	const char* args;
	QCFunc func;
	int etc_data;
} QFunction;

typedef struct
{
	int count;
	const char* types;
	void** args;
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

typedef struct QObject QObject;

struct QObject
{
	const char* name;
	enum QType type;
	int count;
	union
	{
		int value_int;
		float value_float;
		const char* value_string;
		bool value_bool;
		void* value_raw;
		QFunction* value_function;
		QObject** objs;
	};
};


#ifdef __cplusplus
}
#endif
#endif