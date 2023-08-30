#ifndef CPYTHONINTERFACE_H
#define CPYTHONINTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "appframework/IAppSystem.h"
#include "qscript_language.h"
#include "qscript_defs.h"
#include "qscript_structs.h"


abstract_class CPythonInterface : public IBaseScriptingInterface
{
public:
    virtual InitReturnVal_t Init();
    virtual void Shutdown();
    virtual void CreateModule(const char* name);
    virtual void CreateModuleFunction(QScriptModule module, const char* name, void* func);
private:
    CUtlVector<QModule*>* m_modules;
};

static CPythonInterface s_PythonInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CPythonInterface, IBaseScriptingInterface, QSCRIPT_LANGAUGE_INTERFACE_VERSION, s_PythonInterface);


#endif
