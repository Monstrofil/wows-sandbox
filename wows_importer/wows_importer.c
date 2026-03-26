/*
 * wows_importer.c — PEP 302 meta_path importer for WoWS scripts.zip.
 *
 * Pure C implementation: uses zip_reader for archive access and
 * wows_decrypt for the 4-stage decryption. No Python stdlib dependencies.
 */
#include "wows_importer.h"
#include "../wows_stubs/wows_decrypt.h"
#include "../zip_reader/zip_reader.h"
#include <string.h>
#include <stdio.h>

/* ── WoWSImporter type ───────────────────────────────────────────────── */

typedef struct {
    PyObject_HEAD
    zip_archive_t *za;      /* C zip archive handle */
    char *zip_path;         /* path string */
    PyObject *index;        /* dict: module_name (str) → zip_entry_name (str) */
    PyObject *packages;     /* set: package names */
} WoWSImporter;

static void
WoWSImporter_dealloc(WoWSImporter *self)
{
    if (self->za) zip_close(self->za);
    free(self->zip_path);
    Py_XDECREF(self->index);
    Py_XDECREF(self->packages);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int
WoWSImporter_init(WoWSImporter *self, PyObject *args, PyObject *kwds)
{
    const char *zip_path;
    uint32_t i, n;

    if (!PyArg_ParseTuple(args, "s:WoWSImporter", &zip_path))
        return -1;

    self->zip_path = strdup(zip_path);
    self->index = PyDict_New();
    self->packages = PySet_New(NULL);
    if (!self->zip_path || !self->index || !self->packages)
        return -1;

    /* Open zip with C reader */
    self->za = zip_open(zip_path);
    if (self->za == NULL) {
        PyErr_Format(PyExc_IOError, "Cannot open %s", zip_path);
        return -1;
    }

    /* Build index from zip entries */
    n = zip_count(self->za);
    for (i = 0; i < n; i++) {
        const zip_entry_t *entry = zip_get(self->za, i);
        const char *name = entry->filename;
        size_t name_len = strlen(name);

        /* Only scripts/*.pyc */
        if (strncmp(name, "scripts/", 8) != 0) continue;
        if (name_len < 13) continue;  /* "scripts/X.pyc" minimum */
        if (strcmp(name + name_len - 4, ".pyc") != 0) continue;

        /* Convert: scripts/Foo/Bar.pyc → Foo.Bar */
        char mod_name[512];
        size_t mod_len = name_len - 8 - 4;
        if (mod_len >= sizeof(mod_name)) continue;

        memcpy(mod_name, name + 8, mod_len);
        mod_name[mod_len] = '\0';

        {
            char *p;
            for (p = mod_name; *p; p++)
                if (*p == '/') *p = '.';
        }

        PyObject *mod_key = PyString_FromString(mod_name);
        PyObject *zip_name = PyString_FromString(name);
        PyDict_SetItem(self->index, mod_key, zip_name);
        Py_DECREF(zip_name);

        /* Track packages (__init__) */
        if (mod_len > 9 && strcmp(mod_name + mod_len - 9, ".__init__") == 0) {
            mod_name[mod_len - 9] = '\0';
            PyObject *pkg = PyString_FromString(mod_name);
            PySet_Add(self->packages, pkg);
            Py_DECREF(pkg);
        }

        Py_DECREF(mod_key);
    }

    return 0;
}


/*
 * Check if "Parent.Leaf" refers to a top-level engine stub "Leaf"
 * imported as a submodule of zip package "Parent".
 * Returns a borrowed reference to the stub module, or NULL.
 */
static PyObject *
find_engine_submodule(WoWSImporter *self, const char *fullname)
{
    const char *dot = strrchr(fullname, '.');
    if (dot == NULL) return NULL;

    char parent_name[512];
    Py_ssize_t plen = dot - fullname;
    if (plen >= (Py_ssize_t)sizeof(parent_name)) return NULL;

    memcpy(parent_name, fullname, plen);
    parent_name[plen] = '\0';

    PyObject *pkey = PyString_FromString(parent_name);
    int is_our_pkg = pkey && PySet_Contains(self->packages, pkey);
    Py_XDECREF(pkey);
    if (!is_our_pkg) return NULL;

    const char *leaf = dot + 1;
    PyObject *existing = PyDict_GetItemString(PyImport_GetModuleDict(), leaf);
    if (existing != NULL && PyObject_HasAttrString(existing, "__wows_stub__"))
        return existing;  /* borrowed ref */

    return NULL;
}


/* ── find_module ─────────────────────────────────────────────────────── */

static PyObject *
WoWSImporter_find_module(PyObject *obj, PyObject *args)
{
    WoWSImporter *self = (WoWSImporter *)obj;
    char *fullname;
    PyObject *path = NULL;
    PyObject *key;

    if (!PyArg_ParseTuple(args, "s|O:find_module", &fullname, &path))
        return NULL;

    key = PyString_FromString(fullname);
    if (key == NULL) return NULL;

    if (PyDict_Contains(self->index, key) ||
        PySet_Contains(self->packages, key)) {
        Py_DECREF(key);
        Py_INCREF(obj);
        return obj;
    }

    Py_DECREF(key);

    /* Engine submodule fallback: "Sound.EnvironmentManager" → stub "EnvironmentManager" */
    if (find_engine_submodule(self, fullname) != NULL) {
        Py_INCREF(obj);
        return obj;
    }

    Py_RETURN_NONE;
}


/* ── load_module ─────────────────────────────────────────────────────── */

static PyObject *
WoWSImporter_load_module(PyObject *obj, PyObject *args)
{
    WoWSImporter *self = (WoWSImporter *)obj;
    char *fullname;
    PyObject *modules, *mod, *dict;
    PyObject *key = NULL, *zip_entry_name = NULL;
    PyObject *code = NULL;
    int is_package = 0;
    const char *zip_name;
    const zip_entry_t *entry;

    if (!PyArg_ParseTuple(args, "s:load_module", &fullname))
        return NULL;


    /* Already loaded? */
    modules = PyImport_GetModuleDict();
    mod = PyDict_GetItemString(modules, fullname);
    if (mod != NULL) {
        Py_INCREF(mod);
        return mod;
    }

    key = PyString_FromString(fullname);
    if (key == NULL) return NULL;

    /* Is this a package? */
    if (PySet_Contains(self->packages, key)) {
        is_package = 1;
        char init_name[512];
        snprintf(init_name, sizeof(init_name), "%s.__init__", fullname);
        PyObject *init_key = PyString_FromString(init_name);
        zip_entry_name = PyDict_GetItem(self->index, init_key);
        Py_XINCREF(zip_entry_name);
        Py_DECREF(init_key);
    } else {
        zip_entry_name = PyDict_GetItem(self->index, key);
        Py_XINCREF(zip_entry_name);
    }

    if (zip_entry_name == NULL) {
        /* Engine submodule fallback: re-register stub under dotted name */
        PyObject *existing = find_engine_submodule(self, fullname);
        if (existing != NULL) {
            const char *dot = strrchr(fullname, '.');
            Py_INCREF(existing);
            PyDict_SetItemString(modules, fullname, existing);

            /* Link to parent package */
            if (dot != NULL) {
                char parent_name[512];
                Py_ssize_t plen = dot - fullname;
                if (plen < (Py_ssize_t)sizeof(parent_name)) {
                    memcpy(parent_name, fullname, plen);
                    parent_name[plen] = '\0';
                    PyObject *parent = PyDict_GetItemString(modules, parent_name);
                    if (parent != NULL)
                        PyObject_SetAttrString(parent, dot + 1, existing);
                }
            }

            Py_DECREF(key);
            Py_INCREF(existing);
            return existing;
        }

        PyErr_Format(PyExc_ImportError, "No module named %s", fullname);
        Py_DECREF(key);
        return NULL;
    }

    zip_name = PyString_AS_STRING(zip_entry_name);

    /* Read raw .pyc data from zip using C reader */
    entry = zip_find(self->za, zip_name);
    if (entry == NULL) {
        PyErr_Format(PyExc_ImportError, "Entry %s not found in zip", zip_name);
        Py_DECREF(key);
        Py_DECREF(zip_entry_name);
        return NULL;
    }

    {
        size_t pyc_len;
        unsigned char *pyc_data = zip_read(self->za, entry, &pyc_len);
        if (pyc_data == NULL) {
            PyErr_Format(PyExc_ImportError, "Failed to read %s from zip", zip_name);
            Py_DECREF(key);
            Py_DECREF(zip_entry_name);
            return NULL;
        }

        /* Decrypt through 4-stage pipeline */
        code = wows_decrypt_pyc(pyc_data, (Py_ssize_t)pyc_len);
        free(pyc_data);
    }

    Py_DECREF(zip_entry_name);

    if (code == NULL) {
        Py_DECREF(key);
        return NULL;
    }

    /* Create module */
    mod = PyImport_AddModule(fullname);
    if (mod == NULL) {
        Py_DECREF(key);
        Py_DECREF(code);
        return NULL;
    }
    dict = PyModule_GetDict(mod);

    /* Set loader and marker */
    PyDict_SetItemString(dict, "__loader__", obj);
    PyDict_SetItemString(dict, "gCPLBx86",
                         PyString_FromString("1772650424"));

    /* Pre-inject gLostConnection — an Event() the engine sets on every
     * module. Without this, circular imports fail when a partially-loaded
     * module doesn't have gLostConnection yet. We cache one prototype
     * and Py_INCREF it for each module (they're independent Event objects
     * in the real engine, but a shared stub is fine for loading). */
    {
        static PyObject *shared_gLostConnection = NULL;
        if (shared_gLostConnection == NULL) {
            PyObject *event_mod = PyDict_GetItemString(modules, "Event");
            if (event_mod != NULL) {
                PyObject *event_cls = PyObject_GetAttrString(event_mod, "Event");
                if (event_cls != NULL) {
                    shared_gLostConnection = PyObject_CallObject(event_cls, NULL);
                    Py_DECREF(event_cls);
                } else {
                    PyErr_Clear();
                }
            }
        }
        if (shared_gLostConnection != NULL) {
            Py_INCREF(shared_gLostConnection);
            PyDict_SetItemString(dict, "gLostConnection", shared_gLostConnection);
        }
    }

    /* Since we use PyEval_EvalCode instead of PyImport_ExecCodeModuleEx,
     * we must set __builtins__ ourselves. The IMPORT_NAME opcode looks
     * up __import__ from f_builtins, which comes from globals["__builtins__"]. */
    if (PyDict_GetItemString(dict, "__builtins__") == NULL) {
        PyDict_SetItemString(dict, "__builtins__", PyEval_GetBuiltins());
    }

    /* __file__ */
    {
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s",
                 self->zip_path, zip_name);
        PyDict_SetItemString(dict, "__file__",
                             PyString_FromString(file_path));
    }

    /* Package setup */
    if (is_package) {
        char subdir[512];
        char pkg_path[1024];
        strncpy(subdir, fullname, sizeof(subdir) - 1);
        subdir[sizeof(subdir) - 1] = '\0';
        { char *p; for (p = subdir; *p; p++) if (*p == '.') *p = '/'; }

        snprintf(pkg_path, sizeof(pkg_path), "%s/scripts/%s",
                 self->zip_path, subdir);

        PyObject *plist = PyList_New(1);
        PyList_SET_ITEM(plist, 0, PyString_FromString(pkg_path));
        PyDict_SetItemString(dict, "__path__", plist);
        Py_DECREF(plist);
        PyDict_SetItemString(dict, "__package__",
                             PyString_FromString(fullname));
    } else {
        char *dot = strrchr(fullname, '.');
        if (dot)
            PyDict_SetItemString(dict, "__package__",
                PyString_FromStringAndSize(fullname, dot - fullname));
        else
            PyDict_SetItemString(dict, "__package__",
                                 PyString_FromString(""));
    }

    /* Execute code in module namespace.
     * We use PyEval_EvalCode directly (not PyImport_ExecCodeModuleEx)
     * so we control error handling and module cleanup ourselves. */
    {
        PyObject *v = PyEval_EvalCode((PyCodeObject *)code, dict, dict);
        Py_DECREF(code);
        Py_DECREF(key);

        /* The module's code may have replaced sys.modules[fullname]
         * with a different object (e.g. a lazy-loader wrapper, or
         * the deobfuscation layer swapping module objects). If so,
         * we must return the replacement — not the original. This
         * mirrors CPython's own import.c:import_submodule behaviour. */
        {
            PyObject *replaced = PyDict_GetItemString(modules, fullname);
            if (replaced != NULL && replaced != mod) {
                mod = replaced;
                dict = PyModule_GetDict(mod);  /* update dict pointer too */
            }
        }

        if (v != NULL) {
            Py_DECREF(v);


            /* Link submodule to parent package.
             * Standard Python importers do this — when X.Y is loaded,
             * set attr Y on module X so `from X import Y` works. */
            {
                char *dot = strrchr(fullname, '.');
                if (dot != NULL) {
                    char parent_name[512];
                    Py_ssize_t plen = dot - fullname;
                    if (plen < (Py_ssize_t)sizeof(parent_name)) {
                        memcpy(parent_name, fullname, plen);
                        parent_name[plen] = '\0';
                        PyObject *parent = PyDict_GetItemString(
                            modules, parent_name);
                        if (parent != NULL)
                            PyObject_SetAttrString(parent, dot + 1, mod);
                    }
                }
            }

            Py_INCREF(mod);
            return mod;
        }

        /* Execution failed — propagate the error.
         * Print the traceback for diagnosis, remove the broken module
         * from sys.modules, and return NULL so the caller gets a clean
         * ImportError instead of a zombie partially-initialized module. */
        {
            PyObject *exc_type, *exc_val, *exc_tb;
            PyErr_Fetch(&exc_type, &exc_val, &exc_tb);

            /* One-line summary for easy grep */
            if (exc_type && exc_val) {
                PyObject *type_name = PyObject_GetAttrString(exc_type, "__name__");
                PyObject *val_str = PyObject_Str(exc_val);
                fprintf(stderr, "WoWS: failed to exec %s: %s: %s\n",
                        fullname,
                        type_name ? PyString_AsString(type_name) : "?",
                        val_str ? PyString_AsString(val_str) : "?");
                Py_XDECREF(type_name);
                Py_XDECREF(val_str);
            } else {
                fprintf(stderr, "WoWS: failed to exec %s: (unknown error)\n",
                        fullname);
            }

            /* Restore and print full traceback */
            PyErr_Restore(exc_type, exc_val, exc_tb);
            PyErr_Print();  /* clears the error */

            /* Remove broken module from sys.modules — no zombies */
            PyDict_DelItemString(modules, fullname);
            PyErr_Clear();

            /* Return NULL with ImportError so caller gets a clean failure */
            PyErr_Format(PyExc_ImportError,
                         "WoWS: module '%s' failed to execute", fullname);
            return NULL;
        }
    }
}


/* ── get_code — decrypt and return code object without executing ────── */

static PyObject *
WoWSImporter_get_code(PyObject *obj, PyObject *args)
{
    WoWSImporter *self = (WoWSImporter *)obj;
    char *fullname;
    PyObject *key, *zip_entry_name;
    const char *zip_name;
    const zip_entry_t *entry;

    if (!PyArg_ParseTuple(args, "s:get_code", &fullname))
        return NULL;

    key = PyString_FromString(fullname);
    if (key == NULL) return NULL;

    /* Look up in index (try both module and package __init__) */
    if (PySet_Contains(self->packages, key)) {
        char init_name[512];
        snprintf(init_name, sizeof(init_name), "%s.__init__", fullname);
        PyObject *init_key = PyString_FromString(init_name);
        zip_entry_name = PyDict_GetItem(self->index, init_key);
        Py_XINCREF(zip_entry_name);
        Py_DECREF(init_key);
    } else {
        zip_entry_name = PyDict_GetItem(self->index, key);
        Py_XINCREF(zip_entry_name);
    }
    Py_DECREF(key);

    if (zip_entry_name == NULL) {
        PyErr_Format(PyExc_ImportError, "No module named %s", fullname);
        return NULL;
    }

    zip_name = PyString_AS_STRING(zip_entry_name);
    entry = zip_find(self->za, zip_name);
    if (entry == NULL) {
        PyErr_Format(PyExc_ImportError, "Entry %s not found in zip", zip_name);
        Py_DECREF(zip_entry_name);
        return NULL;
    }

    {
        size_t pyc_len;
        unsigned char *pyc_data = zip_read(self->za, entry, &pyc_len);
        Py_DECREF(zip_entry_name);
        if (pyc_data == NULL) {
            PyErr_SetString(PyExc_ImportError, "Failed to read from zip");
            return NULL;
        }
        PyObject *code = wows_decrypt_pyc(pyc_data, (Py_ssize_t)pyc_len);
        free(pyc_data);
        return code;  /* returns code object or NULL with error set */
    }
}


/* ── Type definition ─────────────────────────────────────────────────── */

static PyMethodDef WoWSImporter_methods[] = {
    {"find_module", WoWSImporter_find_module, METH_VARARGS, NULL},
    {"load_module", WoWSImporter_load_module, METH_VARARGS, NULL},
    {"get_code",    WoWSImporter_get_code,    METH_VARARGS,
     "get_code(fullname) -> code object. Decrypt without executing."},
    {NULL, NULL}
};

static PyTypeObject WoWSImporter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "wows_importer.WoWSImporter",
    sizeof(WoWSImporter),
    0,
    (destructor)WoWSImporter_dealloc,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    Py_TPFLAGS_DEFAULT,
    "WoWS scripts.zip importer",
    0, 0, 0, 0, 0, 0,
    WoWSImporter_methods,
    0, 0, 0, 0, 0, 0, 0,
    (initproc)WoWSImporter_init,
    PyType_GenericAlloc,
    PyType_GenericNew,
    PyObject_Del,
};


/* ── Public API ──────────────────────────────────────────────────────── */

int
wows_importer_install(const char *zip_path)
{
    PyObject *meta_path, *importer, *args;

    if (PyType_Ready(&WoWSImporter_Type) < 0)
        return -1;

    args = Py_BuildValue("(s)", zip_path);
    if (args == NULL) return -1;

    importer = PyObject_Call((PyObject *)&WoWSImporter_Type, args, NULL);
    Py_DECREF(args);
    if (importer == NULL) return -1;

    /* Report */
    fprintf(stderr, "WoWS importer: %s (%ld modules)\n",
            zip_path,
            (long)PyDict_Size(((WoWSImporter *)importer)->index));

    /* Insert at position 0 in sys.meta_path */
    meta_path = PySys_GetObject("meta_path");
    if (meta_path == NULL || !PyList_Check(meta_path)) {
        Py_DECREF(importer);
        PyErr_SetString(PyExc_RuntimeError, "sys.meta_path unavailable");
        return -1;
    }

    if (PyList_Insert(meta_path, 0, importer) < 0) {
        Py_DECREF(importer);
        return -1;
    }

    Py_DECREF(importer);
    return 0;
}
