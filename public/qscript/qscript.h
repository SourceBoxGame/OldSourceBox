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
    virtual bool Connect(CreateInterfaceFn factory) = 0;
    virtual InitReturnVal_t Init() = 0;
    virtual void Shutdown() = 0;
    virtual QScriptModule CreateModule(const char* name) = 0;
    virtual QScriptFunction CreateModuleFunction(QScriptModule module, const char* name, const char* args, QCFunc func) = 0;
    virtual const char* GetArgString(QScriptArgs args, int argnum) = 0;
    virtual char* GetArgPermaString(QScriptArgs args, int argnum) = 0;
    virtual int GetArgInt(QScriptArgs args, int argnum) = 0;
    virtual float GetArgFloat(QScriptArgs args, int argnum) = 0;
    virtual QScriptCallback GetArgCallback(QScriptArgs args, int argnum) = 0;
    virtual void LoadMods(const char* filename) = 0;
    virtual void LoadModsInDirectory(const char* folder, const char* filename) = 0;
    virtual void CallCallback(QScriptCallback callback, QScriptArgs args) = 0;
    virtual QScriptArgs CreateArgs(const char* types, ...) = 0;
    virtual void FreeArgs(QScriptArgs a) = 0;
    virtual QScriptReturn RetNone() = 0;
    virtual QScriptReturn RetInt(int value) = 0;
    virtual QScriptReturn RetFloat(float value) = 0;
    virtual QScriptReturn RetBool(bool value) = 0;
    virtual QScriptReturn RetString(const char* value) = 0;
};



#endif
