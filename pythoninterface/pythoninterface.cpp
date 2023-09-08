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
static QObject** s_python_types = 0;
static int* s_python_type_counts = 0;
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
    PyObject* QObjectToPython(QScriptObject obj);
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

struct Python_QObject
{
    PyObject_HEAD
    QObject* obj;
};

static PyObject* Python_Get_QObject_Member(PyObject* obj, void* ch)
{
    QObject* child = (QObject*)ch;
    switch (child->type)
    {
    case QType_Int:
        return PyLong_FromLong(child->value_int);
    case QType_Bool:
        return PyBool_FromLong(child->value_bool);
    case QType_Float:
        return PyFloat_FromDouble((double)child->value_float);
    case QType_String:
        return PyUnicode_FromString(child->value_string);
    default:
        return Py_None;
    }
}

static int Python_Set_QObject_Member(PyObject* obj, PyObject* val, void* ch)
{
    QObject* child = (QObject*)ch;
    switch (child->type)
    {
    case QType_Int:
        child->value_int = PyLong_AsLong(val); break;
    case QType_Bool:
        child->value_bool = Py_IsTrue(val); break;
    case QType_Float:
        child->value_float = (float)PyFloat_AsDouble(val); break;
    case QType_String:
        child->value_string = PyUnicode_AsUTF8(val); break;
    default:
        PyErr_Format(PyExc_TypeError, "%s.%s has invalid type (%i) while trying to set it to %s", obj, child->name, child->type, PyUnicode_AsUTF8(PyObject_Str(val)));
        return -1;
    }
    return 0;
}


static PyObject* Python_Import_Module(void)
{
    PyObject *m = PyModule_Create(&s_python_modules[s_python_init_index]);
    QObject* objs = s_python_types[s_python_init_index];
    for (int i = 0; i != s_python_type_counts[s_python_init_index]; i++)
    {
        QObject* obj = &objs[i];
        if (obj->type != QType_Object)
            continue;
        int member_count, method_count = 0;
        for (int j = 0; j != obj->count; j++)
        {
            QObject* child = obj->objs[j];
            switch (child->type)
            {
            case QType_Int:
            case QType_Bool:
            case QType_Float:
            case QType_String:
                member_count++;
                break;
            case QType_Function:
                method_count++;
                break;
            default:
                break;
            }
        }
        PyMethodDef* methods = (PyMethodDef*)malloc(sizeof(PyMethodDef) * method_count);
        PyGetSetDef* members = (PyGetSetDef*)malloc(sizeof(PyGetSetDef) * member_count);
        int member_index, method_index = 0;
        for (int j = 0; j != obj->count; j++)
        {
            QObject* child = obj->objs[j];
            switch (child->type)
            {
            case QType_Int:
            case QType_Bool:
            case QType_Float:
            case QType_String:
                members[member_index++] = { child->name,Python_Get_QObject_Member,Python_Set_QObject_Member,NULL,child };
                break;
            case QType_Function:
                methods[method_index++] = { child->name,reinterpret_cast<PyCFunction>(child->value_function), METH_QSCRIPT, 0 };
                break;
            default:
                break;
            }
        }
        members[member_index++] = { NULL,NULL,NULL,NULL,NULL };
        methods[method_index++] = { NULL,NULL,NULL,NULL };
        PyTypeObject* pytypeobj = new PyTypeObject();
        pytypeobj->ob_base = { { 1 },NULL };
        pytypeobj->tp_name = obj->name;
        pytypeobj->tp_getset = members;
        pytypeobj->tp_methods = methods;
        PyModule_AddObject(m, obj->name, (PyObject*)pytypeobj);
    }
    s_python_init_index++;

}

void CPythonInterface::ImportClasses(CUtlVector<QObject*>* objs)
{
    s_python_types = (QObject**)malloc(objs->Count() * sizeof(QObject*));
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






typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
} CustomObject;

static PyTypeObject CustomType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "custom.Custom",
    .tp_doc = PyDoc_STR("Custom objects"),
    .tp_basicsize = sizeof(CustomObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
};

static PyModuleDef custommodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "custom",
    .m_doc = "Example module that creates an extension type.",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_custom(void)
{
    PyObject* m;
    if (PyType_Ready(&CustomType) < 0)
        return NULL;

    m = PyModule_Create(&custommodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&CustomType);
    if (PyModule_AddObject(m, "Custom", (PyObject*)&CustomType) < 0) {
        Py_DECREF(&CustomType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}








#endif
