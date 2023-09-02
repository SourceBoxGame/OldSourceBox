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

class CLuaInterface : public IBaseScriptingInterface
{
public:
    virtual InitReturnVal_t Init();
    virtual void Initialize();
    virtual bool Connect(CreateInterfaceFn factory);
    virtual void Shutdown();
    virtual void ImportModules(CUtlVector<QModule*>* modules);
    virtual void LoadMod(const char* path);
    void ExecuteLua(const char* code, int size);
private:
    CUtlVector<QModule*>* m_modules;
};

static CLuaInterface s_LuaInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CLuaPythonInterface, IBaseScriptingInterface, QSCRIPT_LANGAUGE_INTERFACE_VERSION, s_LuaInterface);

InitReturnVal_t CLuaInterface::Init()
{
    return INIT_OK;
}

bool CLuaInterface::Connect(CreateInterfaceFn factory)
{
    ConnectTier1Libraries(&factory, 1);
    ConVar_Register();
    g_pFullFileSystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL);
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

void CLuaInterface::ExecuteLua(const char* code, int size)
{
    lua_State* L = luaL_newstate();
    dumpstack(L);
    for (int i = 0; i < m_modules->Count(); i++)
    {
        QModule* mod = m_modules->Element(i);
        luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
        dumpstack(L);
        lua_getfield(L, -1, mod->name);  /* LOADED[modname] */
        dumpstack(L);
        if (!lua_toboolean(L, -1)) {  /* package not already loaded? */
            lua_pop(L, 1);  /* remove field */
            dumpstack(L);
            lua_createtable(L, 0, mod->functions->Count());
            dumpstack(L);
            luaL_setfuncs(L, mod->functions->Base(), 0, mod->functions->Count());
            dumpstack(L);
            lua_pushvalue(L, -1);
            dumpstack(L);
            lua_setfield(L, 3, mod->name);  /* LOADED[modname] = module */
        }
        dumpstack(L);
        lua_remove(L, 1);  /* remove LOADED table */
        dumpstack(L);
        lua_setglobal(L, mod->name);  /* _G[modname] = module */
        dumpstack(L);
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

#endif