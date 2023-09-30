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
static int s_python_module_count = 0;
IFileSystem* g_pFullFileSystem = 0;
IQScript* g_pQScript = 0;
CUtlVector<QModule*>* m_modules = 0;

class CPythonInterface : public IBaseScriptingInterface
{
public:
    virtual InitReturnVal_t Init();
    virtual void Initialize();
    virtual bool Connect(CreateInterfaceFn factory);
    virtual void Shutdown();
    virtual void ImportModules(CUtlVector<QModule*>* modules);
    virtual QInstance* LoadMod(QMod* mod, const char* path);
    virtual QReturn CallCallback(QCallback* callback, QArgs* args);
    QInstance* ExecutePython(const char* code);
    

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
QInstance* CPythonInterface::LoadMod(QMod* mod, const char* path)
{
    int len = strlen(path);
    if (!(path[len - 3] == '.' && path[len - 2] == 'p' && path[len - 1] == 'y'))
        return 0;

    if (codebuffer == 0)
        codebuffer = new CUtlBuffer();

    codebuffer->Clear();

    if (g_pFullFileSystem->ReadFile(path, NULL, *codebuffer))
        return ExecutePython((const char*)(codebuffer->Base()));
}

QInstance* CPythonInterface::ExecutePython(const char* code)
{
    // Create and initialize the new interpreter.
    assert(save_tstate != NULL);
    const PyInterpreterConfig config = { 0, 0, 0, 1, 0, 1, (2) };

    PyThreadState* tstate = NULL;
    PyStatus status = Py_NewInterpreterFromConfig(&tstate, &config);
    PyRun_SimpleString(code);
    
    QInstance* inst = new QInstance();
    inst->lang = current_interface;
    inst->env = tstate;
    return inst;
}

struct Python_QObject
{
    PyObject_HEAD
    QObject* obj;
};

static PyObject* Python_Get_QObject_Member(PyObject* obj, void* ch)
{
    QType type = ((Python_QObject*)obj)->obj->cls->vars[(int)ch].type;
    QValue val = ((Python_QObject*)obj)->obj->vars[(int)ch];
    switch (type)
    {
    case QType_Int:
        return PyLong_FromLong(val.value_int);
    case QType_Bool:
        return PyBool_FromLong(val.value_bool);
    case QType_Float:
        return PyFloat_FromDouble((double)val.value_float);
    case QType_String:
        return PyUnicode_FromString(val.value_string);
    default:
        return Py_None;
    }
}

static int Python_Set_QObject_Member(PyObject* obj, PyObject* pyval, void* ch)
{
    QType type = ((Python_QObject*)obj)->obj->cls->vars[(int)ch].type;
    QValue* val = &(((Python_QObject*)obj)->obj->vars[(int)ch]);
    switch (type)
    {
    case QType_Int:
        val->value_int = PyLong_AsLong(pyval); break;
    case QType_Bool:
        val->value_bool = Py_IsTrue(pyval); break;
    case QType_Float:
        val->value_float = (float)PyFloat_AsDouble(pyval); break;
    case QType_String:
        val->value_string = PyUnicode_AsUTF8(pyval); break;
    default:
        PyErr_Format(PyExc_TypeError, "%s.%s has invalid type (%i) while trying to set it to %s", obj->ob_type->tp_name, ((Python_QObject*)obj)->obj->cls->vars[(int)ch].name, type, PyUnicode_AsUTF8(PyObject_Str(pyval)));
        return -1;
    }
    return 0;
}



PyObject*
Python_QObject_New(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Python_QObject* self;
    self = (Python_QObject*)type->tp_alloc(type, 0);
    if (!self)
        return NULL;
    self->obj = (QObject*)g_pQScript->CreateObject((QScriptClass)self->ob_base.ob_type->tp_userdata);
    g_pQScript->InitializeObject((QScriptObject)self->obj);
    Py_INCREF(self);
    return (PyObject*)self;
}

PyObject* Python_QClass_Init_Subclass(PyObject* self, PyObject* args)
{
    Msg("%llx %s\n", self, PyUnicode_AsUTF8(PyObject_Str(args)));
    return 0;
}

static PyObject* Python_Import_Module()
{
    PyObject *m = PyModule_Create(&s_python_modules[s_python_init_index]);
    QModule* mod = m_modules->Element(s_python_init_index);
    CUtlVector<QClass*>* classes = mod->classes;
    for (int i = 0; i != classes->Count(); i++)
    {
        QClass* cls = classes->Element(i);

        PyMethodDef* methods = (PyMethodDef*)malloc(sizeof(PyMethodDef) * (cls->methods_count + 1));
        PyGetSetDef* members = (PyGetSetDef*)malloc(sizeof(PyGetSetDef) * (cls->vars_count + 1));
        int member_index = 0;
        int method_index = 0;
        for (int j = 0; j != cls->vars_count; j++)
        {
            QVar* var = &cls->vars[j];
            members[member_index++] = { var->name,Python_Get_QObject_Member,Python_Set_QObject_Member,NULL,(void*)j };
        }
        for (int j = 0; j != cls->sigs_count; j++)
        {
            QInterface* sig = cls->sigs[j];
            for (int k = 0; k != sig->count; k++)
            {
                methods[method_index++] = { sig->names[k],reinterpret_cast<PyCFunction>(&cls->methods[method_index]), METH_OBJQSCRIPT, 0};
            }
        }
        members[member_index++] = { NULL,NULL,NULL,NULL,NULL };
        methods[method_index++] = { NULL,NULL,NULL,NULL };


        PyTypeObject* pytypeobj = new PyTypeObject();
        pytypeobj->ob_base = { { 1 },NULL };
        char* cls_name = (char*)malloc(strlen(cls->name) + strlen(s_python_modules[s_python_init_index].m_name) + 2);
        sprintf(cls_name, "%s.%s", s_python_modules[s_python_init_index].m_name, cls->name);
        pytypeobj->tp_name = cls_name;
        pytypeobj->tp_getset = members;
        pytypeobj->tp_methods = methods;
        pytypeobj->tp_basicsize = sizeof(Python_QObject);
        pytypeobj->tp_itemsize = 0;
        pytypeobj->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        pytypeobj->tp_new = Python_QObject_New;
        pytypeobj->tp_userdata = cls;
        PyType_Ready(pytypeobj);
        Py_INCREF(pytypeobj);
        PyModule_AddObject(m, cls->name, (PyObject*)pytypeobj);
    }
    s_python_init_index++;
    return m;

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

    s_python_module_count = modules->Count();

    int funcIndex = 0;

    m_modules = modules;

    for (int i = 0; i < modules->Count(); i++)
    {
        QModule* mod = modules->Element(i);

        int methodsIndex = funcIndex;

        for (int j = 0; j < mod->functions->Count(); j++)
        {
            QFunction* func = mod->functions->Element(j);
            s_python_methods[funcIndex] = {
                func->func_module->name, reinterpret_cast<PyCFunction>(func), METH_QSCRIPT, ""
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

QReturn CPythonInterface::CallCallback(QCallback* callback, QArgs* args)
{
    QReturn ret;
    ret.type = QType_None;
    ret.value.value_int = 0;
    if (args->count)
    {
        PyObject* v = PyTuple_New(args->count);
        for (int i = 0; i != args->count; i++)
        {
            QArg arg = args->args[i];
            QValue val = arg.val;
            switch (arg.type)
            {
            case 'i':
                PyTuple_SET_ITEM(v, i, PyLong_FromLong(val.value_int));
                break;
            case 's':
                PyTuple_SET_ITEM(v, i, PyUnicode_FromString(val.value_string));
                break;
            case 'f':
                PyTuple_SET_ITEM(v, i, PyFloat_FromDouble((double)val.value_float));
                break;
            case 'b':
                PyTuple_SET_ITEM(v, i, PyBool_FromLong(val.value_bool));
                break;
            default:
                Py_DECREF(v);
                return ret;
            }
        }
        PyObject_Call((PyObject*)callback->callback, v,NULL);
        Py_DECREF(v);
    }
    else
        PyObject_Call((PyObject*)callback->callback, NULL, NULL);
    return ret;
}











#endif
