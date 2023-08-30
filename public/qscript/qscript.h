#ifndef IQSCRIPT_H
#define IQSCRIPT_H
#ifdef _WIN32
#pragma once
#endif

#include "qscript_defs.h"
#include "appframework/IAppSystem.h"

#define QSCRIPT_INTERFACE_VERSION "QScript001"



abstract_class IQScript : public CBaseAppSystem<IAppSystem>
{
public:
    virtual void Initialize() = 0;
    virtual InitReturnVal_t Init() = 0;
    virtual void Shutdown() = 0;
    virtual QScriptModule CreateModule(const char* name) = 0;
    virtual QScriptFunction CreateModuleFunction(QScriptModule module, const char* name, void* func) = 0;
};


#endif
