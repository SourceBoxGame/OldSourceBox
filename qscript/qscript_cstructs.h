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
typedef void(*QCFunc)(QScriptArgs);
#endif



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




#ifdef __cplusplus
}
#endif
#endif