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
    strtotype['s'] = QType_String;
    strtotype['i'] = QType_Int;
    strtotype['f'] = QType_Float;
    strtotype['o'] = QType_Object;
    strtotype['b'] = QType_Bool;
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

QScriptFunction CQScript::CreateModuleFunction(QScriptModule module, const char* name, QType* types, QType returntype, QCFunc funcptr)
{
    QModule* mod = (QModule*)module;
    QModuleFunction* func = new QModuleFunction();
    int i = 0;
    for (; types[i]; i++)
    { }
    func->params.count = i;
    func->params.types = (QType*)malloc(i * sizeof(QType));
    func->type = returntype;
    memcpy(func->params.types, types, i * sizeof(QType));
    func->func = funcptr;
    QFunction* final_func = new QFunction();
    final_func->func_module = func;
    final_func->type = QFunction_Module;
    mod->functions->AddToTail(final_func);
    return (QScriptFunction)func;
}

void CQScript::CreateModuleClass(QScriptModule module, QScriptClass object)
{
    QModule* mod = (QModule*)module;
    QClass* obj = (QClass*)object;
    mod->classes->AddToTail(obj);
}


QScriptModule CQScript::CreateModule(const char* name, QModuleDefFunc* funcs)
{
    QModule* mod = new QModule();
    mod->name = name;
    mod->functions = new CUtlVector<QFunction*>();
    mod->classes = new CUtlVector<QClass*>();
    for (int i = 0; funcs[i].func; i++)
    {
        QModuleFunction* func = new QModuleFunction();
        int types = strlen(funcs[i].types);
        func->params.count = types;
        func->params.types = (QType*)malloc(types * sizeof(QType));
        func->type = funcs[i].ret;
        for (int j = 0; j < types; j++)
        {
            func->params.types[j] = strtotype[funcs[i].types[j]];
        }
        func->name = funcs[i].name;
        func->func = funcs[i].func;
        QFunction* final_func = new QFunction();
        final_func->always_zero = 0;
        final_func->func_module = func;
        final_func->type = QFunction_Module;
        mod->functions->AddToTail(final_func);
    }
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
    const char* name;
    CUtlVector<const char*> method_names;
    CUtlVector<int> method_params_counts;
    CUtlVector<QType*> method_params;
    CUtlVector<QCFunc> methods;
    CUtlVector<const char*> var_names;
    CUtlVector<QType> var_types;
    CUtlVector<int> var_sizes;
};

QScriptClassCreator CQScript::StartClass(const char* name, QScriptClass parent)
{
    QClassCreator* cr = new QClassCreator();
    cr->name = name;
    cr->parent = (QClass*)parent;
    return (QScriptClassCreator)cr;
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
    cr->var_sizes.AddToTail(0);
}

void CQScript::AddString(QScriptClassCreator creator, const char* name, int size)
{
    QClassCreator* cr = (QClassCreator*)creator;
    cr->var_names.AddToTail(name);
    cr->var_types.AddToTail(QType_String);
    cr->var_sizes.AddToTail(size);
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
        cl->methods = (QFunction*)malloc(cl->methods_count * sizeof(QFunction));
        cl->sigs = (QInterface**)malloc(cl->sigs_count * sizeof(QInterface*));
        cl->vars_names = (const char**)malloc(cl->vars_count * sizeof(const char*));
        cl->vars_types = (QType*)malloc(cl->vars_count * sizeof(QType));
        cl->vars_sizes = (int*)malloc(cl->vars_count * sizeof(int));
        memcpy(cl->methods, cr->parent->methods, cr->parent->methods_count * sizeof(QFunction));
        memcpy(cl->sigs, cr->parent->sigs, cr->parent->sigs_count * sizeof(QInterface*));
        memcpy(cl->vars_names, cr->parent->vars_names, cr->parent->vars_count * sizeof(const char*));
        memcpy(cl->vars_types, cr->parent->vars_types, cr->parent->vars_count * sizeof(QType));
        memcpy(cl->vars_sizes, cr->parent->vars_sizes, cr->parent->vars_count * sizeof(int));
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
            cl->methods[cr->parent->methods_count + i].always_zero = 0;
            cl->methods[cr->parent->methods_count + i].type = QFunction_Native;
            cl->methods[cr->parent->methods_count + i].func_native = cr->methods[i];
        }
        cl->sigs[cr->parent->sigs_count] = in;
        memcpy(&cl->vars_names[cr->parent->vars_count], cr->var_names.Base(), cr->var_names.Count() * sizeof(const char*));
        memcpy(&cl->vars_types[cr->parent->vars_count], cr->var_types.Base(), cr->var_types.Count() * sizeof(QType));
        memcpy(&cl->vars_sizes[cr->parent->vars_count], cr->var_sizes.Base(), cr->var_sizes.Count() * sizeof(int));
    }
    else
    {
        cl->methods_count = cr->methods.Count();
        cl->methods = (QFunction*)malloc(cr->methods.Count() + sizeof(QFunction));
        for (int i = 0; i != cr->methods.Count();i++)
        {
            cl->methods[i].type = QFunction_Native;
            cl->methods[i].always_zero = 0;
            cl->methods[i].func_native = cr->methods[i];
        }
        cl->sigs_count = 1;
        cl->sigs = (QInterface**)malloc(sizeof(QInterface*));
        cl->sigs[0] = in;
        cl->vars_count = cr->var_names.Count();
        cl->vars_names = (const char**)malloc(cl->vars_count * sizeof(const char*));
        cl->vars_types = (QType*)malloc(cl->vars_count * sizeof(QType));
        cl->vars_sizes = (int*)malloc(cl->vars_count * sizeof(int));
        memcpy(cl->vars_names, cr->var_names.Base(), cr->var_names.Count() * sizeof(const char*));
        memcpy(cl->vars_types, cr->var_types.Base(), cr->var_types.Count() * sizeof(QType));
        memcpy(cl->vars_sizes, cr->var_sizes.Base(), cr->var_sizes.Count() * sizeof(int));
    }
    cl->name = cr->name;
    return (QScriptClass)cl;
}

QScriptObject CQScript::CreateObject(QScriptClass cls)
{
    QClass* cl = (QClass*)cls;
    QObject* obj = (QObject*)malloc(sizeof(QClass*) + sizeof(QValue) * cl->vars_count);
    obj->cls = cl;
    return (QScriptObject)obj;
}

int CQScript::GetObjectValueIndex(QScriptObject object, const char* name)
{
    QObject* obj = (QObject*)object;
    for (int i = 0; i < obj->cls->vars_count; i++)
    {
        if (strcmp(obj->cls->vars_names[i], name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void CQScript::SetObjectValue(QScriptObject object, int index, QValue val)
{
    QObject* obj = (QObject*)object;
    obj->vars[index] = val;
}

QValue CQScript::GetObjectValue(QScriptObject object, int index)
{
    QObject* obj = (QObject*)object;
    return obj->vars[index];
}

int CQScript::GetObjectMethodIndex(QScriptObject object, const char* name)
{
    QObject* obj = (QObject*)object;
    for (int i = 0; i < obj->cls->sigs_count; i++)
    {
        QInterface* sig = obj->cls->sigs[i];
        for (int j = 0; j < sig->count; j++)
        {
            if (strcmp(sig->names[j], name) == 0)
            {
                return i + j;
            }
        }
    }
    return -1;
}

QReturn CQScript::CallObjectMethod(QScriptObject object, int index, QScriptArgs arguments)
{
    QObject* obj = (QObject*)object;
    QArgs* args = (QArgs*)arguments;
    args->self = obj;
    QFunction* func = &obj->cls->methods[index];
    if (func->type == QFunction_Scripting)
    {
        return ((IBaseScriptingInterface*)func->func_scripting->lang)->CallCallback(func->func_scripting, args);
    }
    else
    {
        return func->func_native((QScriptArgs)args);
    }
}

QType CQScript::GetArgType(QScriptArgs args, int index)
{
    QArgs* a = (QArgs*)args;
    return a->args[index].type;
}

QValue CQScript::GetArgValue(QScriptArgs args, int index)
{
    QArgs* a = (QArgs*)args;
    return a->args[index].val;
}

QType CQScript::GetObjectValueType(QScriptObject object, int index)
{
    QObject* obj = (QObject*)object;
    return obj->cls->vars_types[index];
}

void CQScript::InitalizeObject(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    for (int i = 0; i < obj->cls->vars_count; i++)
    {
        switch (obj->cls->vars_types[i])
        {
        case QType_Int:
            obj->vars[i].value_int = 0;
            break;
        case QType_Float:
            obj->vars[i].value_float = 0.0;
            break;
        case QType_Bool:
            obj->vars[i].value_bool = false;
            break;
        case QType_String:
            obj->vars[i].value_modifiable_string = (char*)malloc(obj->cls->vars_sizes[i]*sizeof(char));
            obj->vars[i].value_modifiable_string[0] = '\x00';
            break;
        default:
            break;
        }
    }
}