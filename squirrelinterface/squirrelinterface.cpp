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
#include "../squirrel/include/squirrel.h"
#include "Windows.h"
#include "utlvector.h"
#include "convar.h"
#include "tier1.h"
#include "filesystem.h"

IFileSystem* g_pFullFileSystem = 0;
void* current_interface = 0;

class CSquirrelInterface : public IBaseScriptingInterface
{
public:
    virtual InitReturnVal_t Init();
    virtual void Initialize();
    virtual bool Connect(CreateInterfaceFn factory);
    virtual void Shutdown();
    virtual void ImportModules(CUtlVector<QModule*>* modules);
    virtual void LoadMod(const char* path);
    virtual void CallCallback(QCallback* callback, QArgs* args);
    void ExecuteSquirrel(const char* code, int size);
private:
    CUtlVector<QModule*>* m_modules;
};

static CSquirrelInterface s_SquirrelInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CSquirrelInterface, IBaseScriptingInterface, QSCRIPT_LANGAUGE_INTERFACE_VERSION, s_SquirrelInterface);

InitReturnVal_t CSquirrelInterface::Init()
{
    return INIT_OK;
}

bool CSquirrelInterface::Connect(CreateInterfaceFn factory)
{
    ConnectTier1Libraries(&factory, 1);
    ConVar_Register();
    g_pFullFileSystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL);
    current_interface = this;
    return true;
}






void CSquirrelInterface::Initialize()
{

}

void CSquirrelInterface::Shutdown()
{

}






static CUtlBuffer* codebuffer = 0;
void CSquirrelInterface::LoadMod(const char* path)
{
    int len = strlen(path);
    if (!(path[len - 4] == '.' && path[len - 3] == 'n' && path[len - 2] == 'u' && path[len - 1] == 't'))
        return;

    if (codebuffer == 0)
        codebuffer = new CUtlBuffer();

    codebuffer->Clear();

    if (g_pFullFileSystem->ReadFile(path, NULL, *codebuffer))
        ExecuteSquirrel((const char*)(codebuffer->Base()), codebuffer->PeekStringLength());
}

void dumpstack(HSQUIRRELVM SQ) {
    int top = sq_gettop(SQ);
    for (int i = 1; i <= top; i++) {
        Warning("%d\t%s\t", i, sq_gettype(SQ, i));
        switch (sq_gettype(SQ, i)) {
        case OT_FLOAT:
            SQFloat f;
            sq_getfloat(SQ, i, &f);
            Warning("%f\n", f);
            break;
        case OT_STRING:
            const SQChar *str;
            SQInteger size;
            sq_getstringandsize(SQ, i, &str, &size);
            Warning("%s | size: %i\n", str, size);
            break;
        case OT_BOOL:
            SQBool bl;
            sq_getbool(SQ, i, &bl);
            Warning("%s\n", (bl ? "true" : "false"));
            break;
        case OT_NULL:
            Warning("%s\n", "null");
            break;
        default:
            HSQOBJECT ptr;
            sq_getstackobj(SQ, i, &ptr);
            Warning("%p\n", ptr);
            break;
        }
    }
    Warning("\n");
}

void CSquirrelInterface::ExecuteSquirrel(const char* code, int size)
{
    HSQUIRRELVM SQ = sq_open(1024);
    dumpstack(SQ);
    sq_compilebuffer(SQ, code, size, "squirrel", false);
    sq_pushroottable(SQ);
    for (int i = 0; i < m_modules->Count(); i++)
    {
        QModule* mod = m_modules->Element(i);
        sq_pushstring(SQ, mod->name, -1);
        sq_newtable(SQ);
        for (size_t j = 0; j < mod->functions->Count(); j++)
        {
            QFunction* f = mod->functions->Element(j);
            sq_pushstring(SQ, f->name, -1);
            sq_newclosure(SQ, (SQFUNCTION)f, 0);
            sq_newslot(SQ, -3, false);
        }
        sq_newslot(SQ, -3, false);
    }
    
    if (SQ_FAILED(sq_call(SQ, 1,false,false)))
    {
        sq_getlasterror(SQ);
        if (sq_gettype(SQ, -1) == OT_STRING)
        {
            const SQChar *str;
            sq_getstring(SQ, -1, &str);
            Warning("[Squirrel]: %s\n", str);
        }
        sq_poptop(SQ);
    }
    sq_settop(SQ, 0);
}


void CSquirrelInterface::ImportModules(CUtlVector<QModule*>* modules)
{
    m_modules = modules;
}

void CSquirrelInterface::CallCallback(QCallback* callback, QArgs* args)
{
    HSQUIRRELVM SQ = (HSQUIRRELVM)callback->env;
    int top = sq_gettop(SQ);
    sq_pushobject(SQ, *(HSQOBJECT*)(callback->callback));
    sq_pushobject(SQ, *(HSQOBJECT*)(callback->object));
    for (int i = 0; i != args->count; i++)
    {
        switch (args->types[i])
        {
        case 'i':
            sq_pushinteger(SQ, (int)args->args[i]);
            break;
        case 's':
            sq_pushstring(SQ, (const char*)args->args[i],strlen((const char*)args->args[i]));
            break;
        case 'f':
            sq_pushfloat(SQ, *(float*)&args->args[i]);
            break;
        default:
            sq_settop(SQ, top);
            return;
        }
    }
    if (sq_call(SQ, args->count+1, false, false))
    {
        const SQChar* str;
        sq_getstring(SQ, -1, &str);
        Warning("[Squirrel]: %s\n", str);
    }
}

#endif
