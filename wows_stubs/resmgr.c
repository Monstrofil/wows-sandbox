/*
 * wows_stubs/resmgr.c — ResMgr module stub with DataSection.
 */
#include "common.h"

static PyObject *DataSection_class = NULL;

/* Lazily build the DataSection stub class. Borrowed reference (kept alive
 * for the process lifetime); NULL on failure. */
static PyObject *
get_DataSection_class(void)
{
    if (DataSection_class == NULL) {
        PyObject *d = PyDict_New();
        PyObject *bi = PyImport_ImportModule("__builtin__");
        PyDict_SetItemString(d, "__builtins__", bi);
        Py_XDECREF(bi);
        PyRun_String(
            "class DataSection(object):\n"
            "    def __init__(self, name=''):\n"
            "        self._name = name\n"
            "    def children(self): return []\n"
            "    def keys(self): return []\n"
            "    def values(self): return []\n"
            "    def items(self): return []\n"
            "    def has_key(self, k): return False\n"
            "    def __contains__(self, k): return False\n"
            "    def __getitem__(self, k): return DataSection()\n"
            "    def __iter__(self): return iter([])\n"
            "    def __nonzero__(self): return False\n"
            "    def __len__(self): return 0\n"
            "    def readString(self, k='', d=''): return d\n"
            "    def readStrings(self, k=''): return []\n"
            "    def readInt(self, k='', d=0): return d\n"
            "    def readFloat(self, k='', d=0.0): return d\n"
            "    def readBool(self, k='', d=False): return d\n"
            "    def readVector2(self, k='', d=None): return d\n"
            "    def readVector3(self, k='', d=None): return d\n"
            "    def __getattr__(self, n):\n"
            "        if n.startswith('_'): raise AttributeError(n)\n"
            "        return DataSection()\n",
            Py_file_input, d, d);
        DataSection_class = PyDict_GetItemString(d, "DataSection");
        Py_XINCREF(DataSection_class);
        gc_untrack_class_and_dict(DataSection_class, d);
    }
    return DataSection_class;
}

PyObject *
ResMgr_openSection(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *cls = get_DataSection_class();
    if (cls == NULL)
        Py_RETURN_NONE;
    return PyObject_CallObject(cls, NULL);
}

static PyMethodDef ResMgr_methods[] = {
    {"openSection", (PyCFunction)ResMgr_openSection,
     METH_VARARGS | METH_KEYWORDS, NULL},
    NOP_VARARGS(isFile),
    NOP_VARARGS(isDir),
    NOP_VARARGS(resolveToAbsolutePath),
    END
};

PyObject *init_ResMgr(void)
{
    PyObject *m = make_module("ResMgr", ResMgr_methods, "ResMgr stub");
    /* Since 15.7.0 CameraTrajectoryStorage does `from ResMgr import
     * DataSection` at module level (used for type annotations / isinstance),
     * so the class must be exported as a module attribute, not only returned
     * from openSection(). Without it the BWPersonality boot chain aborts in
     * LobbySpace.EllipticPoITrajectories. */
    PyObject *cls = get_DataSection_class();
    if (m != NULL && cls != NULL) {
        Py_INCREF(cls);
        PyModule_AddObject(m, "DataSection", cls);
    }
    return m;
}
