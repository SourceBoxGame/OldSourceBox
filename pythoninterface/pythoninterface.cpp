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

static int s_python_init_index = 0;
static PyModuleDef* s_python_modules = 0;
static PyMethodDef* s_python_methods = 0;

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
    return true;
}



typedef PyObject* (*CallbackFunc)(PyObject*, PyObject*, void*, const char*);
CallbackFunc d;
const char* dd;

PyObject* PyActualCallback(PyObject* self, PyObject* args, void* func, const char* argtypes)
{
    if (strlen(argtypes) != PyObject_Length(args)) // TODO : maybe error too
        return Py_None;
    QArgs* qargs = new QArgs();
    qargs->types = argtypes;
    qargs->count = PyObject_Length(args);
    qargs->args = (void**)malloc(qargs->count * sizeof(void*));
    for (int i = 0; i != qargs->count; i++)
    {
        PyObject* item = PyTuple_GET_ITEM(args, i);
        switch (argtypes[i])
        {
        case 's':
            if (PyUnicode_Check(item))
                qargs->args[i] = PyUnicode_1BYTE_DATA(item);
            else
                goto failure;
            break;
        case 'i':
            if (PyLong_Check(item))
                qargs->args[i] = (void*)PyLong_AS_LONG(item);
            else
                goto failure;
            break;
        case 'f':
            if (PyFloat_Check(item))
            {
                float chyba_ciebie_cos_pojebalo = (float)PyFloat_AS_DOUBLE(item);
                qargs->args[i] = reinterpret_cast<void*&>(chyba_ciebie_cos_pojebalo);  //WHO FUCKING CARES IF A VOID* IS NOT A FLOAT  THEY ARE THE SAME FUCKING AMOUNT OF BYTES  ACCESSED IN THE EXACT SAME FUCKING WAY  AND IM SUPPOSED TO JUST ACCEPT THAT I CANNOT FORCE THESE FUCKING BYTES INTO THE EXACT SAME AMOUNT OF SPACE IT WOULD TAKE UP BUT WITH A DIFFERENT FUCKING AUTISM LABEL SLAPPED ON TOP OF IT??????????
            }                                                                          //we should start writing assembly instead
            else
                goto failure;
            break;
        default:
            goto failure;
            break;
        }
        continue;
    failure:
        free(qargs->args);
        free(qargs);
        return Py_None;
    }
    ((void(*)(QScriptArgs))func)((QScriptArgs)qargs);
    return Py_None;
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



#if defined(__x86_64__) || defined(_WIN64) || 1
char thecallbacker[] = { 0x48,0x89,0x54,0x24,0x10,0x48,0x89,0x4c,0x24,0x08,0x48,0x83,0xec,0x28,0x49,0xb9,0xa9,0xcb,0xed,0x0f,0xf0,0xde,0xbc,0x9a,0x49,0xb8,0x21,0x43,0x65,0x87,0x78,0x56,0x34,0x12,0x48,0x8b,0x54,0x24,0x38,0x48,0x8b,0x4c,0x24,0x30,0xff,0x15,0x05,0x00,0x00,0x00,0x48,0x83,0xc4,0x28,0xc3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
PyCFunction PythonCreateModuleFunction(void* funcptr, const char* args)
{
    
    void* func = VirtualAlloc(NULL, sizeof(thecallbacker), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    memcpy(func, thecallbacker, sizeof(thecallbacker));
    void** t = (void**)(((char*)func) + 16);
    *t = (void*)args;
    t = (void**)(((char*)func) + 26);
    *t = funcptr;
    t = (void**)(((char*)func) + 55);
    *t = &PyActualCallback;
    return (PyCFunction)func;
}
#else
//If you encounter this error, that means you have to make a special assembly section that has the PyCallbacker function with slight modifications to make it use an absolute address
#error Implement me!
#endif

#pragma optimize( "", off )
PyObject* PyCallbacker(PyObject* self, PyObject* args)
{
    return d(self, args, (void*)0x1234567887654321, (const char*)0x9ABCDEF00FEDCBA9);
}
#pragma optimize( "", on )

CON_COMMAND(execute_python, "The beginning")
{
    s_PythonInterface.ExecutePython(args.ArgS());
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
            s_python_methods[funcIndex] = {
                func->name, PythonCreateModuleFunction(func->func,func->args), METH_VARARGS, ""
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
