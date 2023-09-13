#ifndef IBASESCRIPTINGINTERFACE_H
#define IBASESCRIPTINGINTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "appframework/IAppSystem.h"
#include "qscript_defs.h"
#include "qscript_structs.h"
#include "utlvector.h"

#define QSCRIPT_LANGAUGE_INTERFACE_VERSION "QScriptLanguage001"


abstract_class IBaseScriptingInterface : public CBaseAppSystem<IAppSystem>
{
public:
    virtual InitReturnVal_t Init() = 0;
    virtual void Initialize() = 0;
    virtual bool Connect(CreateInterfaceFn factory) = 0;
    virtual void Shutdown() = 0;
    virtual void ImportModules(CUtlVector<QModule*>* modules) = 0;
    virtual void LoadMod(const char* path) = 0;
    virtual QReturn CallCallback(QCallback* callback, QArgs* args) = 0;
};


#endif
