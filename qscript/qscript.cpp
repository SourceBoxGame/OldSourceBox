#include "cqscript.h"
#include "filesystem.h"
#include "utlstring.h"

static CQScript s_QScript;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CQScript, IQScript, QSCRIPT_INTERFACE_VERSION, s_QScript);


static const QReturn QNone = {
    QType_None,0
};

IFileSystem* g_pFullFileSystem = 0;



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
    strtotype['p'] = QType_Function;
    m_interfaces = new CUtlVector<IBaseScriptingInterface*>();
    m_modules = new CUtlVector<QModule*>();
    m_mods = new CUtlStringMap<QMod*>();
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


void CQScript::LoadFilesInDirectory(const char* modname, const char* folder, const char* filename)
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
            QMod* mod = GetOrCreateMod(modname);
            char pScriptPath[MAX_PATH];
            strncpy(pScriptPath, folder, MAX_PATH);
            V_AppendSlash(pScriptPath, MAX_PATH);
            strncat(pScriptPath, pszFileName, MAX_PATH);
            if (mod->instances.Defined(pScriptPath))
            {
                pszFileName = g_pFullFileSystem->FindNext(findHandle);
                continue;
            }
            mod->name = modname;
            for (int i = 0; i != m_interfaces->Count(); i++)
            {
                QInstance* ins = m_interfaces->Element(i)->LoadMod(mod, pFilePath);
                if (ins)
                    mod->instances[pScriptPath] = ins;
            }
        }
        pszFileName = g_pFullFileSystem->FindNext(findHandle);
    }
}

void CQScript::LoadFile(const char* path)
{
    if (!IsValidPath(path))
        return;
    const char* scriptPath = strstr(path, "/");
    if (scriptPath == 0 || scriptPath == path)
        return;
    char* modname = new char[MAX_PATH+1];
    int modlen = (int)(scriptPath - path);
    strncpy(modname, path, modlen);
    modname[modlen] = 0;
    QMod* mod = GetOrCreateMod(modname);
    if (mod->instances.Defined(path) && mod->instances[path] != 0)
    {
        free(modname);
        return;
    }
    mod->name = modname;
    char globalPath[MAX_PATH];
    strcpy(globalPath, "mods/");
    strncat(globalPath, path, MAX_PATH);
    for (int i = 0; i != m_interfaces->Count(); i++)
    {
        QInstance* ins = m_interfaces->Element(i)->LoadMod(mod, globalPath);
        if (ins)
            mod->instances[path] = ins;
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
            LoadFilesInDirectory(pszFileName,pszFileName, filename);
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
                LoadFilesInDirectory(pszFileName,path, filename);
            }
            pszFileName = g_pFullFileSystem->FindNext(findHandle);
            continue;
        }
        pszFileName = g_pFullFileSystem->FindNext(findHandle);
    }
}

QMod* CQScript::GetOrCreateMod(const char* name)
{
    if (m_mods->Defined(name))
        return (*m_mods)[name];
    else
    {
        QMod* mod = new QMod();
        (*m_mods)[name] = mod;
        return mod;
    }
}

QScriptClassCreator CQScript::StartClass(const char* name, QScriptClass parent)
{
    QClassCreator* cr = new QClassCreator();
    cr->name = name;
    cr->parent = (QClass*)parent;
    return (QScriptClassCreator)cr;
}

void CQScript::AddMethod(QScriptClassCreator creator, const char* name, QType* params, QCFunc func, bool is_private)
{
    QClassCreator* cr = (QClassCreator*)creator;
    QClassCreatorMethod* meth = new QClassCreatorMethod();
    meth->name = name;
    meth->params = params;
    int i = 0;
    for (i = 0; params[i]; i++);
    {}
    meth->params_count = i;
    meth->is_scripting = false;
    meth->native_func = func;
    meth->is_private = is_private;
    cr->methods.AddToTail(meth);
}

void CQScript::AddScriptingMethod(QScriptClassCreator creator, const char* name, QScriptCallback callback, bool is_private)
{
    QClassCreator* cr = (QClassCreator*)creator;
    QClassCreatorMethod* meth = new QClassCreatorMethod();
    meth->name = name;
    meth->is_scripting = true;
    meth->scripting_func = (QCallback*)callback;
    meth->is_private = is_private;
    cr->methods.AddToTail(meth);
}

void CQScript::AddVariable(QScriptClassCreator creator, const char* name, QType type, QValue defaultval, bool is_private)
{
    QClassCreator* cr = (QClassCreator*)creator;
    QVar* var = new QVar();
    var->name = name;
    var->is_private = is_private;
    var->type = type;
    var->size = 0;
    var->defaultval = defaultval;
    cr->vars.AddToTail(var);
}

void CQScript::AddString(QScriptClassCreator creator, const char* name, const char* defaultval, bool is_private)
{
    QClassCreator* cr = (QClassCreator*)creator;
    QVar* var = new QVar();
    var->name = name;
    var->is_private = is_private;
    var->type = QType_String;
    var->size = 1<<Qlog2(strlen(defaultval));
    var->defaultval.value_string = defaultval;
    cr->vars.AddToTail(var);
}

QScriptClass CQScript::FinishClass(QScriptClassCreator creator)
{
    QClassCreator* cr = (QClassCreator*)creator;
    QClass* cl = new QClass();
    if (cr->parent)
    {
        cl->methods_count = cr->parent->methods_count + cr->methods.Count();
        cl->sigs_count = cr->parent->sigs_count + 1;
        cl->vars_count = cr->parent->vars_count + cr->vars.Count();
        cl->methods = new QFunction[cl->methods_count];
        cl->sigs = new QInterface*[cl->sigs_count];
        cl->vars = new QVar[cl->vars_count];
        memcpy(cl->methods, cr->parent->methods, cr->parent->methods_count * sizeof(QFunction));
        memcpy(cl->sigs, cr->parent->sigs, cr->parent->sigs_count * sizeof(QInterface*));
        memcpy(cl->vars, cr->parent->vars, cr->parent->vars_count * sizeof(QVar));
    }
    QInterface* in = new QInterface();
    in->count = cr->methods.Count();
    if (in->count)
    {
        in->args = (QParams*)malloc(cr->methods.Count() * sizeof(QParams*));
        in->names = (const char**)malloc(cr->methods.Count() * sizeof(const char*));
        for (int i = 0; i != cr->methods.Count(); i++)
        {
            QClassCreatorMethod* meth = cr->methods[i];
            if (meth->params)
            {
                in->args[i].count = meth->params_count;
                in->args[i].types = new QType[in->args[i].count];
                memcpy(in->args[i].types, meth->params, in->args[i].count * sizeof(QType));
            }
            else
            {
                in->args[i].count = 0;
                in->args[i].types = 0;
            }
            in->names[i] = meth->name;
        }
    }
    if (cr->parent)
    {
        for (int i = 0; i != cr->methods.Count();i++)
        {
            cl->methods[cr->parent->methods_count + i].always_zero = 0;
            if (cr->methods[i]->is_scripting)
            {
                cl->methods[cr->parent->methods_count + i].type = QFunction_Scripting;
                cl->methods[cr->parent->methods_count + i].func_scripting = cr->methods[i]->scripting_func;
            }
            else
            {
                cl->methods[cr->parent->methods_count + i].type = QFunction_Native;
                cl->methods[cr->parent->methods_count + i].func_native = cr->methods[i]->native_func;
            }
        }
        cl->sigs[cr->parent->sigs_count] = in;
        for (int i = 0; i < cr->vars.Count(); i++)
        {
            memcpy(&cl->vars[cr->parent->vars_count + i], cr->vars[i], sizeof(QVar));
        }
    }
    else
    {
        cl->methods_count = cr->methods.Count();
        cl->methods = new QFunction[cr->methods.Count()];
        for (int i = 0; i != cr->methods.Count();i++)
        {
            cl->methods[i].always_zero = 0;
            if (cr->methods[i]->is_scripting)
            {
                cl->methods[i].type = QFunction_Scripting;
                cl->methods[i].func_scripting = cr->methods[i]->scripting_func;
            }
            else
            {
                cl->methods[i].type = QFunction_Native;
                cl->methods[i].func_native = cr->methods[i]->native_func;
            }
        }
        cl->sigs_count = 1;
        cl->sigs = new QInterface*[1];
        cl->sigs[0] = in;
        cl->vars_count = cr->vars.Count();
        cl->vars = new QVar[cl->vars_count];
        for (int i = 0; i < cl->vars_count; i++)
        {
            memcpy(&cl->vars[i], (cr->vars[i]), sizeof(QVar));
        }
    }
    cl->name = cr->name;
    delete cr;
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
    const char* othername;
    for (int i = 0; i < obj->cls->vars_count; i++)
    {
        othername = obj->cls->vars[i].name;
        if (strcmp(othername, name) == 0)
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


void CQScript::SetObjectString(QScriptObject object, int index, const char* str)
{
    QObject* obj = (QObject*)object;
    int obj_str_size = mallocsize(obj->vars[index].value_modifiable_string);
    int set_str_size = strlen(str);
    if (obj_str_size < set_str_size)
    {
        char* old_str = obj->vars[index].value_modifiable_string;
        obj->vars[index].value_modifiable_string = (char*)malloc(1<<Qlog2(set_str_size));
        strcpy(obj->vars[index].value_modifiable_string, str);
        delete old_str;
    }
    else
        strcpy(obj->vars[index].value_modifiable_string, str);
}

QValue CQScript::GetObjectValue(QScriptObject object, int index)
{
    QObject* obj = (QObject*)object;
    return obj->vars[index];
}

QScriptFunction CQScript::GetObjectMethod(QScriptObject object, int index)
{
    QObject* obj = (QObject*)object;
    return (QScriptFunction)(&(obj->cls->methods[index]));
}

int CQScript::GetObjectMethodIndex(QScriptObject object, const char* name)
{
    QObject* obj = (QObject*)object;
    int index = 0;
    for (int i = 0; i < obj->cls->sigs_count; i++)
    {
        QInterface* sig = obj->cls->sigs[i];
        for (int j = 0; j < sig->count; j++)
        {
            if (strcmp(sig->names[j], name) == 0)
            {
                return index;
            }
            index++;
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
    return obj->cls->vars[index].type;
}

void CQScript::InitializeObject(QScriptObject object)
{
    QObject* obj = (QObject*)object;
    for (int i = 0; i < obj->cls->vars_count; i++)
    {
        switch (obj->cls->vars[i].type)
        {
        case QType_Int:
        case QType_Float:
        case QType_Bool:
            obj->vars[i] = obj->cls->vars[i].defaultval;
            break;
        case QType_String:
            obj->vars[i].value_modifiable_string = (char*)malloc(obj->cls->vars[i].size*sizeof(char));
            strcpy(obj->vars[i].value_modifiable_string, obj->cls->vars[i].defaultval.value_modifiable_string);
            break;
        default:
            break;
        }
    }
}

void CQScript::CallFunction(QScriptFunction function, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int len = strlen(fmt);
    QArgs* args = (QArgs*)malloc(sizeof(QArgs) + len * sizeof(QArg));
    args->count = len;
    for (int i = 0; i < len; i++)
    {
        QValue val;
        switch (fmt[i])
        {
        case 's':
            val.value_string = va_arg(va, const char*);
            args->args[i].val = val;
            args->args[i].type = QType_String;
            break;
        case 'f':
            val.value_float = va_arg(va, float);
            args->args[i].val = val;
            args->args[i].type = QType_Float;
            break;
        case 'i':
            val.value_int = va_arg(va, int);
            args->args[i].val = val;
            args->args[i].type = QType_Int;
            break;
        case 'b':
            val.value_bool = va_arg(va, bool);
            args->args[i].val = val;
            args->args[i].type = QType_Bool;
            break;
        case 'o':
            val.value_object = va_arg(va, QScriptObject);
            args->args[i].val = val;
            args->args[i].type = QType_Object;
            break;
        default:
            Warning("Tried to create invalid arg type '%c' at %i.\n", fmt[i], i);
            free(args);
            return;
        }
    }
    CallFunctionEx(function, args);
    free(args);
    return;
}

void CQScript::CallFunctionEx(QScriptFunction function, QArgs* args)
{
    QFunction* func = (QFunction*)function;
    if (func->always_zero)
        return;
    switch (func->type)
    {
    case QFunction_Module:
        func->func_module->func((QScriptArgs)args);
        break;
    case QFunction_Native:
        func->func_native((QScriptArgs)args);
        break;
    case QFunction_Scripting:
        ((IBaseScriptingInterface*)func->func_scripting->lang)->CallCallback(func->func_scripting, args);
        break;
    case QFunction_Void:
        break;
    }
}