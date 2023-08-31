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
	const char* name;
	const char* args;
	QCFunc func;
} QFunction;

typedef struct
{
	int count;
	const char* types;
	void** args;
} QArgs;




#ifdef __cplusplus
}
#endif
#endif