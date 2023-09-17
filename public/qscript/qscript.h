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
    virtual QScriptModule CreateModule(const char* name, QModuleDefFunc* funcs) = 0;
    virtual QScriptFunction CreateModuleFunction(QScriptModule module, const char* name, QType* types, QType returntype, QCFunc funcptr) = 0;
    virtual void CreateModuleClass(QScriptModule module, QScriptClass object) = 0;
    virtual void LoadMods(const char* filename) = 0;
    virtual void LoadModsInDirectory(const char* folder, const char* filename) = 0;
    virtual QScriptClassCreator StartClass(const char* name, QScriptClass parent = 0) = 0;
    virtual void AddMethod(QScriptClassCreator creator, const char* name, QType* params, QCFunc func, bool is_private = false) = 0;
    virtual void AddScriptingMethod(QScriptClassCreator creator, const char* name, QScriptCallback callback, bool is_private = false) = 0;
    virtual void AddVariable(QScriptClassCreator creator, const char* name, QType type, bool is_private = false) = 0;
    virtual void AddString(QScriptClassCreator creator, const char* name, int size, bool is_private = false) = 0;
    virtual QScriptClass FinishClass(QScriptClassCreator creator) = 0;
    virtual QScriptObject CreateObject(QScriptClass cls) = 0;
    virtual int GetObjectValueIndex(QScriptObject object, const char* name) = 0;
    virtual void SetObjectValue(QScriptObject object, int index, QValue val) = 0;
    virtual void SetObjectString(QScriptObject object, int index, const char* str) = 0;
    virtual QValue GetObjectValue(QScriptObject object, int index) = 0;
    virtual QType GetObjectValueType(QScriptObject object, int index) = 0;
    virtual int GetObjectMethodIndex(QScriptObject object, const char* name) = 0;
    virtual QReturn CallObjectMethod(QScriptObject object, int index, QScriptArgs arguments) = 0;
    virtual QValue GetArgValue(QScriptArgs args, int index) = 0;
    virtual QType GetArgType(QScriptArgs args, int index) = 0;
    virtual void InitalizeObject(QScriptObject object) = 0;
    virtual void CallFunction(QScriptFunction function, const char* fmt, ...) = 0;
};



#endif
