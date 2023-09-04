#ifndef CSQUIRRELINTERFACE_H
#define CSQUIRRELTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "appframework/IAppSystem.h"
#include "qscript_language.h"
#include "qscript_defs.h"
#include "qscript_structs.h"
#include "qscript/qscript.h"
#include "../squirrel/include/squirrel.h"
#include "sqvm.h"
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
        //Warning("%d\t%s\t", i, sq_gettype(SQ, i));
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

void print(QScriptArgs args)
{
    Msg("%s", ((const char**)(((QArgs*)args)->args))[0]); //hacky way to reconnect two commands do the same
}

void printl(QScriptArgs args)
{

    Msg("%s\n", ((const char**)(((QArgs*)args)->args))[0]); 
}

void warning(QScriptArgs args)
{
    Warning("%s\n", ((const char**)(((QArgs*)args)->args))[0]); 
}

void scriptassert(QScriptArgs args)
{
    Assert(((bool*)(((QArgs*)args)->args))[0]);
}

void errfunc(HSQUIRRELVM SQ, const SQChar* str, ...)
{
    va_list args;
    va_start(args, str);
    WarningV(str, args);
    va_end(args);
}

void base_commands(HSQUIRRELVM SQ)
{
    //QFunction for each native command, that will call an analog
    QFunction* printfunc = new QFunction();
    printfunc->name = "print";
    printfunc->args = "s";
    printfunc->func = print;

    QFunction* printlfunc = new QFunction();
    printlfunc->name = "printl";
    printlfunc->args = "s";
    printlfunc->func = printl;

    QFunction* warningfunc = new QFunction();
    warningfunc->name = "warning";
    warningfunc->args = "s";
    warningfunc->func = warning;

    QFunction* assertfunc = new QFunction();
    assertfunc->name = "assert";
    assertfunc->args = "b";
    assertfunc->func = (QCFunc)scriptassert;

    QFunction* errorfunc = new QFunction();
    errorfunc->name = "error";
    errorfunc->args = "s";
    errorfunc->func = (QCFunc)error;
    errorfunc->native = 1;

    //funcs
    sq_pushstring(SQ, printfunc->name, -1);
    sq_newclosure(SQ, (SQFUNCTION)printfunc, 0);
    sq_setnativeclosurename(SQ, -1, printfunc->name);
    sq_newslot(SQ, -3, false);

    sq_pushstring(SQ, printlfunc->name, -1);
    sq_newclosure(SQ, (SQFUNCTION)printlfunc, 0);
    sq_setnativeclosurename(SQ, -1, printlfunc->name);
    sq_newslot(SQ, -3, false);

    sq_pushstring(SQ, warningfunc->name, -1);
    sq_newclosure(SQ, (SQFUNCTION)warningfunc, 0);
    sq_setnativeclosurename(SQ, -1, warningfunc->name);
    sq_newslot(SQ, -3, false);

    /*sq_pushstring(SQ, assertfunc->name, -1);
    sq_newclosure(SQ, (SQFUNCTION)assertfunc, 0);
    sq_setnativeclosurename(SQ, -1, assertfunc->name);
    sq_newslot(SQ, -3, false);

    sq_pushstring(SQ, errorfunc->name, -1);
    sq_newclosure(SQ, (SQFUNCTION)errorfunc, 0);
    sq_setnativeclosurename(SQ, -1, errorfunc->name);
    sq_newslot(SQ, -3, false);*/

   /*sq_pushstring(SQ, _SC("error"), -1);
    sq_newclosure(SQ, (SQFUNCTION)error, 0);
    sq_newslot(SQ, -3, false);*/

        //generic
    
    /*    1, _SC("seterrorhandler"), NULL, (QCFunc)base_seterrorhandler, 2
    
    { 1,_SC("setdebughook"),NULL,(QCFunc)base_setdebughook,2 },
    { 1,_SC("enabledebuginfo"),NULL,(QCFunc)base_enabledebuginfo,2 },
    { 1,_SC("getstackinfos"),".n",(QCFunc)base_getstackinfos,2 },
    { 1,_SC("getroottable"),NULL,(QCFunc)base_getroottable,1 },
    { 1,_SC("setroottable"),NULL,(QCFunc)base_setroottable,2 },
    { 1,_SC("getconsttable"),NULL,(QCFunc)base_getconsttable,1 },
    { 1,_SC("setconsttable"),NULL,(QCFunc)base_setconsttable,2 },
    { 1,_SC("assert"),NULL,(QCFunc)base_assert,-2 },
        //{1,_SC("print"),NULL,(QCFunc)base_print,2},
    { 1,_SC("print"),NULL,(QCFunc)printf,2 },
    { 1,_SC("error"),NULL,(QCFunc)base_error,2 },
    { 1,_SC("compilestring"),".ss",(QCFunc)base_compilestring,-2 },
    { 1,_SC("newthread"),".c",(QCFunc)base_newthread,2 },
    { 1,_SC("suspend"),NULL,(QCFunc)base_suspend,-1 },
    { 1,_SC("array"),".n",(QCFunc)base_array,-2 },
    { 1,_SC("type"),NULL,(QCFunc)base_type,2 },
    { 1,_SC("callee"),NULL,(QCFunc)base_callee,0 },
    { 1,_SC("dummy"),NULL,(QCFunc)base_dummy,0 },
#ifndef NO_GARBAGE_COLLECTOR
    { 1,_SC("collectgarbage"),NULL,(QCFunc)base_collectgarbage,0 },
    { 1,_SC("resurrectunreachable"),NULL,(QCFunc)base_resurectureachable,0 },
#endif*/

    // constants
    sq_pushstring(SQ, _SC("_versionnumber_"), -1);
    sq_pushinteger(SQ, SQUIRREL_VERSION_NUMBER);
    sq_newslot(SQ, -3, SQFalse);
    sq_pushstring(SQ, _SC("_version_"), -1);
    sq_pushstring(SQ, SQUIRREL_VERSION, -1);
    sq_newslot(SQ, -3, SQFalse);
    sq_pushstring(SQ, _SC("_charsize_"), -1);
    sq_pushinteger(SQ,sizeof(SQChar));
    sq_newslot(SQ,-3, SQFalse);
    sq_pushstring(SQ,_SC("_intsize_"),-1);
    sq_pushinteger(SQ,sizeof(SQInteger));
    sq_newslot(SQ,-3, SQFalse);
    sq_pushstring(SQ,_SC("_floatsize_"),-1);
    sq_pushinteger(SQ,sizeof(SQFloat));
    sq_newslot(SQ,-3, SQFalse);

    sq_pushstring(SQ, _SC("_endl_"), -1); 
    sq_pushstring(SQ, _SC("\n"), -1);
    sq_newslot(SQ, -3, SQFalse);
}


void CSquirrelInterface::ExecuteSquirrel(const char* code, int size)
{
    HSQUIRRELVM SQ = sq_open(1024);

    dumpstack(SQ);
    sq_enabledebuginfo(SQ, true);

    sq_setdebughook(SQ);

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

    //sq_seterrorfunc(SQ, errfunc);
    base_commands(SQ);

    if (SQ_FAILED(sq_call(SQ, 1,false,false)))
    {
        sq_getlasterror(SQ);
        if (sq_gettype(SQ, -1) == OT_STRING)
        {
            const SQChar *str;
            sq_getstring(SQ, -1, &str);
            Warning("[Squirrel]: %s\n", str);
            //SQStackInfos si;
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
