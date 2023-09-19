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
#include "../squirrel/include/sqstdmath.h"

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
    virtual QInstance* LoadMod(QMod* mod, const char* path);
    virtual QReturn CallCallback(QCallback* callback, QArgs* args);
    QInstance* ExecuteSquirrel(QMod* mod, const char* code, int size);
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
QInstance* CSquirrelInterface::LoadMod(QMod* mod, const char* path)
{
    int len = strlen(path);
    if (!(path[len - 4] == '.' && path[len - 3] == 'n' && path[len - 2] == 'u' && path[len - 1] == 't'))
        return 0;

    if (codebuffer == 0)
        codebuffer = new CUtlBuffer();

    codebuffer->Clear();

    if (g_pFullFileSystem->ReadFile(path, NULL, *codebuffer))
        return ExecuteSquirrel(mod, (const char*)(codebuffer->Base()), codebuffer->PeekStringLength());
}

void dumpstack(HSQUIRRELVM SQ) {
    int top = sq_gettop(SQ);
    for (int i = 1; i <= top; i++) {
        Warning("%d\t%x\t", i, sq_gettype(SQ, i));
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
/*
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
    //SQInteger level;
    //sq_getinteger(SQ, -1, &level);
    //write_callstack(SQ, level);

    va_end(args);
}

void errCBfunc(HSQUIRRELVM SQ, const SQChar* str, ...)
{
    va_list args;
    va_start(args, str);
    WarningV(str, args);
    va_end(args);

}


void emptyprintl(HSQUIRRELVM SQ, const SQChar* str, ...)
{
    Msg("%s\n", str);
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

    sq_setprintfunc(SQ, emptyprintl, errfunc);
    sq_seterrorcallstackfunc(SQ, errCBfunc);

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


}
*/
struct SQ_Userdata
{
    union {
        QObject* obj;
        QClass* cls;
        QFunction* func;
    };
};

SQInteger Squirrel_Class_Call(HSQUIRRELVM v)
{
    SQ_Userdata* usr;
    SQUserPointer tag;
    if (SQ_FAILED(sq_getuserdata(v, 1, (SQUserPointer*)&usr, &tag)))
        return 0; // TODO : error because bad udata
    if (tag != "QSCRIPT_CLASS") // :letroll:
        return 0;
    QClass* cls = usr->cls;
    QObject* obj = (QObject*)malloc(sizeof(QObject) + cls->vars_count * sizeof(QValue));
    obj->cls = cls;
    qscript->InitalizeObject((QScriptObject)obj);
    ((SQ_Userdata*)sq_newuserdata(v, sizeof(SQ_Userdata)))->obj = obj;
    sq_settypetag(v, -1, "QSCRIPT_OBJECT");
    sq_pushregistrytable(v);
    sq_pushstring(v, "QSCRIPT_OBJECT",-1);
    sq_get(v, -2);
    sq_remove(v, -2);
    sq_setdelegate(v, -2);
    return 1;
}
SQInteger Squirrel_Class_Inherited(HSQUIRRELVM v)
{
    SQ_Userdata* usr;
    SQUserPointer tag;
    if (SQ_FAILED(sq_getuserdata(v, 1, (SQUserPointer*)&usr, &tag)))
        return 0; // TODO : error because bad udata
    if (tag != "QSCRIPT_CLASS") // :letroll:
        return 0;
    QClass* cls = usr->cls;
    int index = 0;
    for (int i = 0; i < cls->sigs_count; i++)
    {
        QInterface* sig = cls->sigs[i];
        for (int j = 0; j < sig->count; j++)
        {
            QFunction* func = &cls->methods[index];
            sq_pushstring(v, sig->names[i], -1);
            switch (func->type)
            {
            case QFunction_Module:
                sq_newclosure(v, (SQFUNCTION)func, 0);
                break;
            case QFunction_Native:
                sq_newclosure(v, (SQFUNCTION)func, 0);
                break;
            case QFunction_Scripting:
                sq_pushobject(v, *(HSQOBJECT*)func->func_scripting->callback); // TODO : make this push a QFunction
                break;
            default:
                sq_pushnull(v);
                break;
            }
            sq_newslot(v, 2, false);
            index++;
        }
    }
    for (int i = 0; i < cls->vars_count; i++)
    {
        QVar* var = &cls->vars[i];
        QType type = var->type;
        QValue val = var->defaultval;
        sq_pushstring(v,var->name, -1);
        switch (type)
        {
        case QType_Int:
            sq_pushinteger(v, val.value_int);
            break;
        case QType_String:
            sq_pushstring(v, val.value_string, -1);
            break;
        case QType_Float:
            sq_pushfloat(v, val.value_float);
            break;
        case QType_Bool:
            sq_pushbool(v, val.value_bool);
            break;
        default:
            sq_pushnull(v);
            break;
        }
        sq_newslot(v, 2, false);
    }
    return 0;
}

SQInteger Squirrel_Object_Get(HSQUIRRELVM v)
{
    SQ_Userdata* usr;
    SQUserPointer tag;
    if (SQ_FAILED(sq_getuserdata(v, 1, (SQUserPointer*)&usr, &tag)))
        return 0; // TODO : error because bad udata
    if (tag != "QSCRIPT_OBJECT") // :letroll:
        return 0;
    const char* name;
    if (SQ_FAILED(sq_getstring(v, 2, &name)))
        return 0;
    QObject* obj = usr->obj;
    
    int index = qscript->GetObjectValueIndex((QScriptObject)obj, name);
    if (index == -1)
    {
        index = qscript->GetObjectMethodIndex((QScriptObject)obj, name);
        if (index == -1)
            return 0;
        QFunction* func = (QFunction*)qscript->GetObjectMethod((QScriptObject)obj, index);
        switch (func->type)
        {
        case QFunction_Module:
            sq_newclosure(v, (SQFUNCTION)func, 0);
            return 1;
        case QFunction_Native:
            sq_newclosure(v, (SQFUNCTION)func, 0);
            return 1;
        case QFunction_Scripting:
            sq_pushobject(v, *(HSQOBJECT*)func->func_scripting->callback); // TODO : make this push a QFunction
            return 1;
        default:
            return 0;
        }
    }
    QValue val = qscript->GetObjectValue((QScriptObject)obj, index);
    QType type = qscript->GetObjectValueType((QScriptObject)obj, index);
    switch (type)
    {
    case QType_Int:
        sq_pushinteger(v, val.value_int);
        return 1;
    case QType_String:
        sq_pushstring(v, val.value_string,-1);
        return 1;
    case QType_Float:
        sq_pushfloat(v, val.value_float);
        return 1;
    case QType_Bool:
        sq_pushbool(v, val.value_bool);
        return 1;
    default:
        sq_pushnull(v);
        return 1;
    }
}

SQInteger Squirrel_Object_Set(HSQUIRRELVM v)
{
    SQ_Userdata* usr;
    SQUserPointer tag;
    if (SQ_FAILED(sq_getuserdata(v, 1, (SQUserPointer*)&usr, &tag)))
        return 0; // TODO : error because bad udata
    if (tag != "QSCRIPT_OBJECT") // :letroll:
        return 0;
    const char* name;
    if (SQ_FAILED(sq_getstring(v, 2, &name)))
        return 0;
    QObject* obj = usr->obj;
    int index = qscript->GetObjectValueIndex((QScriptObject)obj, name);
    if (index == -1)
        return 0;
    QType type = qscript->GetObjectValueType((QScriptObject)obj, index);
    QValue val;
    SQInteger sqi;
    SQBool sqb;
    SQFloat sqf;
    switch (type)
    {
    case QType_Int:
        sq_getinteger(v, 3, &sqi);
        val.value_int = sqi;
        qscript->SetObjectValue((QScriptObject)obj, index, val);
        return 0;
    case QType_String:
        sq_getstring(v, 3, &val.value_string);
        qscript->SetObjectString((QScriptObject)obj, index, val.value_string);
        return 0;
    case QType_Float:
        sq_getfloat(v, 3, &sqf);
        val.value_float = sqf;
        qscript->SetObjectValue((QScriptObject)obj, index, val);
        return 0;
    case QType_Bool:
        sq_getbool(v, 3, &sqb);
        val.value_bool = sqb;
        qscript->SetObjectValue((QScriptObject)obj, index, val);
        return 0;
    default:
        return 0;
    }
}
/*
class notherone extends sourcebox_client.testclass
{
    function IsIt() {
        sourcebox_client.Msg("It is");
    }
    varar = "yey";
}

local cool = notherone()
sourcebox_client.Msg(cool.testvar.tostring())
sourcebox_client.Msg(cool.varar)
cool.IsIt()
*/


QInstance* CSquirrelInterface::ExecuteSquirrel(QMod* mod, const char* code, int size)
{
    HSQUIRRELVM SQ = sq_open(VM_STATIC_STACKSIZE * 2); //i dont think we will get enough stacksize with just 1024
    QInstance* ins = new QInstance();
    ins->env = SQ;
    ins->lang = current_interface;
    sq_setforeignptr(SQ, this);
    sq_enabledebuginfo(SQ, true);

    sq_setdebughook(SQ);
    sq_pushregistrytable(SQ);

        sq_pushstring(SQ, "QSCRIPT_CLASS",-1);
        sq_newtable(SQ);

            sq_pushstring(SQ, "_call", -1);
            sq_newclosure(SQ, Squirrel_Class_Call, 0);

        sq_newslot(SQ, -3, false);

            sq_pushstring(SQ, "_inherited", -1);
            sq_newclosure(SQ, Squirrel_Class_Inherited, 0);

        sq_newslot(SQ, -3, false);

    sq_newslot(SQ, -3, false);

        sq_pushstring(SQ, "QSCRIPT_OBJECT", -1);
        sq_newtable(SQ);

            sq_pushstring(SQ, "_get", -1);
            sq_newclosure(SQ, Squirrel_Object_Get, 0);

        sq_newslot(SQ, -3, false);

            sq_pushstring(SQ, "_set", -1);
            sq_newclosure(SQ, Squirrel_Object_Set, 0);

        sq_newslot(SQ, -3, false);

    sq_newslot(SQ, -3, false);

    sq_pop(SQ, 1);

    sq_pushregistrytable(SQ);
    sq_pushstring(SQ, "QSCRIPT_CLASS", -1);
    sq_get(SQ, -2);
    sq_remove(SQ, -2);

    sq_compilebuffer(SQ, code, size, "squirrel", SQTrue);
    sq_pushroottable(SQ);
    sqstd_seterrorhandlers(SQ);
    for (int i = 0; i < m_modules->Count(); i++)
    {
        QModule* mod = m_modules->Element(i);
        sq_pushstring(SQ, mod->name, -1);
        sq_newtable(SQ);
        for (size_t j = 0; j < mod->functions->Count(); j++)
        {
            QFunction* f = mod->functions->Element(j);
            sq_pushstring(SQ, f->func_module->name, -1);
            sq_newclosure(SQ, (SQFUNCTION)f, 0);
            sq_newslot(SQ, -3, false);
        }
        for (size_t j = 0; j < mod->classes->Count(); j++)
        {
            QClass* cls = mod->classes->Element(j);
            sq_pushstring(SQ,cls->name,-1);
            SQ_Userdata* usr = (SQ_Userdata*)sq_newuserdata(SQ,sizeof(SQ_Userdata));
            usr->cls = cls;
            sq_settypetag(SQ, -1, "QSCRIPT_CLASS");
            sq_push(SQ, -7);
            sq_setdelegate(SQ, -2);
            sq_newslot(SQ, -3,true);
        }
        sq_newslot(SQ, -3, false);
    }

    //base_commands(SQ);
    //sqstd_register_mathlib(SQ);

    if (SQ_FAILED(sq_call(SQ, 1,false,true)))
    {
        sq_getlasterror(SQ);
        if (sq_gettype(SQ, -1) == OT_STRING)
        {
            const SQChar *str;
            sq_getstring(SQ, -1, &str);
            Warning("[Squirrel]: %s\n", str);

            //SQInteger level;
            //sq_getinteger(SQ, -1, &level);
            //write_callstack(SQ, level);

            //sqstd_printcallstack(SQ);
        }
        sq_poptop(SQ);
    }
    sq_settop(SQ, 0);
    return ins;
}


void CSquirrelInterface::ImportModules(CUtlVector<QModule*>* modules)
{
    m_modules = modules;
}

QReturn CSquirrelInterface::CallCallback(QCallback* callback, QArgs* args)
{
    QReturn p;
    p.type = QType_None;
    QValue val;
    val.value_int = 0;
    p.value = val;
    return p;
    /*
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
    */
}
/*
void write_callstack(HSQUIRRELVM SQ, SQInteger level)
{
    SQStackInfos si;
    SQInteger seq = 0;
    const SQChar* name = NULL;

    Warning("[Squirrel Virtual Machine]: Trying to get data from stack\n");

    if (SQ_SUCCEEDED(sq_stackinfos(SQ, level, &si)))
    //if (SQ_SUCCEEDED(sq_stackinfos(SQ, 0, &si)))
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
            
            //sq_pushstring(SQ, name, -1);
            //sq_push(SQ, -2);
            //sq_newslot(SQ, -4, SQFalse);
            //sq_pop(SQ, 1);
            seq++;
        }

        
    }

    Warning("[Squirrel Virtual Machine]: Done\n");
}
*/
#endif
