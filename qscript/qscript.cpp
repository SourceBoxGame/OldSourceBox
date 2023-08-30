#define PY_SSIZE_T_CLEAN
#include "cqscript.h"
#include "Windows.h"

static CQScript s_QScript;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CQScript, IQScript, QSCRIPT_INTERFACE_VERSION, s_QScript);







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

QScriptFunction CQScript::CreateModuleFunction(QScriptModule module, const char* name, const char* args, void* funcptr)
{
    QModule* mod = (QModule*)module;
    QFunction* func = new QFunction();
    func->name = name;
    func->args = args;
    func->func = funcptr;
    mod->functions->AddToTail(func);
    return (QScriptFunction)func;
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