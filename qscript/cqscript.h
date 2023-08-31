#ifndef CQSCRIPT_H
#define CQSCRIPT_H
#ifdef _WIN32
#pragma once
#endif

#include "qscript.h"
#include "qscript_language.h"
#include "qscript_structs.h"




class CQScript : public IQScript
{
public:
	CQScript();
	virtual void Initialize();
	virtual bool Connect(CreateInterfaceFn factory);
	virtual InitReturnVal_t Init();
	virtual void Shutdown();
	virtual QScriptModule CreateModule(const char* name);
	virtual QScriptFunction CreateModuleFunction(QScriptModule module, const char* name, const char* args, QCFunc func);
	virtual const char* GetArgString(QScriptArgs args, int argnum);
	virtual int GetArgInt(QScriptArgs args, int argnum);
	virtual float GetArgFloat(QScriptArgs args, int argnum);
private:
	void AddScriptingInterface(const char* name, CreateInterfaceFn factory);
	void ImportModules();

	CUtlVector<QModule*>* m_modules;
	CUtlVector<IBaseScriptingInterface*>* m_interfaces;
	
};

#endif
