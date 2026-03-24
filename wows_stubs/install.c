/*
 * wows_stubs/install.c — Register all engine stub modules.
 *
 * This is the only file that knows about all stubs. Each module's
 * init function is declared here and called in wows_stubs_install().
 */
#include "common.h"

/* Fake sys.getwindowsversion() — returns (6, 1, 7601, 2, "Service Pack 1") */
static PyObject *
fake_getwindowsversion(PyObject *self)
{
    return Py_BuildValue("(iiiss)", 6, 1, 7601, 2, "Service Pack 1");
}

/* ── Forward declarations for init functions ──────────────────────────── */

/* Major engine modules (separate .c files) */
PyObject *init_BigWorld(void);
PyObject *init_ResMgr(void);
PyObject *init_Math(void);
PyObject *init_Event(void);
PyObject *init_Lesta(void);

/* Smaller modules (modules.c) */
PyObject *init_GUI(void);
PyObject *init_Helpers(void);
PyObject *init_Helpers_i18n(void);
PyObject *init_Locale(void);
PyObject *init_WWISE(void);
PyObject *init_Windowed(void);
PyObject *init_EffectManager(void);
PyObject *init_EnvironmentManager(void);
PyObject *init_Settings(void);
PyObject *init_SceneGraph(void);
PyObject *init_UserPreferences(void);
/* UserPreferencesAPI loads from scripts.zip */
PyObject *init_VoiceoverSystem(void);
PyObject *init_Workarounds(void);
PyObject *init_EventQueuesManager(void);
PyObject *init_FlatSourcesManager(void);
PyObject *init_DepthOfField(void);
PyObject *init_SpatialUI(void);
PyObject *init_Scaleform(void);
PyObject *init_PreferenceSystem(void);
PyObject *init_GameParams_Defaults(void);
PyObject *init_Mathemagic(void);
PyObject *init_Camera(void);
PyObject *init_CameraUtils(void);
PyObject *init__sha3(void);
PyObject *init__crypto(void);
PyObject *init__counter(void);
PyObject *init__strxor(void);
PyObject *init__AES(void);
PyObject *init__galois(void);
PyObject *init__cpuid(void);
PyObject *init__AESNI(void);
PyObject *init__raw_aes(void);
PyObject *init__raw_aesni(void);
PyObject *init__websocket(void);
PyObject *init_util(void);
PyObject *init_util_UIMath(void);

/* Pure empty stubs */
PyObject *init_debug_utils(void);
PyObject *init_chat_shared(void);
PyObject *init_bwobsolete_helpers(void);
PyObject *init_account_helpers(void);
PyObject *init_items(void);
PyObject *init_nations(void);
PyObject *init_ArenaType(void);
PyObject *init_MapType(void);
PyObject *init_ClientArena(void);
PyObject *init_SoundGroups(void);
PyObject *init_Vivox(void);
PyObject *init_PixieBG(void);
PyObject *init_PostProcessing(void);
PyObject *init_ConnectionManager(void);
PyObject *init_predefined_hosts(void);
PyObject *init_WebBrowser(void);
PyObject *init_CollisionMath(void);
PyObject *init_ProfileParser(void);
PyObject *init_Core(void);
PyObject *init_Vary(void);
PyObject *init_CAT(void);
PyObject *init_Trails(void);
PyObject *init_server_constants(void);
PyObject *init_FlagsPreloader(void);
PyObject *init_PhysicsManager(void);
PyObject *init_StaticSceneMgr(void);
PyObject *init_WorldEventManager(void);
PyObject *init_WorldObjectManager(void);
PyObject *init_WaveSystem(void);
PyObject *init_ManyObjects(void);
PyObject *init_WeatherApi(void);
PyObject *init_Notification(void);
PyObject *init_msvcrt(void);
PyObject *init__subprocess(void);
PyObject *init__winreg(void);


/* ── Public API ───────────────────────────────────────────────────────── */

int
wows_stubs_install(void)
{
    PyObject *builtin_mod;

    /* Major engine modules */
    INIT_STUB(BigWorld);
    INIT_STUB(ResMgr);
    INIT_STUB(Math);
    INIT_STUB(GUI);
    INIT_STUB(Event);
    INIT_STUB(Lesta);

    /* Localization */
    INIT_STUB(Helpers);
    INIT_STUB(Helpers_i18n);
    INIT_STUB(Locale);

    /* Game infrastructure */
    INIT_STUB(debug_utils);
    INIT_STUB(chat_shared);
    INIT_STUB(bwobsolete_helpers);
    INIT_STUB(account_helpers);
    INIT_STUB(items);
    INIT_STUB(nations);
    INIT_STUB(ArenaType);
    INIT_STUB(MapType);
    INIT_STUB(ClientArena);

    /* Audio */
    INIT_STUB(SoundGroups);
    INIT_STUB(WWISE);
    INIT_STUB(Vivox);
    INIT_STUB(VoiceoverSystem);

    /* Graphics / UI */
    INIT_STUB(Settings);
    INIT_STUB(PixieBG);
    INIT_STUB(PostProcessing);
    INIT_STUB(Windowed);
    INIT_STUB(EffectManager);
    INIT_STUB(EnvironmentManager);
    INIT_STUB(SceneGraph);
    INIT_STUB(SpatialUI);
    INIT_STUB(Scaleform);
    INIT_STUB(DepthOfField);
    INIT_STUB(FlagsPreloader);
    INIT_STUB(Trails);

    /* Preferences */
    INIT_STUB(UserPreferences);
    /* UserPreferencesAPI loads from scripts.zip */
    INIT_STUB(PreferenceSystem);

    /* Networking / web */
    INIT_STUB(ConnectionManager);
    INIT_STUB(predefined_hosts);
    INIT_STUB(WebBrowser);

    /* Physics / scene */
    INIT_STUB(CollisionMath);
    INIT_STUB(PhysicsManager);
    INIT_STUB(StaticSceneMgr);
    INIT_STUB(WorldEventManager);
    INIT_STUB(WorldObjectManager);
    INIT_STUB(WaveSystem);
    INIT_STUB(ManyObjects);
    INIT_STUB(WeatherApi);

    /* Misc engine */
    INIT_STUB(ProfileParser);
    INIT_STUB(Core);
    INIT_STUB(Vary);
    INIT_STUB(CAT);
    INIT_STUB(GameParams_Defaults);
    INIT_STUB(server_constants);
    INIT_STUB(Camera);
    INIT_STUB(CameraUtils);
    INIT_STUB(Mathemagic);
    INIT_STUB(FlatSourcesManager);
    INIT_STUB(EventQueuesManager);
    INIT_STUB(Workarounds);
    INIT_STUB(Notification);
    INIT_STUB(msvcrt);
    INIT_STUB(_subprocess);
    INIT_STUB(_winreg);
    /* SSE.SSEClientUtils loads from scripts.zip */

    /* Crypto / hashing C extensions */
    INIT_STUB(_sha3);
    INIT_STUB(_crypto);
    INIT_STUB(_counter);
    INIT_STUB(_strxor);
    INIT_STUB(_AES);
    INIT_STUB(_galois);
    INIT_STUB(_cpuid);
    INIT_STUB(_AESNI);
    INIT_STUB(_raw_aes);
    INIT_STUB(_raw_aesni);
    INIT_STUB(_websocket);

    /* util package */
    INIT_STUB(util);
    {
        PyObject *uimath = init_util_UIMath();
        if (!uimath) goto err;
        PyObject *util_mod = PyImport_ImportModule("util");
        if (util_mod) {
            PyObject_SetAttrString(util_mod, "UIMath", uimath);
            Py_DECREF(util_mod);
        }
    }

    /*
     * Fake Windows environment for the game's scripts.
     * The game client is a Windows executable — scripts.zip contains
     * Windows-specific code (ctypes with windll/kernel32, os.name checks).
     * We fake the platform so obfuscation guards and platform branches
     * evaluate as they would in the real game client.
     */
    {
        PyObject *os_mod = PyImport_ImportModule("os");
        PyObject *sys_mod = PyImport_ImportModule("sys");
        if (os_mod) {
            PyObject_SetAttrString(os_mod, "name",
                                   PyString_FromString("nt"));
            Py_DECREF(os_mod);
        }
        if (sys_mod) {
            PyObject_SetAttrString(sys_mod, "platform",
                                   PyString_FromString("win32"));
            PyObject_SetAttrString(sys_mod, "dllhandle",
                                   PyInt_FromLong(0));
            /* sys.getwindowsversion() */
            {
                static PyMethodDef gwv_def = {
                    "getwindowsversion", (PyCFunction)fake_getwindowsversion,
                    METH_NOARGS, NULL
                };
                PyObject *fn = PyCFunction_NewEx(&gwv_def, NULL, NULL);
                if (fn) {
                    PyObject_SetAttrString(sys_mod, "getwindowsversion", fn);
                    Py_DECREF(fn);
                }
            }
            Py_DECREF(sys_mod);
        }
        /* WindowsError — alias to OSError */
        {
            PyObject *bi = PyImport_ImportModule("__builtin__");
            if (bi) {
                PyObject_SetAttrString(bi, "WindowsError", PyExc_OSError);
                Py_DECREF(bi);
            } else {
                PyErr_Clear();
            }
        }
        /* Patch _ctypes with Windows-only symbols */
        {
            static PyMethodDef win_ctypes_methods[] = {
                {"FormatError",    (PyCFunction)nop_str, METH_VARARGS, NULL},
                {"LoadLibrary",    (PyCFunction)nop_int, METH_VARARGS, NULL},
                {"_check_HRESULT", (PyCFunction)nop_int, METH_VARARGS, NULL},
                {"get_last_error", (PyCFunction)nop_int, METH_VARARGS, NULL},
                {"set_last_error", (PyCFunction)nop_int, METH_VARARGS, NULL},
                {NULL}
            };
            PyObject *m = PyImport_ImportModule("_ctypes");
            if (m) {
                PyMethodDef *md;
                if (!PyObject_HasAttrString(m, "FUNCFLAG_STDCALL"))
                    PyModule_AddIntConstant(m, "FUNCFLAG_STDCALL", 0);
                for (md = win_ctypes_methods; md->ml_name; md++) {
                    if (!PyObject_HasAttrString(m, md->ml_name)) {
                        PyObject *fn = PyCFunction_NewEx(md, NULL, NULL);
                        if (fn) {
                            PyObject_SetAttrString(m, md->ml_name, fn);
                            Py_DECREF(fn);
                        }
                    }
                }
                Py_DECREF(m);
            } else {
                PyErr_Clear();
            }
        }
    }

    /* Patch _socket.socket type with Windows-only 'ioctl' method.
     * The zip's socket.py is the Windows build and accesses this. */
    {
        PyObject *m = PyImport_ImportModule("_socket");
        if (m) {
            PyObject *sock_type = PyObject_GetAttrString(m, "socket");
            if (sock_type && PyType_Check(sock_type)) {
                PyTypeObject *tp = (PyTypeObject *)sock_type;
                if (tp->tp_dict == NULL)
                    PyType_Ready(tp);
                if (tp->tp_dict && !PyDict_GetItemString(tp->tp_dict, "ioctl")) {
                    static PyMethodDef ioctl_def = {
                        "ioctl", (PyCFunction)nop_int, METH_VARARGS, NULL
                    };
                    PyObject *descr = PyDescr_NewMethod(tp, &ioctl_def);
                    if (descr) {
                        PyDict_SetItemString(tp->tp_dict, "ioctl", descr);
                        Py_DECREF(descr);
                        PyType_Modified(tp);
                    } else {
                        PyErr_Clear();
                    }
                }
                Py_DECREF(sock_type);
            } else {
                PyErr_Clear();
            }
            /* Windows socket constants */
            if (!PyObject_HasAttrString(m, "SIO_RCVALL"))
                PyModule_AddIntConstant(m, "SIO_RCVALL", 0x98000001);
            if (!PyObject_HasAttrString(m, "SIO_KEEPALIVE_VALS"))
                PyModule_AddIntConstant(m, "SIO_KEEPALIVE_VALS", 0x98000004);
            if (!PyObject_HasAttrString(m, "RCVALL_ON"))
                PyModule_AddIntConstant(m, "RCVALL_ON", 1);
            if (!PyObject_HasAttrString(m, "RCVALL_OFF"))
                PyModule_AddIntConstant(m, "RCVALL_OFF", 0);
            Py_DECREF(m);
        } else {
            PyErr_Clear();
        }
    }

    /* Patch _ssl with Windows-only functions */
    {
        static PyMethodDef win_ssl_methods[] = {
            {"enum_certificates", (PyCFunction)nop_list, METH_VARARGS, NULL},
            {"enum_crls",         (PyCFunction)nop_list, METH_VARARGS, NULL},
            {NULL}
        };
        PyObject *m = PyImport_ImportModule("_ssl");
        if (m) {
            PyMethodDef *md;
            for (md = win_ssl_methods; md->ml_name; md++) {
                if (!PyObject_HasAttrString(m, md->ml_name)) {
                    PyObject *fn = PyCFunction_NewEx(md, NULL, NULL);
                    if (fn) {
                        PyObject_SetAttrString(m, md->ml_name, fn);
                        Py_DECREF(fn);
                    }
                }
            }
            Py_DECREF(m);
        } else {
            PyErr_Clear();
        }
    }

    /* DO NOT preload stdlib modules here. The goal is a standalone binary
     * with no system Python dependency. All modules in scripts.zip are
     * the Windows game build — fix platform differences via stubs/mocks
     * in the Windows environment setup above, not by preloading stdlib. */

    /* Add _NamedConstantsType and gCPLBx86 to __builtin__ */
    builtin_mod = PyImport_ImportModule("__builtin__");
    if (builtin_mod != NULL) {
        PyObject_SetAttrString(builtin_mod, "_NamedConstantsType",
                               get_flex_base_type());
        PyObject_SetAttrString(builtin_mod, "gCPLBx86",
                               PyString_FromString("1772650424"));
        Py_DECREF(builtin_mod);
    }

    return 0;

err:
    if (PyErr_Occurred()) {
        fprintf(stderr, "wows_stubs_install failed:\n");
        PyErr_Print();
    }
    return -1;
}
