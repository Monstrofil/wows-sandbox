/*
 * wows_stubs/common.c — FlexBase type and shared helpers.
 */
#include "common.h"

static PyObject *_FlexBaseType = NULL;

PyObject *
get_flex_base_type(void)
{
    if (_FlexBaseType == NULL) {
        PyObject *d = PyDict_New();
        PyObject *bi = PyImport_ImportModule("__builtin__");
        PyDict_SetItemString(d, "__builtins__", bi);
        Py_XDECREF(bi);
        PyRun_String(
            "class _FlexBase(object):\n"
            "    def __init__(self, *args, **kwargs):\n"
            "        pass\n"
            "    def __getattr__(self, name):\n"
            "        if name.startswith('_'): raise AttributeError(name)\n"
            "        return _FlexBase()\n"
            "    def __call__(self, *args, **kwargs):\n"
            "        return _FlexBase()\n"
            "    def __iadd__(self, other): return self\n"
            "    def __isub__(self, other): return self\n"
            "    def __nonzero__(self): return False\n"
            "    def __iter__(self): return iter([])\n"
            "    def __str__(self): return ''\n"
            "    def __int__(self): return 0\n"
            "    def __float__(self): return 0.0\n",
            Py_file_input, d, d);
        _FlexBaseType = PyDict_GetItemString(d, "_FlexBase");
        Py_XINCREF(_FlexBaseType);
        /* Remove class and dict from GC tracking — they are immortal */
        gc_untrack_class_and_dict(_FlexBaseType, d);
    }
    Py_XINCREF(_FlexBaseType);
    return _FlexBaseType;
}
