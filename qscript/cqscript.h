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
	virtual InitReturnVal_t Init();
	virtual void Shutdown();
	virtual QScriptModule CreateModule(const char* name);
	virtual QScriptFunction CreateModuleFunction(QScriptModule module, const char* name, void* func);
private:
	void AddScriptingInterface(const char* name);
	void ImportModules();

	CUtlVector<QModule*>* m_modules;
	CUtlVector<IBaseScriptingInterface*> m_interfaces;
	
};

#endif
