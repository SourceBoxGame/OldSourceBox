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
#include "lua.hpp"
#include "Windows.h"
#include "utlvector.h"
#include "convar.h"
#include "tier1.h"
#include "filesystem.h"
#include "qscript.h"

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
    virtual void LoadMod(const char* path);
    virtual QReturn CallCallback(QCallback* callback, QArgs* args);
    void ExecuteLua(const char* code, int size);
private:
    CUtlVector<QModule*>* m_modules;
};

static CLuaInterface s_LuaInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CLuaInterface, IBaseScriptingInterface, QSCRIPT_LANGAUGE_INTERFACE_VERSION, s_LuaInterface);

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
void CLuaInterface::LoadMod(const char* path)
{
    int len = strlen(path);
    if (!(path[len - 4] == '.' && path[len - 3] == 'l' && path[len - 2] == 'u' && path[len - 1] == 'a'))
        return;

    if (codebuffer == 0)
        codebuffer = new CUtlBuffer();

    codebuffer->Clear();

    if (g_pFullFileSystem->ReadFile(path, NULL, *codebuffer))
        ExecuteLua((const char*)(codebuffer->Base()),codebuffer->PeekStringLength());
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

int Lua_QScript_Index(lua_State* L)
{
    QObject* obj = (QObject*)lua_touserdata(L, 1);
    const char* name = lua_tostring(L, 2);
    int index = g_pQScript->GetObjectValueIndex((QScriptObject)obj, name);
    if (index == -1)
        return 0;
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

int Lua_QScript_Object(lua_State* L)
{
    QClass* cls = 0;
    if (!lua_islightuserdata(L, 1))
    {
        if(!(cls = (QClass*)luaL_checkudata(L,1,"QSCRIPT_CLASS")))
            return 0;
    }
    else
        cls = (QClass*)lua_touserdata(L, 1);
    QObject* obj = (QObject*)lua_newuserdata(L, sizeof(QObject)+cls->vars_count*sizeof(QValue));
    obj->cls = cls;
    g_pQScript->InitalizeObject((QScriptObject)obj);
    luaL_setmetatable(L, "QSCRIPT_OBJECT");
    return 1;
}
/*
MyEnt = class(sourcebox_server.ents.CBaseEntity)
MyEnt.classname = "your_mom"
...
sourcebox_server.RegisterEntity(MyEnt)


*/

int Lua_QScript_Class(lua_State* L)
{
    QClass* cls = 0;
    if (lua_gettop(L) != 0)
    {
        if (!lua_islightuserdata(L, 1))
        {
            if (!(cls = (QClass*)luaL_checkudata(L, 1, "QSCRIPT_CLASS")))
                return 0;
        }
        else
            cls = (QClass*)lua_touserdata(L, 1);
    }
    QClass* child = new QClass();
    child->
}

void CLuaInterface::ExecuteLua(const char* code, int size)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_newmetatable(L, "QSCRIPT_OBJECT");
    lua_pushstring(L, "__index");
    lua_pushcclosure(L, Lua_QScript_Index, 0);
    lua_settable(L, -3);
    lua_pop(L, 1);
    lua_pushcclosure(L, Lua_QScript_Object, 0);
    lua_setglobal(L, "object");
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
                lua_pushlightuserdata(L, cls);
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
        switch (arg.type)
        {
        case 'i':
            lua_pushinteger(L, arg.val.value_int);
            break;
        case 's':
            lua_pushstring(L, arg.val.value_string);
            break;
        case 'f':
            lua_pushnumber(L, arg.val.value_float);
            break;
        case 'b':
            lua_pushboolean(L, arg.val.value_bool);
            break;
        default:
            lua_settop(L, top);
            return ret;
        }
    }
    if (lua_pcall(L, args->count, 1, 0))
        Warning("[Lua]: %s\n", lua_tostring(L, -1));
    if (lua_isstring(L, -1))
    {
        ret.type = QType_String;
        ret.value.value_string = lua_tolstring(L, 3, 0);
        return ret;
    }
    if (lua_isinteger(L, -1))
    {
        ret.type = QType_String;
        ret.value.value_int = lua_tointeger(L, 3);
        return ret;
    }
    if (lua_isnumber(L, -1))
    {
        ret.type = QType_Float;
        ret.value.value_float = lua_tonumber(L, 3);
        return ret;
    }
    if (lua_isboolean(L, -1))
    {
        ret.type = QType_Bool;
        ret.value.value_bool = (bool)lua_toboolean(L, 3);
        return ret;
    }
    ret.type = QType_None;
    return ret;
}

#endif
