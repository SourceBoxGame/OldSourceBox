#define PY_SSIZE_T_CLEAN
#include "cqscript.h"
#include "filesystem.h"

static CQScript s_QScript;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CQScript, IQScript, QSCRIPT_INTERFACE_VERSION, s_QScript);


static const QReturn QNone = {
    QType_None,0
};

IFileSystem* g_pFullFileSystem = 0;


// singleton accessor
CQScript& QScript()
{
    return s_QScript;
}

CQScript::CQScript() {
    
}

void CQScript::Initialize()
{
    for (int i = 0; i != m_interfaces->Count(); i++)
    {
        m_interfaces->Element(i)->ImportModules(m_modules);
        m_interfaces->Element(i)->Initialize();
    }
}

void CQScript::AddScriptingInterface(const char* name, CreateInterfaceFn factory)
{
    CSysModule* scriptingModule = Sys_LoadModule(name);
    if (scriptingModule)
    {
        CreateInterfaceFn scriptingFactory = Sys_GetFactory(scriptingModule);
        if (scriptingFactory)
        {
            IBaseScriptingInterface* scriptingInterface = (IBaseScriptingInterface*)scriptingFactory(QSCRIPT_LANGAUGE_INTERFACE_VERSION, NULL);
            m_interfaces->AddToTail(scriptingInterface);
            scriptingInterface->Connect(factory);
        }
    }
}

bool CQScript::Connect(CreateInterfaceFn factory)
{
    m_interfaces = new CUtlVector<IBaseScriptingInterface*>();
    m_modules = new CUtlVector<QModule*>();
    g_pFullFileSystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL);
    AddScriptingInterface("luainterface" DLL_EXT_STRING, factory);
    AddScriptingInterface("squirrelinterface" DLL_EXT_STRING, factory);
    AddScriptingInterface("pythoninterface" DLL_EXT_STRING, factory);
    return true;
}

InitReturnVal_t CQScript::Init()
{
    return INIT_OK;
}

void CQScript::Shutdown()
{
    
}

QScriptFunction CQScript::CreateModuleFunction(QScriptModule module, const char* name, const char* args, QCFunc funcptr)
{
    QModule* mod = (QModule*)module;
    QFunction* func = new QFunction();
    func->name = name;
    func->args = args;
    func->func = funcptr;
    mod->functions->AddToTail(func);
    return (QScriptFunction)func;
}

void CQScript::CreateModuleObject(QScriptModule module, QScriptObject object)
{
    QModule* mod = (QModule*)module;
    QObject* obj = (QObject*)object;
    mod->objs->AddToTail(obj);
}


QScriptModule CQScript::CreateModule(const char* name)
{
    QModule* mod = new QModule();
    mod->name = name;
    mod->functions = new CUtlVector<QFunction*>();
    m_modules->AddToTail(mod);
    return (QScriptModule)mod;
}
const char* CQScript::GetArgString(QScriptArgs args, int argnum)
{
    return ((const char**)(((QArgs*)args)->args))[argnum];
}

char* CQScript::GetArgPermaString(QScriptArgs args, int argnum)
{
    const char* name = GetArgString(args, argnum);
    char* permaname = (char*)malloc(strlen(name) + 1);
    strcpy(permaname, name);
    return permaname;
}

int CQScript::GetArgInt(QScriptArgs args, int argnum)
{
    return ((int*)(((QArgs*)args)->args))[argnum];
}

float CQScript::GetArgFloat(QScriptArgs args, int argnum)
{
    return ((float*)(((QArgs*)args)->args))[argnum];
}

bool CQScript::GetArgBool(QScriptArgs args, int argnum)
{
    return ((bool*)(((QArgs*)args)->args))[argnum];
}

QScriptCallback CQScript::GetArgCallback(QScriptArgs args, int argnum)
{
    return (QScriptCallback)((QCallback**)(((QArgs*)args)->args))[argnum];
}

void CQScript::LoadFilesInDirectory(const char* folder, const char* filename)
{
    FileFindHandle_t findHandle;
    char searchPath[MAX_PATH];
    strcpy(searchPath, "mods/");
    strncat(searchPath, folder,MAX_PATH);
    strncat(searchPath, "/*",MAX_PATH);
    const char* pszFileName = g_pFullFileSystem->FindFirst(searchPath, &findHandle);
    char pszFileNameNoExt[MAX_PATH];
    while (pszFileName)
    {
        if (pszFileName[0] == '.')
        {
            pszFileName = g_pFullFileSystem->FindNext(findHandle);
            continue;
        }
        if (g_pFullFileSystem->FindIsDirectory(findHandle))
        {
            pszFileName = g_pFullFileSystem->FindNext(findHandle);
            continue;
        }
        V_StripExtension(pszFileName, pszFileNameNoExt, MAX_PATH);
        if (V_strcmp(filename, pszFileNameNoExt) == 0)
        {
            char pFilePath[MAX_PATH];
            strcpy(pFilePath, "mods/");
            strncat(pFilePath, folder, MAX_PATH);
            V_AppendSlash(pFilePath, MAX_PATH);
            strncat(pFilePath, pszFileName, MAX_PATH);
            for (int i = 0; i != m_interfaces->Count(); i++)
            {
                m_interfaces->Element(i)->LoadMod(pFilePath);
            }
        }
        pszFileName = g_pFullFileSystem->FindNext(findHandle);
    }
}

void CQScript::LoadMods(const char* filename)
{
    FileFindHandle_t findHandle;
    const char* pszFileName = g_pFullFileSystem->FindFirst("mods/*", &findHandle);
    char pszFileNameNoExt[MAX_PATH];
    while (pszFileName)
    {
        if (pszFileName[0] == '.')
        {
            pszFileName = g_pFullFileSystem->FindNext(findHandle);
            continue;
        }
        if (g_pFullFileSystem->FindIsDirectory(findHandle))
        {
            LoadFilesInDirectory(pszFileName, filename);
            pszFileName = g_pFullFileSystem->FindNext(findHandle);
            continue;
        }
        pszFileName = g_pFullFileSystem->FindNext(findHandle);
    }
}


void CQScript::LoadModsInDirectory(const char* folder, const char* filename)
{
    FileFindHandle_t findHandle;
    char path[MAX_PATH];
    const char* pszFileName = g_pFullFileSystem->FindFirst("mods/*", &findHandle);
    char pszFileNameNoExt[MAX_PATH];
    while (pszFileName)
    {
        if (pszFileName[0] == '.')
        {
            pszFileName = g_pFullFileSystem->FindNext(findHandle);
            continue;
        }
        if (g_pFullFileSystem->FindIsDirectory(findHandle))
        {
            snprintf(path, MAX_PATH, "mods/%s/%s",pszFileName,folder);
            if (g_pFullFileSystem->IsDirectory(path))
            {
                snprintf(path, MAX_PATH, "%s/%s", pszFileName, folder);
                LoadFilesInDirectory(path, filename);
            }
            pszFileName = g_pFullFileSystem->FindNext(findHandle);
            continue;
        }
        pszFileName = g_pFullFileSystem->FindNext(findHandle);
    }
}

void CQScript::CallCallback(QScriptCallback callback, QScriptArgs args)
{
    QCallback* c = (QCallback*)callback;
    QArgs* a = (QArgs*)args;
    ((IBaseScriptingInterface*)c->lang)->CallCallback(c, a);
}

QScriptArgs CQScript::CreateArgs(const char* types, ...)
{
    va_list va;
    QArgs* args = new QArgs();
    va_start(va, types);
    int len = strlen(types);
    args->args = (void**)malloc(len * sizeof(void*));
    args->count = len;
    args->types = types;
    for (int i = 0; i < len; i++)
    {
        switch (types[i])
        {
        case 's':
            args->args[i] = va_arg(va, void*);
            break;
        case 'f':
        case 'i':
            args->args[i] = (void*)va_arg(va, int);
            break;
        case 'b':
            args->args[i] = (void*)va_arg(va, bool);
            break;
        case 'o':
            args->args[i] = (void*)va_arg(va, QObject*);
            break;
        default:
            Warning("Tried to create invalid arg type '%c' at %i.\n", types[i], i);
            free(args->args);
            free(args);
            return NULL;
        }
    }
    return (QScriptArgs)args;

}

void CQScript::FreeArgs(QScriptArgs a)
{
    QArgs* args = (QArgs*)a;
    free(args->args);
    free(args);
}

QScriptReturn CQScript::RetNone()
{
    return (QScriptReturn)&QNone;
}

QScriptReturn CQScript::RetInt(int value)
{
    QReturn* ret = new QReturn();
    ret->type = QType_Int;
    ret->value = (void*)value;
    return (QScriptReturn)ret;
}
QScriptReturn CQScript::RetFloat(float value)
{
    QReturn* ret = new QReturn();
    ret->type = QType_Float;
    ret->value = *(void**)&value;
    return (QScriptReturn)ret;
}
QScriptReturn CQScript::RetBool(bool value)
{
    QReturn* ret = new QReturn();
    ret->type = QType_Bool;
    ret->value = (void*)value;
    return (QScriptReturn)ret;
}
QScriptReturn CQScript::RetString(const char* value)
{
    QReturn* ret = new QReturn();
    ret->type = QType_String;
    ret->value = (void*)value;
    return (QScriptReturn)ret;
}

QScriptObject CQScript::GetObjectElementByName(QScriptObject object, const char* name)
{
    QObject* obj = (QObject*)object;
    if (obj->type != QType_Object)
        return NULL;
    for (int i = 0; i != obj->count; i++)
    {
        if (V_strcmp(obj->objs[i]->name, name) == 0)
            return obj->objs[i];
    }
    return NULL;
}

int CQScript::GetObjectInt(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    if (obj->type != QType_Int)
        return NULL;
    return obj->value_int;
}

float CQScript::GetObjectFloat(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    if (obj->type != QType_Float)
        return NULL;
    return obj->value_float;
}

const char* CQScript::GetObjectString(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    if (obj->type != QType_String)
        return NULL;
    return obj->value_string;
}

bool CQScript::GetObjectBool(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    if (obj->type != QType_Bool)
        return NULL;
    return obj->value_bool;
}

void* CQScript::GetObjectVoid(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    return obj->value_raw;
}

QType CQScript::GetObjectType(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    return obj->type;
}

void CQScript::SetObjectValue(QScriptObject object, QType type, void* val)
{
    QObject* obj = (QObject*)object;
    obj->type = type;
    obj->value_raw = val;
}

void CQScript::SetObjectInt(QScriptObject object, int val)
{
    QObject* obj = (QObject*)object;
    obj->type = QType_Int;
    obj->value_int = val;
}

void CQScript::SetObjectFloat(QScriptObject object, float val)
{
    QObject* obj = (QObject*)object;
    obj->type = QType_Float;
    obj->value_float = val;
}
void CQScript::SetObjectString(QScriptObject object, const char* val)
{
    QObject* obj = (QObject*)object;
    obj->type = QType_String;
    obj->value_string = val;
}
void CQScript::SetObjectBool(QScriptObject object, bool val)
{
    QObject* obj = (QObject*)object;
    obj->type = QType_Bool;
    obj->value_bool = val;
}

void CQScript::SetObjectFunction(QScriptObject object, QScriptFunction func)
{
    QObject* obj = (QObject*)object;
    obj->type = QType_Function;
    obj->value_function = (QFunction*)func;
}