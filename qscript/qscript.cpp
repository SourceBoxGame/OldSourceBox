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
    mod->objs = new CUtlVector<QObject*>();
    m_modules->AddToTail(mod);
    return (QScriptModule)mod;
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

struct QClassCreator
{
    QClass* parent;
    CUtlVector<const char*> method_names;
    CUtlVector<int> method_params_counts;
    CUtlVector<QType*> method_params;
    CUtlVector<QCFunc> methods;
    CUtlVector<const char*> var_names;
    CUtlVector<QType> var_types;
};

QScriptClassCreator CQScript::StartClass(const char* name, QScriptClass parent)
{
    QClassCreator* cr = new QClassCreator();
    cr->parent = (QClass*)parent;
    return cr;
}

void CQScript::AddMethod(QScriptClassCreator creator, const char* name, QType* params, QCFunc func)
{
    QClassCreator* cr = (QClassCreator*)creator;
    cr->method_names.AddToTail(name);
    cr->method_params.AddToTail(params);
    int i = 0;
    for (i = 0; params[i]; i++);
    {}
    cr->method_params_counts.AddToTail(i);
    cr->methods.AddToTail(func);
}

void CQScript::AddVariable(QScriptClassCreator creator, const char* name, QType type)
{
    QClassCreator* cr = (QClassCreator*)creator;
    cr->var_names.AddToTail(name);
    cr->var_types.AddToTail(type);
}

QScriptClass CQScript::FinishClass(QScriptClassCreator creator)
{
    QClassCreator* cr = (QClassCreator*)creator;
    QClass* cl = new QClass();
    if (cr->parent)
    {
        cl->methods_count = cr->parent->methods_count + cr->methods.Count();
        cl->sigs_count = cr->parent->sigs_count + 1;
        cl->vars_count = cr->parent->vars_count + cr->var_names.Count();
        cl->methods = (QObjectFunction*)malloc(cl->methods_count * sizeof(QObjectFunction));
        cl->sigs = (QInterface**)malloc(cl->sigs_count * sizeof(QInterface*));
        cl->vars_names = (const char**)malloc(cl->vars_count * sizeof(const char*));
        cl->vars_types = (QType*)malloc(cl->vars_count * sizeof(QType));
        memcpy(cl->methods, cr->parent->methods, cr->parent->methods_count * sizeof(QObjectFunction));
        memcpy(cl->sigs, cr->parent->sigs, cr->parent->sigs_count * sizeof(QInterface*));
        memcpy(cl->vars_names, cr->parent->vars_names, cr->parent->vars_count * sizeof(const char*));
        memcpy(cl->vars_types, cr->parent->vars_types, cr->parent->vars_count * sizeof(QType));
    }
    QInterface* in = new QInterface();
    in->args = (QParams*)malloc(cr->methods.Count() * sizeof(QParams*));
    for (int i = 0; i != cr->methods.Count(); i++)
    {
        in->args[i].count = cr->method_params_counts[i];
        in->args[i].types = (QType*)malloc(in->args[i].count * sizeof(QType));
        memcpy(in->args[i].types, cr->method_params.Base(), in->args[i].count * sizeof(QType));
    }
    in->names = (const char**)malloc(cr->methods.Count() * sizeof(const char*));
    memcpy(in->names, cr->method_names.Base(), cr->methods.Count() * sizeof(const char*));
    if (cr->parent)
    {
        for (int i = 0; i != cr->methods.Count();i++)
        {
            cl->methods[cr->parent->methods_count + i].is_scripting = false;
            cl->methods[cr->parent->methods_count + i].func_native = cr->methods[i];
        }
        cl->sigs[cr->parent->sigs_count] = in;
        memcpy(&cl->vars_names[cr->parent->vars_count], cr->var_names.Base(), cr->var_names.Count() * sizeof(const char*));
        memcpy(&cl->vars_types[cr->parent->vars_count], cr->var_types.Base(), cr->var_types.Count() * sizeof(QType));
    }
    else
    {
        cl->methods_count = cr->methods.Count();
        cl->methods = (QObjectFunction*)malloc(cr->methods.Count() + sizeof(QObjectFunction));
        for (int i = 0; i != cr->methods.Count();i++)
        {
            cl->methods[i].is_scripting = false;
            cl->methods[i].func_native = cr->methods[i];
        }
        cl->sigs_count = 1;
        cl->sigs = (QInterface**)malloc(sizeof(QInterface*));
        cl->sigs[0] = in;
        cl->vars_count = cr->var_names.Count();
        cl->vars_names = (const char**)malloc(cl->vars_count * sizeof(const char*));
        cl->vars_types = (QType*)malloc(cl->vars_count * sizeof(QType));
        memcpy(cl->vars_names, cr->var_names.Base(), cr->var_names.Count() * sizeof(const char*));
        memcpy(cl->vars_types, cr->var_types.Base(), cr->var_types.Count() * sizeof(QType));
    }
    return (QScriptClass)cl;
}
