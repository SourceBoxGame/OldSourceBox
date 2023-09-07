#ifndef CSQUIRRELINTERFACE
#define CSQUIRRELTERFACE
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
#include "../squirrel/include/sqstdstring.h"
#include "sqvm.h"
#include "Windows.h"
#include "utlvector.h"
#include "convar.h"
#include "tier1.h"
#include "filesystem.h"
#include "../squirrel/include/sqstdaux.h"

#include "squirrelinterface.hpp"

IFileSystem* g_pFullFileSystem = 0;
IQScript* qscript = 0;
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
    qscript = (IQScript*)factory(QSCRIPT_INTERFACE_VERSION, NULL);
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
            //Warning("%f\n", f);
            break;
        case OT_STRING:
            const SQChar *str;
            SQInteger size;
            sq_getstringandsize(SQ, i, &str, &size);
            //Warning("%s | size: %i\n", str, size);
            break;
        case OT_BOOL:
            SQBool bl;
            sq_getbool(SQ, i, &bl);
            //Warning("%s\n", (bl ? "true" : "false"));
            break;
        case OT_NULL:
            //Warning("%s\n", "null");
            break;
        default:
            HSQOBJECT ptr;
            sq_getstackobj(SQ, i, &ptr);
            //Warning("%p\n", ptr);
            break;
        }
    }
    Warning("\n");
}

QScriptReturn print(QScriptArgs args)
{
    Msg("%s", ((const char**)(((QArgs*)args)->args))[0]); //hacky way to reconnect two commands do the same
    return qscript->RetNone();
}

QScriptReturn printl(QScriptArgs args)
{
    Msg("%s\n", ((const char**)(((QArgs*)args)->args))[0]); 
    return qscript->RetNone();
}

QScriptReturn warning(QScriptArgs args)
{
    Warning("%s\n", ((const char**)(((QArgs*)args)->args))[0]); 
    return qscript->RetNone();
}


void errfunc(HSQUIRRELVM SQ, const SQChar* str, ...)
{
    va_list args;
    va_start(args, str);
    Warning("[Squirrel]: ");
    WarningV(str, args);
    Msg("\n");

    //sqstd_printcallstack(SQ);
    /*SQInteger level;
    sq_getinteger(SQ, -1, &level);
    write_callstack(SQ, level);*/

    va_end(args);
}



void base_commands(HSQUIRRELVM SQ)
{

    DECLARE_SQUIRREL_FUNCTION(print, "print", "s", false, SQ)
    //DECLARE_SQUIRREL_FUNCTION(sq_printf, "printf", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(printl, "printl", "s", false, SQ)
    DECLARE_SQUIRREL_FUNCTION(warning, "warning", "s", false, SQ)
    DECLARE_SQUIRREL_FUNCTION(error, "error", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(_string_format, "format", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(Squirrel_assert, "assert", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(suspend, "suspend", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(callee, "callee", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(sq_typefunc, "type", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(compilestring, "compilestring", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(newthread, "newthread", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(getstackinfos, "getstackinfos", "s", true, SQ)

    DECLARE_SQUIRREL_FUNCTION(getroottable, "getroottable", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(getconsttable, "getconsttable", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(setroottable, "setroottable", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(setconsttable, "setconsttable", "s", true, SQ)

#ifndef NO_GARBAGE_COLLECTOR
    DECLARE_SQUIRREL_FUNCTION(collectgarbage, "collectgarbage", "s", true, SQ)
    DECLARE_SQUIRREL_FUNCTION(resurectureachable, "resurectureachable", "s", true, SQ)
#endif


    // constants
    DECLARE_SQUIRREL_VARIABLE_INTEGER(SQUIRREL_VERSION_NUMBER, "_versionnumber_", SQ)
    DECLARE_SQUIRREL_VARIABLE_STRING(SQUIRREL_VERSION, "_version_", SQ)
    DECLARE_SQUIRREL_VARIABLE_INTEGER(sizeof(SQChar), "_charsize_", SQ)
    DECLARE_SQUIRREL_VARIABLE_INTEGER(sizeof(SQInteger), "_intsize_", SQ)
    DECLARE_SQUIRREL_VARIABLE_INTEGER(sizeof(SQFloat), "_floatsize_", SQ)
    DECLARE_SQUIRREL_VARIABLE_STRING("\n", "_endl_", SQ)


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
}


void CSquirrelInterface::ExecuteSquirrel(const char* code, int size)
{
    HSQUIRRELVM SQ = sq_open(VM_STATIC_STACKSIZE * 2); //i dont think we will get enough stacksize with just 1024

    //sq_setforeignptr(SQ, this);
    dumpstack(SQ);
    sq_enabledebuginfo(SQ, true);

    sq_setdebughook(SQ);
    sq_compilebuffer(SQ, code, size, "squirrel", SQTrue);

    sq_pushroottable(SQ);
    //sqstd_seterrorhandlers(SQ);
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

    base_commands(SQ);
    //sq_seterrorfunc(SQ, errfunc);


    if (SQ_FAILED(sq_call(SQ, 1,false,true)))
    {
        sq_getlasterror(SQ);
        if (sq_gettype(SQ, -1) == OT_STRING)
        {
            const SQChar *str;
            sq_getstring(SQ, -1, &str);
            Warning("[Squirrel]: %s\n", str);

            /*SQInteger level;
            sq_getinteger(SQ, -1, &level);
            write_callstack(SQ, level);*/

            //sqstd_printcallstack(SQ);
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
        case 'b':
            sq_pushbool(SQ, *(bool*)&args->args[i]);
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

void write_callstack(HSQUIRRELVM SQ, SQInteger level)
{
    SQStackInfos si;
    SQInteger seq = 0;
    const SQChar* name = NULL;

    Warning("[Squirrel Virtual Machine]: Trying to get data from stack\n");

    //if (SQ_SUCCEEDED(sq_stackinfos(SQ, level, &si)))
    if (SQ_SUCCEEDED(sq_stackinfos(SQ, 0, &si)))
    {
        const SQChar* fn = _SC("unknown");
        const SQChar* src = _SC("unknown");
        if (si.funcname)fn = si.funcname;
        if (si.source)src = si.source;

        Warning("Function: %s\n", fn);
        Warning("Line: %s\n", si.line);

        seq = 0;
        while ((name = sq_getlocal(SQ, level, seq))) {
            
            Warning("Table: %s\n", name);
            
            /*sq_pushstring(SQ, name, -1);
            sq_push(SQ, -2);
            sq_newslot(SQ, -4, SQFalse);
            sq_pop(SQ, 1);*/
            seq++;
        }

        
    }

    Warning("[Squirrel Virtual Machine]: Done\n");
}

#endif
