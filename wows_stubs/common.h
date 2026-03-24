/*
 * wows_stubs/common.h — Shared macros and helpers for engine stub modules.
 *
 * Provides NOP functions, method table macros, and module creation helpers
 * inspired by BigWorld's PY_AUTO_MODULE_FUNCTION pattern.
 */
#ifndef WOWS_STUBS_COMMON_H
#define WOWS_STUBS_COMMON_H

#include "Python.h"

/* ── NOP function implementations ─────────────────────────────────────── */

#define UNUSED __attribute__((unused))

UNUSED static PyObject *nop(PyObject *self, PyObject *args, PyObject *kw)
    { Py_RETURN_NONE; }
UNUSED static PyObject *nop_str(PyObject *self, PyObject *args)
    { return PyString_FromString(""); }
UNUSED static PyObject *nop_int(PyObject *self, PyObject *args)
    { return PyInt_FromLong(0); }
UNUSED static PyObject *nop_false(PyObject *self, PyObject *args)
    { Py_RETURN_FALSE; }
UNUSED static PyObject *nop_true(PyObject *self, PyObject *args)
    { Py_RETURN_TRUE; }
UNUSED static PyObject *nop_dict(PyObject *self, PyObject *args)
    { return PyDict_New(); }
UNUSED static PyObject *nop_list(PyObject *self, PyObject *args)
    { return PyList_New(0); }
UNUSED static PyObject *nop_tuple(PyObject *self, PyObject *args)
    { return PyTuple_New(0); }
UNUSED static PyObject *nop_tuple2(PyObject *self, PyObject *args)
    { return Py_BuildValue("(dd)", 1.0, 1.0); }
UNUSED static PyObject *nop_tuple3(PyObject *self, PyObject *args)
    { return Py_BuildValue("(iii)", 0, 0, 0); }
UNUSED static PyObject *nop_str_url(PyObject *self, PyObject *args)
    { return PyString_FromString("http://localhost/stub"); }
UNUSED static PyObject *nop_str_consumer(PyObject *self, PyObject *args)
    { return PyString_FromString("consumer"); }

/* ── Method table entry macros ────────────────────────────────────────── */

#define NOP_NOARGS(name) \
    {#name, (PyCFunction)nop, METH_NOARGS, NULL}
#define NOP_VARARGS(name) \
    {#name, (PyCFunction)nop, METH_VARARGS|METH_KEYWORDS, NULL}
#define NOP_STR(name) \
    {#name, (PyCFunction)nop_str, METH_VARARGS, NULL}
#define NOP_INT(name) \
    {#name, (PyCFunction)nop_int, METH_VARARGS, NULL}
#define NOP_FALSE(name) \
    {#name, (PyCFunction)nop_false, METH_VARARGS, NULL}
#define NOP_TRUE(name) \
    {#name, (PyCFunction)nop_true, METH_VARARGS, NULL}
#define NOP_DICT(name) \
    {#name, (PyCFunction)nop_dict, METH_VARARGS, NULL}
#define NOP_LIST(name) \
    {#name, (PyCFunction)nop_list, METH_VARARGS, NULL}
#define NOP_TUPLE(name) \
    {#name, (PyCFunction)nop_tuple, METH_VARARGS, NULL}
#define NOP_TUPLE2(name) \
    {#name, (PyCFunction)nop_tuple2, METH_VARARGS, NULL}
#define NOP_TUPLE3(name) \
    {#name, (PyCFunction)nop_tuple3, METH_VARARGS, NULL}
#define END {NULL, NULL, 0, NULL}

/* ── Module creation helpers ──────────────────────────────────────────── */

/* Create a module and register it in sys.modules with __path__. */
static inline PyObject *
make_module(const char *name, PyMethodDef *methods, const char *doc)
{
    PyObject *mod = Py_InitModule3(name, methods, doc);
    if (mod == NULL) return NULL;
    PyModule_AddObject(mod, "__path__", PyList_New(0));
    PyModule_AddObject(mod, "__wows_stub__", (Py_INCREF(Py_True), Py_True));
    return mod;
}

static inline void
add_int(PyObject *mod, const char *name, long val)
{
    PyModule_AddIntConstant(mod, name, val);
}

static inline void
add_str(PyObject *mod, const char *name, const char *val)
{
    PyModule_AddStringConstant(mod, name, val);
}

/* ── Empty-module macro ───────────────────────────────────────────────── */

UNUSED static PyMethodDef _empty_methods[] = { END };

/*
 * STUB_MODULE(Name) — declare an empty engine stub module.
 * Generates init_Name() that creates a module with no methods.
 * For modules that only need to exist in sys.modules.
 */
#define STUB_MODULE(varname, modname) \
    PyObject *init_##varname(void) { \
        return make_module(modname, _empty_methods, modname " stub"); \
    }

/*
 * STUB_MODULE_METHODS(Name, methods) — declare a stub with methods.
 * Generates init_Name() that creates a module with the given method table.
 */
#define STUB_MODULE_METHODS(varname, modname, methods) \
    PyObject *init_##varname(void) { \
        return make_module(modname, methods, modname " stub"); \
    }

/*
 * INIT_STUB(Name) — call init_Name() and goto err on failure.
 * Used in wows_stubs_install().
 */
#define INIT_STUB(name) \
    if (!init_##name()) goto err

/* ── GC-safety helper ─────────────────────────────────────────────────── */

/*
 * Untrack a class created by PyRun_String and its enclosing dict from
 * the cyclic GC.  Heap types (created via Python `class` statements)
 * have Py_TPFLAGS_HAVE_GC and a tp_traverse that walks back into the
 * dict used as globals/locals for PyRun_String.  If that dict is leaked
 * (which we do on purpose to keep method closures alive), GC's
 * visit_decref segfaults because it encounters objects in an
 * inconsistent state.
 *
 * Calling PyObject_GC_UnTrack on both the class and the dict removes
 * them from GC's tracked set entirely.  Since they are immortal (kept
 * alive for the lifetime of the process), they never need collection.
 */
/*
 * Custom tp_new that untracks every instance from GC immediately after
 * creation. This is inherited by subclasses, so game code that does
 * `class MyShip(BigWorld.Entity)` also produces untracked instances.
 *
 * Why: subtype_traverse visits instance __dict__ BEFORE calling the
 * base tp_traverse. Our stub instances can create deep reference chains
 * (FlexBase.__getattr__ returns new instances), causing GC stack overflow.
 * Untracking prevents GC from ever traversing these objects.
 */
UNUSED static PyObject *
_stub_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *self = PyType_GenericNew(type, args, kwds);
    if (self != NULL)
        PyObject_GC_UnTrack(self);
    return self;
}

static inline void
gc_untrack_class_and_dict(PyObject *cls, PyObject *dict)
{
    if (cls  != NULL) {
        PyObject_GC_UnTrack(cls);
        if (PyType_Check(cls)) {
            PyTypeObject *tp = (PyTypeObject *)cls;
            /* Override tp_new so all instances (including subclass
             * instances) are immediately untracked from GC. */
            tp->tp_new = _stub_tp_new;
        }
    }
    if (dict != NULL) PyObject_GC_UnTrack(dict);
}

/* ── FlexBase type ────────────────────────────────────────────────────── */

/*
 * FlexBase: a Python class that accepts any __init__ args, returns
 * FlexBase() from __getattr__ and __call__, and supports common dunder
 * methods. Used as a base type for engine classes that game code inherits
 * from or instantiates.
 *
 * Created once and cached. The enclosing dict is intentionally leaked
 * to prevent dangling references in class method closures.
 */
PyObject *get_flex_base_type(void);

#endif /* WOWS_STUBS_COMMON_H */
