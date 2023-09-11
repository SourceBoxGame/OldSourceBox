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

IFileSystem* g_pFullFileSystem = 0;
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
    virtual void CallCallback(QCallback* callback, QArgs* args);
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

void Lua_Push_QObject(lua_State* L, QObject* obj)
{
    switch (obj->type)
    {
    case QType_Int:
        lua_pushinteger(L, obj->value_int);
        return;
    case QType_String:
        lua_pushstring(L, obj->value_string);
        return;
    case QType_Float:
        lua_pushnumber(L, obj->value_float);
        return;
    case QType_Bool:
        lua_pushboolean(L, obj->value_bool);
        return;
    default:
        lua_pushnil(L);
        return;
    }
}

int Lua_QScript_Index(lua_State* L)
{
    QObject* obj = (QObject*)lua_touserdata(L, 1);
    QTree* tree = obj->value_tree;
    const char* name = lua_tostring(L, 2);
    for (int i = 0; i < tree->immutable_objs_count; i++)
    {
        QObject* child = tree->immutable_objs[i];
        if (strcmp(child->name, name) != 0)
            continue;
        Lua_Push_QObject(L,child);
        return 1;
    }
    for (int i = 0; i < tree->immutable_methods_count; i++)
    {
        QFunction* child = tree->immutable_methods[i];
        if (strcmp(child->name, name) != 0)
            continue;
        lua_pushcfunction(L, (lua_CFunction)child);
        return 1;
    }
    for (int i = 0; i < tree->obj_count; i++)
    {
        QObject* child = tree->objs[i];
        if (strcmp(child->name, name) != 0)
            continue;
        Lua_Push_QObject(L, child);
        return 1;
    }
    for (int i = 0; i < tree->method_count; i++)
    {
        QFunction* child = tree->methods[i];
        if (strcmp(child->name, name) != 0)
            continue;
        lua_pushcfunction(L, (lua_CFunction)child);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

void Lua_Set_QObject(lua_State* L, QObject* obj)
{
    switch (obj->type)
    {
    case QType_String:
        if (lua_isstring(L, 3))
            obj->value_string = lua_tolstring(L, 3, 0);
        return;
    case QType_Int:
        if (lua_isinteger(L, 3))
            obj->value_int = lua_tointeger(L, 3);
        return;
    case QType_Float:
        if (lua_isnumber(L, 3))
            obj->value_float = lua_tonumber(L, 3);
        return;
    case QType_Bool:
        if (lua_isboolean(L, 3))
            obj->value_bool = (bool)lua_toboolean(L, 3);
        return;
    default:
        return;
    }
}

int Lua_QScript_New_Index(lua_State* L)
{
    QObject* obj = (QObject*)lua_touserdata(L, 1);
    QTree* tree = obj->value_tree;
    const char* name = lua_tostring(L, 2);
    for (int i = 0; i < tree->obj_count; i++)
    {
        QObject* child = tree->objs[i];
        if (strcmp(child->name, name) != 0)
            continue;
        Lua_Set_QObject(L, child);
        return 0;
    }
    if (!lua_isfunction(L, 3))
        return 0;
    for (int i = 0; i < tree->method_count; i++)
    {
        QObjectFunction* child = tree->methods[i];
        if (strcmp(child->name, name) != 0)
            continue;
        if (child->is_scripting)
            free(child->scripting_func);
        child->is_scripting = true;
        child->scripting_func = new QCallback();
        lua_pushvalue(L, 3);
        child->scripting_func->callback = (void*)luaL_ref(L, LUA_REGISTRYINDEX);
        child->scripting_func->lang = current_interface;
        child->scripting_func->env = L;
        return 0;
    }
    return 0;
}

QFunction Lua_QScript_Index_Func = {
    '\x01',
    "",
    "",
    (QCFunc)Lua_QScript_Index,
    0
};

QFunction Lua_QScript_New_Index_Func = {
    '\x01',
    "",
    "",
    (QCFunc)Lua_QScript_New_Index,
    0
};

void CLuaInterface::ExecuteLua(const char* code, int size)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    for (int i = 0; i < m_modules->Count(); i++)
    {
        QModule* mod = m_modules->Element(i);
        luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
        lua_getfield(L, -1, mod->name);  /* LOADED[modname] */
        if (!lua_toboolean(L, -1)) {  /* package not already loaded? */
            lua_pop(L, 1);  /* remove field */
            lua_createtable(L, 0, mod->functions->Count());
            luaL_setfuncsqscript(L, mod->functions->Base(), 0, mod->functions->Count());
            for (int j = 0; j < mod->objs->Count(); j++)
            {
                QObject* obj = mod->objs->Element(j);
                if (obj->type != QType_Object)
                    continue;
                int len = strlen(mod->name) + strlen(obj->name) + 2;
                char* name = (char*)malloc(len *sizeof(char));
                snprintf(name, len, "%s.%s",mod->name,obj->name);
                luaL_newmetatable(L, name);
                lua_pushstring(L,"__index");
                lua_pushcfunction(L, (lua_CFunction)&Lua_QScript_Index_Func);
                lua_settable(L, -3);
                lua_pushstring(L, "__newindex");
                lua_pushcfunction(L, (lua_CFunction)&Lua_QScript_New_Index_Func);
                lua_settable(L, -3);
                

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

void CLuaInterface::CallCallback(QCallback* callback, QArgs* args)
{
    lua_State* L = (lua_State*)callback->env;
    int top = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, (int)callback->callback);
    for (int i = 0; i != args->count; i++)
    {
        switch (args->types[i])
        {
        case 'i':
            lua_pushinteger(L, (int)args->args[i]);
            break;
        case 's':
            lua_pushstring(L, (const char*)args->args[i]);
            break;
        case 'f':
            lua_pushnumber(L, *(float*)&args->args[i]);
            break;
        case 'b':
            lua_pushboolean(L, (bool)args->args[i]);
            break;
        default:
            lua_settop(L, top);
            return;
        }
    }
    if (lua_pcall(L, args->count, 0, 0))
        Warning("[Lua]: %s\n", lua_tostring(L, -1));
}

#endif
