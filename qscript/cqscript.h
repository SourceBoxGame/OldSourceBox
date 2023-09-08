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
	virtual void CreateModuleObject(QScriptModule module, QScriptObject object);
	virtual const char* GetArgString(QScriptArgs args, int argnum);
	virtual char* GetArgPermaString(QScriptArgs args, int argnum);
	virtual int GetArgInt(QScriptArgs args, int argnum);
	virtual float GetArgFloat(QScriptArgs args, int argnum);
	virtual bool GetArgBool(QScriptArgs args, int argnum);
	virtual QScriptCallback GetArgCallback(QScriptArgs args, int argnum);
	virtual void LoadMods(const char* filename);
	virtual void LoadModsInDirectory(const char* folder, const char* filename);
	virtual void CallCallback(QScriptCallback callback, QScriptArgs args);
	virtual QScriptArgs CreateArgs(const char* types, ...);
	virtual void FreeArgs(QScriptArgs a);
	virtual QScriptReturn RetNone();
	virtual QScriptReturn RetInt(int value);
	virtual QScriptReturn RetFloat(float value);
	virtual QScriptReturn RetBool(bool value);
	virtual QScriptReturn RetString(const char* value);
	virtual QScriptObject GetObjectElementByName(QScriptObject obj, const char* name);
	virtual int GetObjectInt(QScriptObject object);
	virtual float GetObjectFloat(QScriptObject object);
	virtual const char* GetObjectString(QScriptObject object);
	virtual bool GetObjectBool(QScriptObject object);
	virtual void* GetObjectVoid(QScriptObject object);
	virtual QType GetObjectType(QScriptObject object);
	virtual void SetObjectValue(QScriptObject object, QType type, void* val);
	virtual void SetObjectInt(QScriptObject object, int val);
	virtual void SetObjectFloat(QScriptObject object, float val);
	virtual void SetObjectString(QScriptObject object, const char* val);
	virtual void SetObjectBool(QScriptObject object, bool val);
	virtual void SetObjectFunction(QScriptObject object, QScriptFunction val);
private:
	virtual void LoadFilesInDirectory(const char* folder, const char* filename);
	void AddScriptingInterface(const char* name, CreateInterfaceFn factory);

	CUtlVector<QModule*>* m_modules;
	CUtlVector<IBaseScriptingInterface*>* m_interfaces;
	
};


#endif
