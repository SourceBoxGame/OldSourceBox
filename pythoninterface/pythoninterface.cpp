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

static int s_python_init_index = 0;
static PyModuleDef* s_python_modules = 0;
static PyMethodDef* s_python_methods = 0;
IFileSystem* g_pFullFileSystem = 0;

class CPythonInterface : public IBaseScriptingInterface
{
public:
    virtual InitReturnVal_t Init();
    virtual void Initialize();
    virtual bool Connect(CreateInterfaceFn factory);
    virtual void Shutdown();
    virtual void ImportModules(CUtlVector<QModule*>* modules);
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
CON_COMMAND(python_run_file, "The beginning")
{
    if (args.ArgC() == 1)
    {
        Msg("Include the path of the file you want to execute!");
        return;
    }

    if(codebuffer == 0)
        codebuffer = new CUtlBuffer();

    codebuffer->Clear();

    if(g_pFullFileSystem->ReadFile(args.ArgS(), NULL, *codebuffer))
        s_PythonInterface.ExecutePython((const char*)(codebuffer->Base()));
}

void CPythonInterface::ExecutePython(const char* code)
{
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

#endif
