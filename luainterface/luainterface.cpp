#ifndef CLUAINTERFACE_H
#define CLUAINTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "appframework/IAppSystem.h"
#include "qscript_language.h"
#include "qscript_defs.h"
#include "qscript_structs.h"
#include "lua.hpp"
#include "utlvector.h"
#include "convar.h"
#include "tier1.h"
#include "filesystem.h"
#include "qscript.h"
#include "UtlStringMap.h"
extern "C" {
#include "ldo.h"
}


IFileSystem* g_pFullFileSystem = 0;
IQScript* g_pQScript = 0;
extern "C"
{
    void* current_interface = 0;
}
class CLuaInterface : public IBaseScriptingInterface
{
public:
    virtual InitReturnVal_t Init();
    virtual void Initialize();
    virtual bool Connect(CreateInterfaceFn factory);
    virtual void Shutdown();
    virtual void ImportModules(CUtlVector<QModule*>* modules);
    virtual QInstance* LoadMod(QMod* mod, const char* path);
    virtual QReturn CallCallback(QCallback* callback, QArgs* args);
    QInstance* ExecuteLua(QMod* mod, const char* code, int size);
private:
    CUtlVector<QModule*>* m_modules;
};

static CLuaInterface s_LuaInterface; // has a name which is QSCRIPT_LANGAUGE_INTERFACE_VERSION
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CLuaInterface, IBaseScriptingInterface, QSCRIPT_LANGAUGE_INTERFACE_VERSION, s_LuaInterface);


int Qlog2(int val)
{
    if (val <= 0)
        return 0;
    int answer = 1;
    val -= 1;
    while (val >>= 1)
        answer++;
    return answer;
}

bool IsValidPath(const char* pszFilename)
{
    if (!pszFilename)
    {
        return false;
    }

    if (Q_strlen(pszFilename) <= 0 ||
        Q_strstr(pszFilename, "\\\\") ||	// to protect network paths
        Q_strstr(pszFilename, ":") || // to protect absolute paths
        Q_strstr(pszFilename, "..") ||   // to protect relative paths
        Q_strstr(pszFilename, "\n") ||   // CFileSystem_Stdio::FS_fopen doesn't allow this
        Q_strstr(pszFilename, "\r"))    // CFileSystem_Stdio::FS_fopen doesn't allow this
    {
        return false;
    }

    return true;
}

InitReturnVal_t CLuaInterface::Init()
{
    return INIT_OK;
}

bool CLuaInterface::Connect(CreateInterfaceFn factory)
{
    ConnectTier1Libraries(&factory, 1);
    ConVar_Register();
    g_pFullFileSystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL);
    g_pQScript = (IQScript*)factory(QSCRIPT_INTERFACE_VERSION, NULL);
    current_interface = this;
    return true;
}






void CLuaInterface::Initialize()
{

}

void CLuaInterface::Shutdown()
{

}






static CUtlBuffer* codebuffer = 0;
QInstance* CLuaInterface::LoadMod(QMod* mod, const char* path)
{
    int len = strlen(path);
    if (!(path[len - 4] == '.' && path[len - 3] == 'l' && path[len - 2] == 'u' && path[len - 1] == 'a'))
        return 0;

    if (codebuffer == 0)
        codebuffer = new CUtlBuffer();

    codebuffer->Clear();

    if (g_pFullFileSystem->ReadFile(path, NULL, *codebuffer))
        return ExecuteLua(mod, (const char*)(codebuffer->Base()), codebuffer->PeekStringLength());
}

void dumpstack(lua_State* L) {
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) {
        Warning("%d\t%s\t", i, luaL_typename(L, i));
        switch (lua_type(L, i)) {
        case LUA_TNUMBER:
            Warning("%g\n", lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            Warning("%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            Warning("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            Warning("%s\n", "nil");
            break;
        default:
            Warning("%p\n", lua_topointer(L, i));
            break;
        }
    }
    Warning("\n");
}


struct Lua_Userdata
{
    union {
        QObject* obj;
        QClass* cls;
        QClassCreator* creator;
        QFunction* func;
    };
};




int Lua_QScript_Index(lua_State* L)
{
    QObject* obj;
    Lua_Userdata* usr;
    if (!(usr = (Lua_Userdata*)luaL_checkudata(L, 1, "QSCRIPT_OBJECT")))
        return 0;
    obj = usr->obj;
    const char* name = lua_tostring(L, 2);
    int index = g_pQScript->GetObjectValueIndex((QScriptObject)obj, name);
    if (index == -1)
    {
        index = g_pQScript->GetObjectMethodIndex((QScriptObject)obj, name);
        if (index == -1)
            return 0;
        QFunction* func = (QFunction*)g_pQScript->GetObjectMethod((QScriptObject)obj, index);
        usr = (Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata));
        usr->func = func;
        luaL_setmetatable(L, "QSCRIPT_FUNCTION");
        return 1;
    }
    QValue val = g_pQScript->GetObjectValue((QScriptObject)obj, index);
    QType type = g_pQScript->GetObjectValueType((QScriptObject)obj, index);
    switch (type)
    {
    case QType_Int:
        lua_pushinteger(L, val.value_int);
        return 1;
    case QType_String:
        lua_pushstring(L, val.value_string);
        return 1;
    case QType_Float:
        lua_pushnumber(L, val.value_float);
        return 1;
    case QType_Bool:
        lua_pushboolean(L, val.value_bool);
        return 1;
    default:
        lua_pushnil(L);
        return 1;
    }
    
}


int Lua_QScript_New_Index(lua_State* L)
{
    QObject* obj;
    Lua_Userdata* usr;
    if (!(usr = (Lua_Userdata*)luaL_checkudata(L, 1, "QSCRIPT_OBJECT")))
        return 0;
    obj = usr->obj;
    const char* name = lua_tostring(L, 2);
    int index = g_pQScript->GetObjectValueIndex((QScriptObject)obj, name);
    if (index == -1)
        return 0;
    QType type = g_pQScript->GetObjectValueType((QScriptObject)obj, index);
    QValue val;
    switch (type) // TODO : error check the type
    {
    case QType_Int:
        val.value_int = lua_tointeger(L, 3);
        g_pQScript->SetObjectValue((QScriptObject)obj, index, val);
        return 0;
    case QType_String:
        g_pQScript->SetObjectString((QScriptObject)obj, index, lua_tostring(L,3));
        return 0;
    case QType_Float:
        val.value_float = lua_tonumber(L, 3);
        g_pQScript->SetObjectValue((QScriptObject)obj, index, val);
        return 0;
    case QType_Bool:
        val.value_bool = lua_toboolean(L, 3);
        g_pQScript->SetObjectValue((QScriptObject)obj, index, val);
        return 0;
    default:
        return 0;
    }
}

int Lua_QScript_Object(lua_State* L)
{
    Lua_Userdata* luaclass;
    if(!(luaclass = (Lua_Userdata*)luaL_checkudata(L,1,"QSCRIPT_CLASS")))
        return 0;
    QClass* cls = luaclass->cls;
    QObject* obj = (QObject*)malloc(sizeof(QObject)+cls->vars_count*sizeof(QValue));
    obj->cls = cls;
    g_pQScript->InitializeObject((QScriptObject)obj);
    ((Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata)))->obj = obj;
    luaL_setmetatable(L, "QSCRIPT_OBJECT");
    return 1;
}

int Lua_QScript_Class(lua_State* L)
{
    Lua_Userdata* parentluaclass;
    QClass* cls = 0;
    if (lua_gettop(L) > 0)
    {
        if (!(parentluaclass = (Lua_Userdata*)luaL_checkudata(L, 1, "QSCRIPT_CLASS")))
            return 0; // TODO : error here
        cls = parentluaclass->cls;
    }
    Lua_Userdata* luaclass = (Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata));
    luaclass->creator = new QClassCreator();
    QClassCreator* child = luaclass->creator;
    child->parent = cls;
    //child->name = lua_tolstring(L, 1, 0);
    child->name = 0;
    luaL_setmetatable(L, "QSCRIPT_CLASS_CREATOR");
    return 1;
}

int Lua_QScript_Class_Creator_NewIndex(lua_State* L)
{
    Lua_Userdata* luaclass;
    if (!(luaclass = (Lua_Userdata*)luaL_checkudata(L, 1, "QSCRIPT_CLASS_CREATOR")))
        return 0; // TODO : error here
    QClassCreator* cls = luaclass->creator;
    if (lua_isfunction(L, 3))
    {
        QClassCreatorMethod* meth = (QClassCreatorMethod*)lua_newuserdata(L, sizeof(QClassCreatorMethod));
        lua_pushvalue(L, 3);
        QCallback* callback = new QCallback();
        callback->callback = (void*)luaL_ref(L, LUA_REGISTRYINDEX);
        callback->lang = current_interface;
        callback->env = L;
        callback->object = 0;
        meth->scripting_func = callback;
        meth->is_scripting = true;
        const char* name = lua_tostring(L, 2);
        meth->name = new char[strlen(name)+1];
        strcpy(const_cast<char*>(meth->name), name);
        meth->is_private = false;
        meth->params = 0;
        meth->params_count = 0;
        cls->methods.AddToTail(meth);
        lua_pop(L, 1);
        return 0;
    }
    else
    {
        QVar* var = new QVar();
        var->is_private = false;
        const char* name = lua_tostring(L, 2);
        var->name = new char[strlen(name)+1];
        strcpy(const_cast<char*>(var->name), name);
        if (lua_isstring(L, -1))
        {
            var->type = QType_String;
            const char* str = lua_tolstring(L, 3, 0);
            var->size = 1<<Qlog2(strlen(str));
            var->defaultval.value_modifiable_string = (char*)malloc(var->size+1);
            strcpy(var->defaultval.value_modifiable_string, str);
        }
        else if (lua_isinteger(L, -1))
        {
            var->type = QType_Int;
            var->defaultval.value_int = lua_tointeger(L, 3);
        }
        else if (lua_isnumber(L, -1))
        {
            var->type = QType_Float;
            var->defaultval.value_float = lua_tonumber(L, 3);
        }
        else if (lua_isboolean(L, -1))
        {
            var->type = QType_Bool;
            var->defaultval.value_bool = (bool)lua_toboolean(L, 3);
        }
        else
        {
            var->type = QType_None;
        }
        cls->vars.AddToTail(var);
        return 0;
    }
}

int Lua_QScript_Finish(lua_State* L)
{
    Lua_Userdata* luaclass;
    if (!(luaclass = (Lua_Userdata*)luaL_checkudata(L, 1, "QSCRIPT_CLASS_CREATOR")))
        return 0; // TODO : error here
    QClassCreator* cls = luaclass->creator;
    luaclass->cls = (QClass*)g_pQScript->FinishClass((QScriptClassCreator)cls);
    luaL_setmetatable(L, "QSCRIPT_CLASS");
    return 0;
}

int Lua_QScript_Export(lua_State* L)
{
    lua_Debug dbg;
    if (lua_getstack(L, 2, &dbg))
        return 0; // TODO : error here, function can only be executed in global context
    QInstance* ins = (QInstance*)lua_touserdata(L,lua_upvalueindex(1));
    QFunction* func;
    Lua_Userdata* usr;
    if (usr = (Lua_Userdata*)luaL_testudata(L, 1, "QSCRIPT_OBJECT"))
    {
        lua_pushglobaltable(L);
        lua_pushnil(L);
        while (lua_next(L,-2) != 0)
        { 
            if (lua_isuserdata(L, -1) && (usr == (Lua_Userdata*)luaL_testudata(L, -1, "QSCRIPT_OBJECT")))
            {
                QExport* exp = new QExport();
                exp->obj = usr->obj;
                exp->type = QExport_Object;
                exp->name = lua_tostring(L, -2);
                ins->exports.AddToTail(exp);
                lua_pop(L, 3);
                return 0;
            }
            lua_pop(L, 1);
        }
        // TODO : error here, must be a global variable
        return 0;
    }
    else if (usr = (Lua_Userdata*)luaL_testudata(L, 1, "QSCRIPT_CLASS"))
    {
        lua_pushglobaltable(L);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            if (lua_isuserdata(L, -1) && (usr == (Lua_Userdata*)luaL_testudata(L, -1, "QSCRIPT_CLASS")))
            {
                QExport* exp = new QExport();
                exp->cls = usr->cls;
                exp->type = QExport_Class;
                exp->name = lua_tostring(L, -2);
                ins->exports.AddToTail(exp);
                lua_pop(L, 3);
                return 0;
            }
            lua_pop(L, 1);
        }
        // TODO : error here, must be a global variable
        return 0;
    }
    else if (lua_isfunction(L,1))
    {
        lua_pushglobaltable(L);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            if (lua_isfunction(L, -1) && lua_rawequal(L, -1, 1))
            {
                func = new QFunction();
                func->always_zero = 0;
                func->type = QFunction_Scripting;
                QCallback* callback = new QCallback();
                lua_pushvalue(L, 1);
                callback->callback = (void*)luaL_ref(L, LUA_REGISTRYINDEX);
                callback->env = L;
                callback->lang = current_interface;
                callback->object = 0;
                func->func_scripting = callback;
                QExport* exp = new QExport();
                exp->func = func;
                exp->type = QExport_Function;
                exp->name = lua_tostring(L, -2);
                ins->exports.AddToTail(exp);
                lua_pop(L, 3);
                return 0;
            }
            lua_pop(L, 1);
        }
        // TODO : error here, must be a global variable
        return 0;
    }
    // TODO : error here, must be a QObject, QClass or QFunction
    return 0;
}



int Lua_QScript_Import(lua_State* L)
{
    lua_Debug dbg;
    if (lua_getstack(L, 2, &dbg))
        return 0; // TODO : error here, function can only be executed in global context
    QMod* mod = (QMod*)lua_touserdata(L, lua_upvalueindex(1));
    const char* path;
    if (!(path = luaL_checkstring(L, 1)))
        return 0; // TODO : error here, string is required
    //import("librarymod/lib.nut")
    //-> library/lib.nut -> GETS PASSED TO LOADFILE
    //-> mods/librarymod/lib.nut -> LOADFILE PASSES THIS TO FILESYSTEM
    //-> sourcebox/mods/librarymod/lib.nut -> FILESYSTEM PASSES THIS TO GLOBAL FILESYSTEM
    //-> C:/sourcebox/sourcebox/mods/librarymod.nut -> GLOBAL FILESYSTEM ACTUALLY READS THIS
    if (!IsValidPath(path))
        return 0; // TODO : error here, nuh uh
    if (!mod->instances.Defined(path))
    {
        mod->instances[path] = 0;
        g_pQScript->LoadFile(path);
    }
    QInstance* inst = mod->instances[path];
    if (!inst)
        return 0; // TODO : error here, most likely a import loop or bad path
    CUtlVector<QExport*>* exports = &mod->instances[path]->exports;
    Lua_Userdata* ud;
    lua_createtable(L, 0, exports->Count());
    for (int i = 0; i < exports->Count(); i++)
    {
        QExport* qexport = exports->Element(i);
        lua_pushstring(L, qexport->name);
        switch (qexport->type)
        {
        case QExport_Object:
            ud = (Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata));
            ud->obj = qexport->obj;
            luaL_setmetatable(L, "QSCRIPT_OBJECT");
            break;
        case QExport_Class:
            ud = (Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata));
            ud->cls = qexport->cls;
            luaL_setmetatable(L, "QSCRIPT_CLASS");
            break;
        case QExport_Function:
            ud = (Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata));
            ud->func = qexport->func;
            luaL_setmetatable(L, "QSCRIPT_FUNCTION");
            break;
        }
        lua_settable(L, -3);
    }
    return 1;
}

int Lua_QScript_Function_Call(lua_State* L)
{
    Lua_Userdata* usr = (Lua_Userdata*)luaL_checkudata(L,1,"QSCRIPT_FUNCTION");
    if (!usr)
        return 0;
    QFunction* func = usr->func;
    if (func->always_zero)
        return 0;
    QArgs* args;
    QReturn ret;
    lua_remove(L, 1);
    int count = lua_gettop(L);
    switch (func->type)
    {
    case QFunction_Module:
        return LuaActualCallback(L, func);
    case QFunction_Native:
        Warning("Calling QFunction_Native is unsuppported in Lua yet (you can add it if you want at line %i in file luainterface.cpp)\n", __LINE__);
        return 0;
        /*args = (QArgs*)malloc(sizeof(QArgs) + lua_gettop(L) * sizeof(QArg));
        args->count = lua_gettop(L);
        ret = func->func_native((QScriptArgs)args);

        free(args);

        switch (ret.type)
        {
        case QType_Int:
            lua_pushinteger(L, ret.value.value_int);
            return 1;
        case QType_Float:
            lua_pushnumber(L, ret.value.value_float);
            return 1;
        case QType_String:
            lua_pushstring(L, ret.value.value_string);
            return 1;
        case QType_Bool:
            lua_pushboolean(L, ret.value.value_bool);
            return 1;
        default:
            return 0;
        }*/
    case QFunction_Scripting:
        args = (QArgs*)malloc(count * sizeof(QArg) + sizeof(QArgs));
        args->count = count;
        args->self = 0;
        for (int i = 0; i < count; i++)
        {
            Lua_Userdata* nusr;
            union QValue val;
            if (lua_isinteger(L, i + 1))
            {
                args->args[i].type = QType_Int;
                val.value_int = lua_tointeger(L, i + 1);
            }
            else if (lua_isnumber(L, i + 1))
            {
                args->args[i].type = QType_Float;
                val.value_float = (float)lua_tonumber(L, i + 1);
            }
            else if (lua_isboolean(L, i + 1))
            {
                args->args[i].type = QType_Bool;
                val.value_bool = lua_toboolean(L, i + 1);
            }
            else if (lua_isstring(L, i + 1))
            {
                args->args[i].type = QType_String;
                val.value_string = lua_tolstring(L, i + 1, 0);
            }
            else if (lua_isfunction(L, i + 1))
            {
                args->args[i].type = QType_Function;
                QCallback* callback = (QCallback*)malloc(sizeof(QCallback));
                lua_pushvalue(L, i + 1);
                callback->callback = (void*)luaL_ref(L, LUA_REGISTRYINDEX);
                callback->lang = current_interface;
                callback->env = L;
                QFunction* func = (QFunction*)malloc(sizeof(QFunction));
                func->always_zero = 0;
                func->func_scripting = callback;
                func->type = QFunction_Scripting;
                val.value_function = (QScriptFunction)func;
            }
            else if (nusr = (Lua_Userdata*)luaL_testudata(L, i + 1, "QSCRIPT_OBJECT"))
            {
                args->args[i].type = QType_Object;
                val.value_object = (QScriptObject)nusr->obj;
            }
            args->args[i].val = val;
            continue;
        }
        QReturn ret = ((IBaseScriptingInterface*)func->func_scripting->lang)->CallCallback(func->func_scripting, args);
        switch (ret.type)
        {
        case QType_Bool:
            lua_pushboolean(L, ret.value.value_bool);
            return 1;
        case QType_Float:
            lua_pushnumber(L, ret.value.value_float);
            return 1;
        case QType_String:
            lua_pushstring(L, ret.value.value_string);
            return 1;
        case QType_Int:
            lua_pushinteger(L, ret.value.value_int);
            return 1;
        case QType_Object:
            ((Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata)))->obj = (QObject*)ret.value.value_object;
            luaL_setmetatable(L, "QSCRIPT_OBJECT");
            return 1;
        default:
            return 0;
        }
    }
    return 0;
}

QInstance* CLuaInterface::ExecuteLua(QMod* mod, const char* code, int size)
{
    lua_State* L = luaL_newstate();
    QInstance* ins = new QInstance();
    ins->env = L;
    ins->lang = (IBaseScriptingInterface*)current_interface;
    luaL_openlibs(L);
    luaL_newmetatable(L, "QSCRIPT_OBJECT");
    lua_pushstring(L, "__index"); // variable = object.some_value --> variable = object:__index("some_value")
    lua_pushcclosure(L, Lua_QScript_Index, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex"); // object.some_value = "foo" --> object:__newindex("some_value","foo")
    lua_pushcclosure(L, Lua_QScript_New_Index, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "nie dla psa");
    lua_settable(L, -3);
    lua_pop(L, 1);
    luaL_newmetatable(L, "QSCRIPT_CLASS");
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "nie dla psa");
    lua_settable(L, -3);
    lua_pop(L, 1);
    luaL_newmetatable(L, "QSCRIPT_FUNCTION");
    lua_pushstring(L, "__call");
    lua_pushcclosure(L, Lua_QScript_Function_Call, 0); // func(...) --> func:__call(...)
    lua_settable(L, -3);
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "nie dla psa");
    lua_settable(L, -3);
    lua_pop(L, 1);
    luaL_newmetatable(L, "QSCRIPT_CLASS_CREATOR");
    lua_pushstring(L, "__newindex");
    lua_pushcclosure(L, Lua_QScript_Class_Creator_NewIndex, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "nie dla psa");
    lua_settable(L, -3);
    lua_pop(L, 1);
    lua_pushcclosure(L, Lua_QScript_Object, 0);
    lua_setglobal(L, "object");
    lua_pushcclosure(L, Lua_QScript_Class, 0);
    lua_setglobal(L, "class");
    lua_pushcclosure(L, Lua_QScript_Finish, 0);
    lua_setglobal(L, "finish");
    lua_pushlightuserdata(L, ins);
    lua_pushcclosure(L, Lua_QScript_Export, 1);
    lua_setglobal(L, "export");
    lua_pushlightuserdata(L, mod);
    lua_pushcclosure(L, Lua_QScript_Import, 1);
    lua_setglobal(L, "import");
    for (int i = 0; i < m_modules->Count(); i++)
    {
        QModule* mod = m_modules->Element(i);
        luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
        lua_getfield(L, -1, mod->name);  /* LOADED[modname] */
        if (!lua_toboolean(L, -1)) {  /* package not already loaded? */
            lua_pop(L, 1);  /* remove field */
            lua_createtable(L, 0, mod->functions->Count());
            luaL_setfuncsqscript(L, mod->functions->Base(), 0, mod->functions->Count());
            for (int j = 0; j < mod->classes->Count(); j++)
            {
                QClass* cls = mod->classes->Element(j);
                Lua_Userdata* luaclass = (Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata));
                luaclass->cls = cls;
                luaL_setmetatable(L, "QSCRIPT_CLASS");
                lua_setfield(L, -2, cls->name);
            }
            lua_pushvalue(L, -1);
            lua_setfield(L, 3, mod->name);  /* LOADED[modname] = module */
        }
        lua_remove(L, 1);  /* remove LOADED table */
        lua_setglobal(L, mod->name);  /* _G[modname] = module */
    }
    luaL_loadstring(L, code);
    if (lua_pcall(L, 0, 0, 0))
    {
        Warning("[Lua]: %s\n", lua_tostring(L, -1));
    }
    return ins;
}


void CLuaInterface::ImportModules(CUtlVector<QModule*>* modules)
{
    m_modules = modules;
}

QReturn CLuaInterface::CallCallback(QCallback* callback, QArgs* args)
{
    lua_State* L = (lua_State*)callback->env;
    int top = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, (int)callback->callback);
    if (args->self)
        lua_pushlightuserdata(L, args->self);
    QReturn ret;
    ret.type = QType_None;
    for (int i = 0; i != args->count; i++)
    {
        QArg arg = args->args[i];  //ahoy its me mr krabs arg arg arg arg arg arg arg arg
        Lua_Userdata* usr;
        switch (arg.type)
        {
        case QType_Int:
            lua_pushinteger(L, arg.val.value_int);
            break;
        case QType_String:
            lua_pushstring(L, arg.val.value_string);
            break;
        case QType_Float:
            lua_pushnumber(L, arg.val.value_float);
            break;
        case QType_Bool:
            lua_pushboolean(L, arg.val.value_bool);
            break;
        case QType_Object:
            usr = (Lua_Userdata*)lua_newuserdata(L, sizeof(Lua_Userdata));
            usr->obj = (QObject*)arg.val.value_object;
            luaL_setmetatable(L, "QSCRIPT_OBJECT");
            break;
        default:
            lua_pushnil(L);
            break;
        }
    }
    if (lua_pcall(L, args->count, 1, 0))
        Warning("[Lua]: %s\n", lua_tostring(L, -1));
    if (lua_isstring(L, -1))
    {
        ret.type = QType_String;
        ret.value.value_string = lua_tolstring(L, -1, 0);
        return ret;
    }
    if (lua_isinteger(L, -1))
    {
        ret.type = QType_String;
        ret.value.value_int = lua_tointeger(L, -1);
        return ret;
    }
    if (lua_isnumber(L, -1))
    {
        ret.type = QType_Float;
        ret.value.value_float = lua_tonumber(L, -1);
        return ret;
    }
    if (lua_isboolean(L, -1))
    {
        ret.type = QType_Bool;
        ret.value.value_bool = (bool)lua_toboolean(L, -1);
        return ret;
    }
    Lua_Userdata* usr;
    if (usr = (Lua_Userdata*)luaL_testudata(L, -1, "QSCRIPT_OBJECT"))
    {
        ret.type = QType_Object;
        ret.value.value_object = (QScriptObject)usr->obj;
        return ret;
    }
    ret.type = QType_None;
    return ret;
}

#endif
