#ifndef IBASESCRIPTINGINTERFACE_H
#define IBASESCRIPTINGINTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "appframework/IAppSystem.h"
#include "qscript_defs.h"

#define QSCRIPT_LANGAUGE_INTERFACE_VERSION "QScriptLanguage001"


abstract_class IBaseScriptingInterface : public CBaseAppSystem<IAppSystem>
{
public:
    virtual InitReturnVal_t Init() = 0;
    virtual void Shutdown() = 0;
    virtual void CreateModule(const char* name) = 0;
    virtual void CreateModuleFunction(QScriptModule module, const char* name, void* func) = 0;
};


#endif
