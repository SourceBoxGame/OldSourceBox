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


struct Lua_Class
{
    bool is_creating;
    union {
        QClass* cls;
        QClassCreator* creator;
    };
};




int Lua_QScript_Index(lua_State* L)
{
    QObject* obj;
    if (!(obj = (QObject*)luaL_checkudata(L, 1, "QSCRIPT_OBJECT")))
        return 0;
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


int Lua_QScript_New_Index(lua_State* L)
{
    QObject* obj;
    if (!(obj = (QObject*)luaL_checkudata(L, 1, "QSCRIPT_OBJECT")))
        return 0;
    const char* name = lua_tostring(L, 2);
    int index = g_pQScript->GetObjectValueIndex((QScriptObject)obj, name);
    if (index == -1)
        return 0;
    QType type = g_pQScript->GetObjectValueType((QScriptObject)obj, index);
    QValue val;
    switch (type)
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
    Lua_Class* luaclass;
    if(!(luaclass = (Lua_Class*)luaL_checkudata(L,1,"QSCRIPT_CLASS")))
        return 0;
    if (luaclass->is_creating)
        return 0;
    QClass* cls = luaclass->cls;
    QObject* obj = (QObject*)lua_newuserdata(L, sizeof(QObject)+cls->vars_count*sizeof(QValue));
    obj->cls = cls;
    g_pQScript->InitalizeObject((QScriptObject)obj);
    luaL_setmetatable(L, "QSCRIPT_OBJECT");
    return 1;
}


int Lua_QScript_Class(lua_State* L)
{
    Lua_Class* parentluaclass;
    QClass* cls = 0;
    if (lua_gettop(L) == 0)
        return 0; // TODO : error here
    if (lua_gettop(L) > 0)
    {
        if (!(parentluaclass = (Lua_Class*)luaL_checkudata(L, 1, "QSCRIPT_CLASS")))
            return 0; // TODO : error here
        if (parentluaclass->is_creating)
            return 0;
        cls = parentluaclass->cls;
    }
    Lua_Class* luaclass = (Lua_Class*)lua_newuserdata(L, sizeof(Lua_Class));
    luaclass->creator = new QClassCreator();
    luaclass->is_creating = true;
    QClassCreator* child = luaclass->creator;
    child->parent = cls;
    //child->name = lua_tolstring(L, 1, 0);
    child->name = 0;
    luaL_setmetatable(L, "QSCRIPT_CLASS_CREATOR");
    return 1;
}

int Lua_QScript_Class_Creator_NewIndex(lua_State* L)
{
    Lua_Class* luaclass;
    if (!(luaclass = (Lua_Class*)luaL_checkudata(L, 1, "QSCRIPT_CLASS_CREATOR")))
        return 0; // TODO : error here
    if (!luaclass->is_creating)
        return 0; // TODO : error here
    QClassCreator* cls = luaclass->creator;
    if (lua_isfunction(L, 3))
    {
        QClassCreatorMethod* meth = (QClassCreatorMethod*)lua_newuserdata(L, sizeof(QClassCreatorMethod));
        lua_pushvalue(L, 2);
        QCallback* callback = new QCallback();
        callback->callback = (void*)luaL_ref(L, LUA_REGISTRYINDEX);
        callback->lang = current_interface;
        callback->env = L;
        meth->scripting_func = callback;
        meth->name = lua_tostring(L, 2);
        meth->is_private = false;
        cls->methods.AddToTail(meth);
        lua_pop(L, 1);
        return 0;
    }
    else
    {
        QVar* var = new QVar();
        var->is_private = false;
        const char* name = lua_tostring(L, 2);
        var->name = new char[strlen(name)];
        strcpy(const_cast<char*>(var->name), name);
        if (lua_isstring(L, -1))
        {
            var->type = QType_String;
            const char* str = lua_tolstring(L, 3, 0);
            var->size = 1<<Qlog2(strlen(str));
            var->defaultval.value_modifiable_string = (char*)malloc(var->size);
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
    Lua_Class* luaclass;
    if (!(luaclass = (Lua_Class*)luaL_checkudata(L, 1, "QSCRIPT_CLASS_CREATOR")))
        return 0; // TODO : error here
    if (!luaclass->is_creating)
        return 0; // TODO : error here
    QClassCreator* cls = luaclass->creator;
    luaclass->is_creating = false;
    luaclass->cls = (QClass*)g_pQScript->FinishClass((QScriptClassCreator)cls);
    luaL_setmetatable(L, "QSCRIPT_CLASS");
    return 0;
}

void CLuaInterface::ExecuteLua(const char* code, int size)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_newmetatable(L, "QSCRIPT_OBJECT");
    lua_pushstring(L, "__index");
    lua_pushcclosure(L, Lua_QScript_Index, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcclosure(L, Lua_QScript_New_Index, 0);
    lua_settable(L, -3);
    lua_pop(L, 1);
    luaL_newmetatable(L, "QSCRIPT_CLASS");
    lua_pop(L, 1);
    luaL_newmetatable(L, "QSCRIPT_CLASS_CREATOR");
    lua_pushstring(L, "__newindex");
    lua_pushcclosure(L, Lua_QScript_Class_Creator_NewIndex, 0);
    lua_settable(L, -3);
    lua_pop(L, 1);
    lua_pushcclosure(L, Lua_QScript_Object, 0);
    lua_setglobal(L, "object");
    lua_pushcclosure(L, Lua_QScript_Class, 0);
    lua_setglobal(L, "class");
    lua_pushcclosure(L, Lua_QScript_Finish, 0);
    lua_setglobal(L, "finish");
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
                Lua_Class* luaclass = (Lua_Class*)lua_newuserdata(L, sizeof(Lua_Class));
                luaclass->cls = cls;
                luaclass->is_creating = false;
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
