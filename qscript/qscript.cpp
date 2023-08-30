#define PY_SSIZE_T_CLEAN
#include "cqscript.h"
#include "Windows.h"

static CQScript s_QScript;
CQScript* g_pQScript = &s_QScript;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CQScript, IQScript, QSCRIPT_INTERFACE_VERSION, s_QScript);

static int s_remove_me_python_init_index = 0;
static PyModuleDef* s_remove_me_modules = 0;
static PyMethodDef* s_remove_me_methods = 0;



static PyObject* Remove_Me_Import_Module(void)
{
    return PyModule_Create(&s_remove_me_modules[s_remove_me_python_init_index++]);
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
    m_modules = new CUtlVector<QModule*>();
    s_remove_me_python_init_index = 0;
    s_remove_me_modules = 0;
    s_remove_me_methods = 0;
}

void CQScript::AddScriptingInterface(const char* name)
{
    CSysModule* scriptingInterface = Sys_LoadModule(name);
    if (scriptingInterface)
    {
        CreateInterfaceFn scriptingFactory = Sys_GetFactory(scriptingInterface);
        if (scriptingFactory)
        {
            m_interfaces.AddToTail((IBaseScriptingInterface*)scriptingFactory(QSCRIPT_LANGAUGE_INTERFACE_VERSION, NULL));
        }
    }
}

InitReturnVal_t CQScript::Init()
{
    AddScriptingInterface("luainterface.dll");
    AddScriptingInterface("squirrelinterface.dll");
    AddScriptingInterface("pythoninterface.dll");
    
    return INIT_OK;
}

QScriptModule CQScript::CreateModule(const char* name)
{
    QModule* mod = new QModule();
    mod->functions = new CUtlVector<QFunction*>();
    mod->name = name;
    QScriptModule mdld = (QScriptModule)(m_modules->Element(m_modules->AddToTail(mod)));
    return mdld;
}


void CQScript::ExecuteScript(const char* code)
{
    InitPython();
    PyRun_SimpleString(code);
}

typedef PyObject* (*CallbackFunc)(PyObject*, PyObject*, void*);
CallbackFunc d;

PyObject* PyActualCallback(PyObject* self, PyObject* args, void* func)
{
    const char* str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return Py_None;
    ((void(*)(const char*))func)(str);
    return Py_None;
}

void CQScript::InitPython()
{
    if (!Py_IsInitialized())
    {
        ImportModules();
        Py_Initialize();
    }
}

void CQScript::Shutdown()
{
    Py_Finalize();
}

#if defined(__x86_64__) || defined(_WIN64) || 1
char thecallbacker[] = { 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4c, 0x24, 0x08, 0x48, 0x83, 0xec, 0x28, 0x49, 0xb8, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0x48, 0x8b, 0x54, 0x24, 0x38, 0x48, 0x8b, 0x4c, 0x24, 0x30, 0xff, 0x15, 0x05, 0x00, 0x00, 0x00, 0x48, 0x83, 0xc4, 0x28, 0xc3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
QScriptFunction CQScript::CreateModuleFunction(QScriptModule module, const char* name, void* funcptr)
{
    QModule* mod = (QModule*)module;
    QFunction* func = new QFunction();
    func->func = VirtualAlloc(NULL,sizeof(thecallbacker),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    func->name = name;
    memcpy(func->func, thecallbacker, sizeof(thecallbacker));
    void** t = (void**)(((char*)(func->func)) + 16);
    *t = funcptr;
    t = (void**)(((char*)(func->func)) + 45);
    *t = &PyActualCallback;
    return (QScriptFunction)(mod->functions->Element(mod->functions->AddToTail(func)));
}
#else
//If you encounter this error, that means you have to make a special assembly section that has the PyCallbacker function with slight modifications to make it use an absolute address
#error Implement me!
#endif

#pragma optimize( "", off )
PyObject* PyCallbacker(PyObject* self, PyObject* args)
{
    return d(self, args, (void*)0x123456789ABCDEF0);
}
#pragma optimize( "", on )


void CQScript::ImportModules()
{
    s_remove_me_python_init_index = 0;
    if (s_remove_me_modules)
    {
        free(s_remove_me_modules);
    }
    s_remove_me_modules = (PyModuleDef*)malloc(m_modules->Count()*sizeof(PyModuleDef));
    if (s_remove_me_methods)
    {
        free(s_remove_me_methods);
    }
    int methodCount = 0;
    for (int i = 0; i != m_modules->Count(); i++)
    {
        methodCount += m_modules->Element(i)->functions->Count() + 1;
    }
    s_remove_me_methods = (PyMethodDef*)malloc(methodCount * sizeof(PyMethodDef));
    int funcIndex = 0;
    for (int i = 0; i < m_modules->Count(); i++)
    {
        QModule* mod = m_modules->Element(i);
        int methodsIndex = funcIndex;
        for (int j = 0; j < mod->functions->Count(); j++)
        {
            QFunction* func = mod->functions->Element(j);
            s_remove_me_methods[funcIndex] = {
                func->name, (PyCFunction)func->func, METH_VARARGS, ""
            };
            funcIndex++;
        }
        s_remove_me_methods[funcIndex] = {
            NULL, NULL, 0, NULL
        };
        funcIndex++;
        s_remove_me_modules[i] = {
            PyModuleDef_HEAD_INIT,
            mod->name, NULL, -1, &s_remove_me_methods[methodsIndex],
            NULL, NULL, NULL, NULL
        };
        PyImport_AppendInittab(mod->name, Remove_Me_Import_Module);
    }
}







/*
#define PY_SSIZE_T_CLEAN
#include "convar.h"
#include "Python.h"




static PyObject*
sourcebox_msg(PyObject* self, PyObject* args)
{
    const char* str;
    if (!PyArg_ParseTuple(args, "s", str))
        return NULL;
    Msg(str);
    return Py_None;
}

static PyMethodDef SourceBoxMethods[] = {
    {"Msg", sourcebox_msg, METH_VARARGS,
     "Print something to the console"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef SourceBoxModule = {
    PyModuleDef_HEAD_INIT, "sourcebox", NULL, -1, SourceBoxMethods,
    NULL, NULL, NULL, NULL
};

static PyObject*
PyInit_sourcebox(void)
{
    return PyModule_Create(&SourceBoxModule);
}


CON_COMMAND(execute_python, "The beginning")
{
    if (!Py_IsInitialized())
    {
        Py_SetProgramName(L"Command");
        PyImport_AppendInittab("sourcebox", &PyInit_sourcebox);
        Py_Initialize();
    }
	PyRun_SimpleString(args.GetCommandString());
	if (int err = Py_FinalizeEx() < 0)
	{
		Msg("python error: %x (%i)", err, err);
	}
}
*/