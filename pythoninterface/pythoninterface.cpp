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
#include "Python.h"
#include "Windows.h"
#include "utlvector.h"
#include "convar.h"
#include "tier1.h"
#include "filesystem.h"
#include "qscript/qscript.h"


extern "C"
{
    void* current_interface = 0;
}
static int s_python_init_index = 0;
static PyModuleDef* s_python_modules = 0;
static PyMethodDef* s_python_methods = 0;
IFileSystem* g_pFullFileSystem = 0;
IQScript* g_pQScript = 0;

class CPythonInterface : public IBaseScriptingInterface
{
public:
    virtual InitReturnVal_t Init();
    virtual void Initialize();
    virtual bool Connect(CreateInterfaceFn factory);
    virtual void Shutdown();
    virtual void ImportModules(CUtlVector<QModule*>* modules);
    virtual void LoadMod(const char* path);
    virtual void CallCallback(QCallback* callback, QArgs* args);
    void ExecutePython(const char* code);
};

static CPythonInterface s_PythonInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CPythonInterface, IBaseScriptingInterface, QSCRIPT_LANGAUGE_INTERFACE_VERSION, s_PythonInterface);

InitReturnVal_t CPythonInterface::Init()
{
    s_python_init_index = 0;
    s_python_modules = 0;
    s_python_methods = 0;
    return INIT_OK;
}

bool CPythonInterface::Connect(CreateInterfaceFn factory)
{
    ConnectTier1Libraries(&factory, 1);
    ConVar_Register();
    g_pFullFileSystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL);
    g_pQScript = (IQScript*)factory(QSCRIPT_INTERFACE_VERSION, NULL);
    current_interface = this;
    return true;
}






void CPythonInterface::Initialize()
{
    if (!Py_IsInitialized())
    {
        Py_Initialize();
    }
}

void CPythonInterface::Shutdown()
{
    Py_Finalize();
}






static CUtlBuffer* codebuffer = 0;
void CPythonInterface::LoadMod(const char* path)
{
    int len = strlen(path);
    if (!(path[len - 3] == '.' && path[len - 2] == 'p' && path[len - 1] == 'y'))
        return;

    if (codebuffer == 0)
        codebuffer = new CUtlBuffer();

    codebuffer->Clear();

    if (g_pFullFileSystem->ReadFile(path, NULL, *codebuffer))
        s_PythonInterface.ExecutePython((const char*)(codebuffer->Base()));
}

void CPythonInterface::ExecutePython(const char* code)
{
    // Create and initialize the new interpreter.
    assert(save_tstate != NULL);
    const PyInterpreterConfig config = { 0, 0, 0, 1, 0, 1, (2) };

    PyThreadState* tstate = NULL;
    PyStatus status = Py_NewInterpreterFromConfig(&tstate, &config);
    PyRun_SimpleString(code);
    

}

static PyObject* Python_Import_Module(void)
{
    return PyModule_Create(&s_python_modules[s_python_init_index++]);
}

void CPythonInterface::ImportModules(CUtlVector<QModule*>* modules)
{
    s_python_init_index = 0;

    if (s_python_modules)
        free(s_python_modules);

    s_python_modules = (PyModuleDef*)malloc(modules->Count() * sizeof(PyModuleDef));

    if (s_python_methods)
        free(s_python_methods);

    int methodCount = 0;

    for (int i = 0; i != modules->Count(); i++)
        methodCount += modules->Element(i)->functions->Count() + 1;

    s_python_methods = (PyMethodDef*)malloc(methodCount * sizeof(PyMethodDef));

    int funcIndex = 0;

    for (int i = 0; i < modules->Count(); i++)
    {
        QModule* mod = modules->Element(i);

        int methodsIndex = funcIndex;

        for (int j = 0; j < mod->functions->Count(); j++)
        {
            QFunction* func = mod->functions->Element(j);
            s_python_methods[funcIndex] = {                      //Yes. We just DID modify the python interpreter. Everybody has to modify theirs too if they dont want to deal with assembly hacks :)
                func->name, reinterpret_cast<PyCFunction>(func), METH_QSCRIPT, ""
            };
            funcIndex++;
        }

        s_python_methods[funcIndex] = {
            NULL, NULL, 0, NULL
        };

        funcIndex++;

        s_python_modules[i] = {
            PyModuleDef_HEAD_INIT,
            mod->name, NULL, -1, &s_python_methods[methodsIndex],
            NULL, NULL, NULL, NULL
        };

        PyImport_AppendInittab(mod->name, Python_Import_Module);
    }
}

void CPythonInterface::CallCallback(QCallback* callback, QArgs* args)
{
    if (args->count)
    {
        PyObject* v = PyTuple_New(args->count);
        for (int i = 0; i != args->count; i++)
        {
            switch (args->types[i])
            {
            case 'i':
                PyTuple_SET_ITEM(v, i, PyLong_FromLong((int)args->args[i]));
                break;
            case 's':
                PyTuple_SET_ITEM(v, i, PyUnicode_FromString((const char*)args->args[i]));
                break;
            case 'f':
                PyTuple_SET_ITEM(v, i, PyFloat_FromDouble((double)(*(float*)&args->args[i])));
                break;
            case 'b':
                PyTuple_SET_ITEM(v, i, PyBool_FromLong((bool)(args->args[i])));
                break;
            default:
                Py_DECREF(v);
                return;
            }
        }
        PyObject_Call((PyObject*)callback->callback, v,NULL);
        Py_DECREF(v);
    }
    else
        PyObject_Call((PyObject*)callback->callback, NULL, NULL);
    
}

#endif
