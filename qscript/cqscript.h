#ifndef CQSCRIPT_H
#define CQSCRIPT_H
#ifdef _WIN32
#pragma once
#endif

#include "qscript.h"
#include "qscript_language.h"
#include "qscript_structs.h"
#include "UtlStringMap.h"



class CQScript : public IQScript
{
public:
	CQScript();
	virtual void Initialize();
	virtual bool Connect(CreateInterfaceFn factory);
	virtual InitReturnVal_t Init();
	virtual void Shutdown();
	virtual QScriptModule CreateModule(const char* name, QModuleDefFunc* funcs);
	virtual QScriptFunction CreateModuleFunction(QScriptModule module, const char* name, QType* types, QType returntype, QCFunc funcptr);
	virtual void CreateModuleClass(QScriptModule module, QScriptClass object);
	virtual void LoadFile(const char* path);
	virtual void LoadMods(const char* filename);
	virtual void LoadModsInDirectory(const char* folder, const char* filename);
	virtual QScriptClassCreator StartClass(const char* name, QScriptClass parent = 0);
	virtual void AddMethod(QScriptClassCreator creator, const char* name, QType* params, QCFunc func, bool is_private = false);
	virtual void AddScriptingMethod(QScriptClassCreator creator, const char* name, QScriptCallback callback, bool is_private = false);
	virtual void AddVariable(QScriptClassCreator creator, const char* name, QType type, QValue defaultval, bool is_private = false);
	virtual void AddString(QScriptClassCreator creator, const char* name, const char* defaultval, bool is_private = false);
	virtual QScriptClass FinishClass(QScriptClassCreator creator);
	virtual QScriptObject CreateObject(QScriptClass cls);
	virtual int GetObjectValueIndex(QScriptObject object, const char* name);
	virtual void SetObjectValue(QScriptObject object, int index, QValue val);
	virtual void SetObjectString(QScriptObject object, int index, const char* str);
	virtual QValue GetObjectValue(QScriptObject object, int index);
	virtual QType GetObjectValueType(QScriptObject object, int index);
	virtual int GetObjectMethodIndex(QScriptObject object, const char* name);
	virtual QScriptFunction GetObjectMethod(QScriptObject object, int index);
	virtual QReturn CallObjectMethod(QScriptObject object, int index, QScriptArgs arguments);
	virtual QValue GetArgValue(QScriptArgs args, int index);
	virtual QType GetArgType(QScriptArgs args, int index);
	virtual void InitializeObject(QScriptObject object);
	virtual void CallFunction(QScriptFunction function, const char* fmt, ...);
	virtual void CallFunctionEx(QScriptFunction function, QArgs* args);
private:
	virtual void LoadFilesInDirectory(const char* modname,const char* folder, const char* filename);
	void AddScriptingInterface(const char* name, CreateInterfaceFn factory);
	QMod* GetOrCreateMod(const char* name);

	CUtlVector<QModule*>* m_modules;
	CUtlVector<IBaseScriptingInterface*>* m_interfaces;
	CUtlStringMap<QMod*>* m_mods;
	
};


#endif
