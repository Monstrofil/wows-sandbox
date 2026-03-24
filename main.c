/*
 * main.c — WoWS Shell: embedded Python 2.7 with WoWS module loader.
 *
 * Usage:
 *   ./wows_shell                  — Interactive REPL with game modules
 *   ./wows_shell script.py        — Execute script with game modules
 *   ./wows_shell -c "code"        — Execute code string
 */
#include "Python.h"
#include "wows_stubs/wows_stubs.h"
#include "wows_importer/wows_importer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * Dummy Windows API symbols. The game's ctypes (from scripts.zip) is the
 * Windows build and does ctypes.windll.kernel32.GetLastError etc. On Linux,
 * CDLL(None) resolves symbols from the main binary via dlsym. We export
 * these dummies so the resolution succeeds.
 */
int GetLastError(void) { return 0; }
void SetLastError(int e) { (void)e; }
int FormatMessageW(void) { return 0; }

/*
 * Replace sys.path so that:
 *  - Pure Python modules come from scripts.zip (via our meta_path importer)
 *  - C extension modules are static builtins (no lib-dynload needed)
 *
 * We set sys.path to an empty list and clear sys.path_hooks and
 * sys.path_importer_cache so the old filesystem importers don't interfere.
 */
static int
clear_filesystem_imports(const char *argv0)
{
    PyObject *sys_mod, *new_path, *empty;

    sys_mod = PyImport_ImportModule("sys");
    if (sys_mod == NULL)
        return -1;

    /* Set sys.path to only contain helpers/ directory (relative to binary).
     * All C extensions are static builtins, game modules come from zip. */
    {
        char helpers_dir[4096];
        const char *slash;
        strncpy(helpers_dir, argv0, sizeof(helpers_dir) - 40);
        helpers_dir[sizeof(helpers_dir) - 40] = '\0';
        slash = strrchr(helpers_dir, '/');
        if (slash != NULL)
            strcpy((char *)slash + 1, "helpers");
        else
            strcpy(helpers_dir, "helpers");
        new_path = PyList_New(1);
        PyList_SET_ITEM(new_path, 0, PyString_FromString(helpers_dir));
    }

    if (PyObject_SetAttrString(sys_mod, "path", new_path) < 0) {
        Py_DECREF(new_path); Py_DECREF(sys_mod); return -1;
    }
    Py_DECREF(new_path);

    /* sys.path_hooks = [] */
    empty = PyList_New(0);
    if (empty == NULL) { Py_DECREF(sys_mod); return -1; }
    if (PyObject_SetAttrString(sys_mod, "path_hooks", empty) < 0) {
        Py_DECREF(empty); Py_DECREF(sys_mod); return -1;
    }
    Py_DECREF(empty);

    /* sys.path_importer_cache = {} */
    empty = PyDict_New();
    if (empty == NULL) { Py_DECREF(sys_mod); return -1; }
    if (PyObject_SetAttrString(sys_mod, "path_importer_cache", empty) < 0) {
        Py_DECREF(empty); Py_DECREF(sys_mod); return -1;
    }
    Py_DECREF(empty);

    Py_DECREF(sys_mod);
    return 0;
}

/* Default path to scripts.zip (same directory as binary) */
static char *
find_scripts_zip(const char *argv0)
{
    static char path[4096];
    char *slash;

    /* Try data/ relative to the binary */
    strncpy(path, argv0, sizeof(path) - 30);
    path[sizeof(path) - 30] = '\0';
    slash = strrchr(path, '/');
    if (slash != NULL) {
        strcpy(slash + 1, "data/scripts.zip");
    } else {
        strcpy(path, "data/scripts.zip");
    }

    if (access(path, R_OK) == 0)
        return path;

    /* Try data/ in current directory */
    strcpy(path, "data/scripts.zip");
    if (access(path, R_OK) == 0)
        return path;

    /* Legacy: scripts.zip in current directory */
    strcpy(path, "scripts.zip");
    if (access(path, R_OK) == 0)
        return path;

    return NULL;
}


int
main(int argc, char **argv)
{
    const char *zip_path;
    const char *script_path = NULL;
    const char *code_string = NULL;
    int i;

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            code_string = argv[++i];
        } else if (strcmp(argv[i], "--zip") == 0 && i + 1 < argc) {
            /* Allow overriding scripts.zip path */
            i++; /* handled below */
        } else if (argv[i][0] != '-') {
            script_path = argv[i];
            break;
        }
    }

    /* Find scripts.zip */
    zip_path = NULL;
    for (i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--zip") == 0) {
            zip_path = argv[i + 1];
            break;
        }
    }
    if (zip_path == NULL) {
        zip_path = find_scripts_zip(argv[0]);
    }

    /* Initialize Python.
     * Set PYTHONHOME to 3rdparty/cpython/Lib so encodings and other
     * bootstrap modules are found. Py_NoSiteFlag skips site.py. */
    {
        static char pylib[4096];
        const char *slash;
        strncpy(pylib, argv[0], sizeof(pylib) - 50);
        pylib[sizeof(pylib) - 50] = '\0';
        slash = strrchr(pylib, '/');
        if (slash != NULL)
            strcpy((char *)slash + 1, "3rdparty/cpython");
        else
            strcpy(pylib, "3rdparty/cpython");
        Py_SetPythonHome(pylib);
        Py_NoSiteFlag = 1;
    }
    Py_SetProgramName(argv[0]);
    Py_Initialize();
    PySys_SetArgvEx(argc, argv, 0);

    /* GC is safe — stub classes override tp_new to untrack all instances
     * (see _stub_tp_new in common.h), preventing GC from traversing
     * deep FlexBase reference chains that cause stack overflow. */

    /* Install stubs first (before any imports) */
    if (wows_stubs_install() < 0) {
        fprintf(stderr, "Failed to install WoWS stubs\n");
        if (PyErr_Occurred()) PyErr_Print();
        Py_Finalize();
        return 1;
    }

    /* Install WoWS importer */
    if (zip_path == NULL) {
        fprintf(stderr, "Warning: scripts.zip not found. "
                "Use --zip <path> or place it next to the binary.\n");
    } else {
        if (wows_importer_install(zip_path) < 0) {
            fprintf(stderr, "Failed to install WoWS importer for %s\n",
                    zip_path);
            if (PyErr_Occurred()) PyErr_Print();
            Py_Finalize();
            return 1;
        }
    }

    /* Clear filesystem import paths so all future imports use scripts.zip */
    if (zip_path != NULL) {
        if (clear_filesystem_imports(argv[0]) < 0) {
            fprintf(stderr, "Failed to clear filesystem import paths\n");
            if (PyErr_Occurred()) PyErr_Print();
            Py_Finalize();
            return 1;
        }
    }

    /* Try to import BWPersonality */
    if (zip_path != NULL) {
        PyObject *bwp = PyImport_ImportModule("BWPersonality");
        if (bwp != NULL) {
            fprintf(stderr, "BWPersonality loaded.\n");
            Py_DECREF(bwp);
        } else {
            fprintf(stderr, "Warning: BWPersonality import failed:\n");
            PyErr_Print();
            PyErr_Clear();
        }
    }

    /* Simulate the engine boot: call BigWorld.loadPreferences() and store
     * the result in ModsShell.gPrefs.  In the real engine, this is done
     * by the C++ startup code before any Python mod code runs. */
    {
        PyObject *bw = PyImport_ImportModule("BigWorld");
        if (bw) {
            PyObject *prefs = PyObject_CallMethod(bw, "loadPreferences", NULL);
            if (prefs && prefs != Py_None) {
                PyObject *ms = PyImport_ImportModule("ModsShell");
                if (ms) {
                    PyObject_SetAttrString(ms, "gPrefs", prefs);
                    Py_DECREF(ms);
                } else {
                    PyErr_Clear();
                }
            }
            Py_XDECREF(prefs);
            Py_DECREF(bw);
        } else {
            PyErr_Clear();
        }
    }


    /* Load helpers — import wows_helpers and call setup() to inject
     * find_module/find_class/etc. into __builtin__. The helpers/
     * directory was added to sys.path in clear_filesystem_imports(). */
    PyRun_SimpleString(
        "try:\n"
        "    import wows_helpers; wows_helpers.setup()\n"
        "except ImportError: pass\n"
    );

    /* Execute mode */
    if (code_string != NULL) {
        /* -c mode */
        int result = PyRun_SimpleString(code_string);
        Py_Finalize();
        return result != 0 ? 1 : 0;
    } else if (script_path != NULL) {
        /* Script mode */
        FILE *fp = fopen(script_path, "r");
        if (fp == NULL) {
            perror(script_path);
            Py_Finalize();
            return 1;
        }
        int result = PyRun_SimpleFileExFlags(fp, script_path, 1, NULL);
        Py_Finalize();
        return result != 0 ? 1 : 0;
    } else {
        /* REPL mode */
        fprintf(stderr, "WoWS Shell - Python 2.7 with game module support\n");
        fprintf(stderr, "Helpers:\n");
        fprintf(stderr, "  find_module(pattern)  - search loaded modules by name\n");
        fprintf(stderr, "  find_class(name)      - find a class across all modules\n");
        fprintf(stderr, "  find_attr(name)       - find any attribute by name\n");
        fprintf(stderr, "  find_func(name)       - find callable by name\n");
        fprintf(stderr, "  dump(obj)             - pretty-print object attributes\n");
        fprintf(stderr, "  grep_modules(pattern) - search co_names in loaded modules\n");
        fprintf(stderr, "\n");
        PyRun_InteractiveLoopFlags(stdin, "<stdin>", NULL);
        Py_Finalize();
        return 0;
    }
}
