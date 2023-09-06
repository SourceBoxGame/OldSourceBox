#ifndef QSCIRPTCSTRUCTS_H
#define QSCIRPTCSTRUCTS_H
#ifdef _WIN32
#pragma once
#endif
#ifdef __cplusplus
extern "C" {
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

enum QType {
	QType_None = 0,
	QType_Int,
	QType_Float,
	QType_String,
	QType_Bool,
	QType_Callback,
	QType_Object
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

typedef struct
{
	enum QType type;
	const char* name;
	void* value;
	unsigned int count;
	QObject* objects;
} QObject;

#ifdef __cplusplus
}
#endif
#endif