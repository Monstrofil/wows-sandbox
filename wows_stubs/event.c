/*
 * wows_stubs/event.c — Event module stub.
 *
 * Event.Event() returns an object with __iadd__/__isub__/__call__
 * for the observer pattern used throughout WoWS scripts.
 */
#include "common.h"

static PyObject *Event_class = NULL;

PyObject *
Event_Event(PyObject *self, PyObject *args, PyObject *kwargs)
{
    if (Event_class == NULL) {
        PyObject *d = PyDict_New();
        PyObject *bi = PyImport_ImportModule("__builtin__");
        PyDict_SetItemString(d, "__builtins__", bi);
        Py_XDECREF(bi);
        PyRun_String(
            "class Event(object):\n"
            "    def __init__(self, *a, **kw):\n"
            "        self._handlers = []\n"
            "    def __iadd__(self, handler):\n"
            "        self._handlers.append(handler)\n"
            "        return self\n"
            "    def __isub__(self, handler):\n"
            "        try: self._handlers.remove(handler)\n"
            "        except ValueError: pass\n"
            "        return self\n"
            "    def __call__(self, *a, **kw):\n"
            "        for h in self._handlers:\n"
            "            try: h(*a, **kw)\n"
            "            except: pass\n"
            "    def clear(self):\n"
            "        self._handlers = []\n",
            Py_file_input, d, d);
        Event_class = PyDict_GetItemString(d, "Event");
        Py_XINCREF(Event_class);
    }

    if (Event_class == NULL)
        Py_RETURN_NONE;

    return PyObject_Call(Event_class, args ? args : PyTuple_New(0), kwargs);
}

static PyMethodDef Event_methods[] = {
    {"Event",   (PyCFunction)Event_Event, METH_VARARGS | METH_KEYWORDS, NULL},
    {"Handler", (PyCFunction)Event_Event, METH_VARARGS | METH_KEYWORDS, NULL},
    END
};

PyObject *init_Event(void)
{
    return make_module("Event", Event_methods, "Event stub");
}
