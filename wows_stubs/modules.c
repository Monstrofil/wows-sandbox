/*
 * wows_stubs/modules.c — Smaller engine module stubs.
 *
 * Modules that need only a few methods or just an empty presence
 * in sys.modules are defined here using STUB_MODULE macros.
 */
#include "common.h"

/* ── GUI ──────────────────────────────────────────────────────────────── */

static PyMethodDef GUI_methods[] = {
    NOP_VARARGS(addRoot),  NOP_VARARGS(delRoot),
    NOP_VARARGS(roots),    NOP_VARARGS(screenResolution),
    NOP_VARARGS(Simple),   NOP_VARARGS(Text),
    NOP_VARARGS(Window),   END
};
STUB_MODULE_METHODS(GUI, "GUI", GUI_methods)

/* ── Helpers / i18n ───────────────────────────────────────────────────── */

STUB_MODULE(Helpers, "Helpers")

static PyObject *
i18n_makeString(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *key = "";
    if (!PyArg_ParseTuple(args, "|s", &key)) {
        PyErr_Clear();
        Py_RETURN_NONE;
    }
    return PyString_FromString(key);
}
static PyMethodDef Helpers_i18n_methods[] = {
    {"makeString",      (PyCFunction)i18n_makeString, METH_VARARGS | METH_KEYWORDS, NULL},
    {"makePluralString",(PyCFunction)i18n_makeString, METH_VARARGS | METH_KEYWORDS, NULL},
    {"convert",         (PyCFunction)i18n_makeString, METH_VARARGS | METH_KEYWORDS, NULL},
    END
};
STUB_MODULE_METHODS(Helpers_i18n, "Helpers.i18n", Helpers_i18n_methods)

/* ── Locale ───────────────────────────────────────────────────────────── */

static PyMethodDef Locale_methods[] = {
    NOP_VARARGS(getLanguage),
    END
};
PyObject *init_Locale(void) {
    PyObject *m = make_module("Locale", Locale_methods, "Locale stub");
    if (m) {
        PyObject *d = PyDict_New();
        PyObject *bi = PyImport_ImportModule("__builtin__");
        PyDict_SetItemString(d, "__builtins__", bi);
        Py_XDECREF(bi);
        PyRun_String(
            "class _TranslatorObj(object):\n"
            "    '''translate() returns locale strings, translatePlural() returns numeric strings.'''\n"
            "    def translate(self, key, *a, **kw):\n"
            "        if isinstance(key, str) and key.startswith('IDS_'):\n"
            "            return '{}'  # empty JSON dict\n"
            "        return key\n"
            "    def translatePlural(self, key, n=0, *a, **kw):\n"
            "        return '0'  # numeric string for int()\n"
            "class _Translator(object):\n"
            "    '''Locale.translator — callable that returns a translator object.'''\n"
            "    def __call__(self, *a, **kw): return _TranslatorObj()\n"
            "_translator = _Translator()\n",
            Py_file_input, d, d);
        /* Untrack both classes and the dict from GC */
        gc_untrack_class_and_dict(PyDict_GetItemString(d, "_TranslatorObj"), d);
        gc_untrack_class_and_dict(PyDict_GetItemString(d, "_Translator"), NULL);
        PyObject *inst = PyDict_GetItemString(d, "_translator");
        if (inst) {
            Py_INCREF(inst);
            PyModule_AddObject(m, "translator", inst);
        }
    }
    return m;
}

/* ── WWISE (audio engine) ─────────────────────────────────────────────── */

static PyMethodDef WWISE_methods[] = {
    NOP_VARARGS(createSource),  NOP_VARARGS(destroySource),
    NOP_VARARGS(postEvent),     NOP_VARARGS(setState),
    NOP_VARARGS(setSwitch),     NOP_VARARGS(setRTPC),
    END
};
STUB_MODULE_METHODS(WWISE, "WWISE", WWISE_methods)

/* ── Windowed ─────────────────────────────────────────────────────────── */

static PyMethodDef Windowed_methods[] = { NOP_FALSE(isVideoWindowed), END };
STUB_MODULE_METHODS(Windowed, "Windowed", Windowed_methods)

/* ── EffectManager ────────────────────────────────────────────────────── */

static PyMethodDef EffectManager_methods[] = {
    NOP_VARARGS(setIntensity),     NOP_VARARGS(createOnPosition),
    NOP_VARARGS(createOnNode),     NOP_VARARGS(createOnModel),
    NOP_VARARGS(setLocalMatrix),   NOP_VARARGS(destroy),
    NOP_VARARGS(listSetAlphaMultiplier), END
};
STUB_MODULE_METHODS(EffectManager, "EffectManager", EffectManager_methods)

/* ── EnvironmentManager ───────────────────────────────────────────────── */

static PyMethodDef EnvironmentManager_methods[] = {
    NOP_VARARGS(selectForcedModification),  NOP_VARARGS(deselectForcedModification),
    NOP_VARARGS(selectModification),        NOP_VARARGS(deselectModification),
    NOP_VARARGS(selectLanguage),            NOP_VARARGS(setState),
    NOP_VARARGS(getLanguagesNames),         NOP_VARARGS(getModificationsNames),
    NOP_VARARGS(createSource),              NOP_STR(format),
    END
};
PyObject *init_EnvironmentManager(void) {
    PyObject *m = make_module("EnvironmentManager", EnvironmentManager_methods,
                              "EnvironmentManager stub");
    if (m) PyModule_AddObject(m, "EnvironmentManager", get_flex_base_type());
    return m;
}

/* ── Settings ─────────────────────────────────────────────────────────── */

PyObject *init_Settings(void) {
    PyObject *m = make_module("Settings", _empty_methods, "Settings stub");
    if (m) {
        Py_INCREF(&PyBaseObject_Type);
        PyModule_AddObject(m, "GraphicsSettingCallback", (PyObject *)&PyBaseObject_Type);
    }
    return m;
}

/* ── SceneGraph ───────────────────────────────────────────────────────── */

PyObject *init_SceneGraph(void) {
    PyObject *m = make_module("SceneGraph", _empty_methods, "SceneGraph stub");
    if (m) PyModule_AddObject(m, "PrimInstDrawer", get_flex_base_type());
    return m;
}

/* ── UserPreferences ──────────────────────────────────────────────────── */

static PyMethodDef UserPreferences_methods[] = {
    NOP_VARARGS(upGetAttribute), NOP_VARARGS(upSetAttribute), END
};
PyObject *init_UserPreferences(void) {
    PyObject *m = make_module("UserPreferences", UserPreferences_methods,
                              "UserPreferences stub");
    if (m) {
        PyModule_AddObject(m, "PyItem", get_flex_base_type());
        static const char *sections[] = {
            "sound", "graphics", "game", "controls",
            "scriptsPreferences", "chat", NULL
        };
        const char **p;
        for (p = sections; *p; p++) {
            PyObject *cls = get_flex_base_type();
            if (cls) {
                PyObject *inst = PyObject_CallObject(cls, NULL);
                if (inst) PyModule_AddObject(m, *p, inst);
                Py_DECREF(cls);
            }
        }
    }
    return m;
}

/* UserPreferencesAPI — loads from scripts.zip, no stub needed */

/* ── VoiceoverSystem ──────────────────────────────────────────────────── */

static PyMethodDef VoiceoverSystem_methods[] = {
    NOP_VARARGS(setValidCrewNames), END
};
STUB_MODULE_METHODS(VoiceoverSystem, "VoiceoverSystem", VoiceoverSystem_methods)

/* ── Workarounds ──────────────────────────────────────────────────────── */

static PyMethodDef Workarounds_methods[] = {
    NOP_VARARGS(setMusicGameState), END
};
STUB_MODULE_METHODS(Workarounds, "Workarounds", Workarounds_methods)

/* ── EventQueuesManager ───────────────────────────────────────────────── */

static PyMethodDef EventQueuesManager_methods[] = {
    NOP_VARARGS(createQueue), NOP_VARARGS(destroyQueue), END
};
STUB_MODULE_METHODS(EventQueuesManager, "EventQueuesManager", EventQueuesManager_methods)

/* ── FlatSourcesManager ───────────────────────────────────────────────── */

static PyMethodDef FlatSourcesManager_methods[] = {
    NOP_VARARGS(createSource), NOP_VARARGS(destroySource), END
};
STUB_MODULE_METHODS(FlatSourcesManager, "FlatSourcesManager", FlatSourcesManager_methods)

/* ── DepthOfField ─────────────────────────────────────────────────────── */

static PyMethodDef DepthOfField_methods[] = { NOP_DICT(getAllProperties), END };
PyObject *init_DepthOfField(void) {
    PyObject *m = make_module("DepthOfField", DepthOfField_methods, "DepthOfField stub");
    if (m) {
        PyObject *cls = get_flex_base_type();
        if (cls) {
            PyObject *inst = PyObject_CallObject(cls, NULL);
            if (inst) PyModule_AddObject(m, "DEFAULT_DOF", inst);
            Py_DECREF(cls);
        }
    }
    return m;
}

/* ── SpatialUI ────────────────────────────────────────────────────────── */

static PyMethodDef SpatialUI_methods[] = {
    NOP_VARARGS(taSetParams), NOP_VARARGS(taSet),
    NOP_VARARGS(taRemove),    NOP_VARARGS(taToggle),
    NOP_VARARGS(taSetColor),  NOP_VARARGS(taSetPosition),
    NOP_VARARGS(taSetVisible),NOP_INT(taCreate),
    NOP_VARARGS(taSetTransform), NOP_VARARGS(taSetShipHighlighterIndex),
    END
};
PyObject *init_SpatialUI(void) {
    PyObject *m = make_module("SpatialUI", SpatialUI_methods, "SpatialUI stub");
    if (m) {
        add_int(m, "DT_BIT_ENABLE", 1);
        add_int(m, "DT_BIT_DISABLE", 0);
        add_int(m, "DT_BIT_INSIDE_BOX", 2);

        /* Params class */
        {
            PyObject *d = PyDict_New();
            PyObject *bi = PyImport_ImportModule("__builtin__");
            PyDict_SetItemString(d, "__builtins__", bi);
            Py_XDECREF(bi);
            PyRun_String(
                "class Params(object):\n"
                "    def __init__(self, **kw):\n"
                "        self.__dict__.update(kw)\n"
                "    def __getattr__(self, n):\n"
                "        if n.startswith('_'): raise AttributeError(n)\n"
                "        return 0\n",
                Py_file_input, d, d);
            PyObject *cls = PyDict_GetItemString(d, "Params");
            gc_untrack_class_and_dict(cls, d);
            if (cls) {
                Py_INCREF(cls);
                PyModule_AddObject(m, "Params", cls);
            }
        }

        /* Drawing types */
        static const char *types[] = {
            "Lines", "LDR", "CurvedQuad", "LineStrip", "Quad",
            "SectorOutline", "Circle", "Arc", "Polygon", "Text", "Icon",
            NULL
        };
        const char **p;
        for (p = types; *p; p++)
            PyModule_AddObject(m, *p, get_flex_base_type());
    }
    return m;
}

/* ── _Scaleform ───────────────────────────────────────────────────────── */

static PyMethodDef Scaleform_methods[] = { NOP_VARARGS(setFps), END };
PyObject *init_Scaleform(void) {
    PyObject *m = make_module("_Scaleform", Scaleform_methods, "_Scaleform stub");
    if (m) add_int(m, "SCALEFORM_FPS", 60);
    return m;
}

/* ── PreferenceSystem ─────────────────────────────────────────────────── */

PyObject *init_PreferenceSystem(void) {
    PyObject *m = make_module("PreferenceSystem", _empty_methods, "PreferenceSystem stub");
    if (m) {
        PyModule_AddObject(m, "PyItem", get_flex_base_type());
        PyObject *d = PyDict_New();
        PyObject *bi = PyImport_ImportModule("__builtin__");
        PyDict_SetItemString(d, "__builtins__", bi);
        Py_XDECREF(bi);
        PyRun_String(
            "class _DataHub(object):\n"
            "    def children(self): return []\n"
            "    def __getattr__(self, n):\n"
            "        if n.startswith('_'): raise AttributeError(n)\n"
            "        return self\n"
            "    def __call__(self, *a, **kw): return self\n"
            "    def __getitem__(self, k): return self\n"
            "    def __setitem__(self, k, v): pass\n"
            "    def __contains__(self, k): return False\n"
            "    def __iter__(self): return iter([])\n"
            "    def __len__(self): return 0\n"
            "    def keys(self): return []\n"
            "    def values(self): return []\n"
            "    def items(self): return []\n"
            "    def get(self, k, d=None): return d\n"
            "    def __nonzero__(self): return True\n"
            "    def __str__(self): return ''\n",
            Py_file_input, d, d);
        PyObject *cls = PyDict_GetItemString(d, "_DataHub");
        gc_untrack_class_and_dict(cls, d);
        if (cls) {
            static const char *hubs[] = {
                "dataHub", "direct", "classID", "event", "applyPending", NULL
            };
            const char **n;
            for (n = hubs; *n; n++) {
                PyObject *inst = PyObject_CallObject(cls, NULL);
                if (inst) PyModule_AddObject(m, *n, inst);
            }
        }
    }
    return m;
}

/* ── GameParams_Defaults ──────────────────────────────────────────────── */

PyObject *init_GameParams_Defaults(void) {
    PyObject *m = make_module("GameParams_Defaults", _empty_methods,
                              "GameParams_Defaults stub");
    if (m == NULL) return NULL;

    PyObject *d = PyDict_New();
    PyObject *builtins = PyImport_ImportModule("__builtin__");
    PyDict_SetItemString(d, "__builtins__", builtins);
    Py_XDECREF(builtins);
    PyRun_String(
        "import sys as _sys\n"
        "class _ModifierParams(object):\n"
        "    def __getattr__(self, name):\n"
        "        if name.startswith('_'): raise AttributeError(name)\n"
        "        return 0\n"
        "    def __dir__(self):\n"
        "        mvt_mod = _sys.modules.get('Modifiers.ModifierValueType')\n"
        "        if mvt_mod is not None:\n"
        "            MVT = getattr(mvt_mod, 'ModifierValueType', None)\n"
        "            if MVT is not None:\n"
        "                return [n for n in dir(MVT) if not n.startswith('_')]\n"
        "        return []\n"
        "    def __contains__(self, item): return True\n"
        "    def __iter__(self): return iter(self.__dir__())\n",
        Py_file_input, d, d);
    PyObject *cls = PyDict_GetItemString(d, "_ModifierParams");
    gc_untrack_class_and_dict(cls, d);
    if (cls != NULL) {
        PyObject *instance = PyObject_CallObject(cls, NULL);
        if (instance != NULL)
            PyModule_AddObject(m, "ModifierParamsList", instance);
    }
    return m;
}

/* ── Mathemagic ───────────────────────────────────────────────────────── */

static PyMethodDef Mathemagic_methods[] = {
    NOP_VARARGS(slerp),              NOP_VARARGS(getWorldRayAndPoint),
    NOP_VARARGS(getDirectionFromYaw),NOP_VARARGS(clamp),
    NOP_VARARGS(lerp),               END
};
PyObject *init_Mathemagic(void) {
    PyObject *m = make_module("Mathemagic", Mathemagic_methods, "Mathemagic stub");
    if (m) {
        PyModule_AddObject(m, "UP_VECTOR", Py_None);
        Py_INCREF(Py_None);
    }
    return m;
}

/* ── Camera ───────────────────────────────────────────────────────────── */

STUB_MODULE(Camera, "Camera")
static PyMethodDef CameraUtils_methods[] = {
    NOP_VARARGS(getWorldRayAndPoint), END
};
STUB_MODULE_METHODS(CameraUtils, "Camera.CameraUtils", CameraUtils_methods)

/* SSE.SSEClientUtils — loads from scripts.zip, no stub needed */

/* ── Crypto / SHA3 C extensions ───────────────────────────────────────── */

static PyMethodDef _sha3_methods[] = {
    NOP_VARARGS(keccak_512), NOP_VARARGS(keccak_256),
    NOP_VARARGS(sha3_512),   NOP_VARARGS(sha3_256),
    END
};
PyObject *init__sha3(void) {
    PyObject *m = make_module("_sha3", _sha3_methods, "_sha3 stub");
    if (m) {
        static const char *hashes[] = {
            "keccak_512", "keccak_256", "keccak_384", "keccak_224",
            "sha3_512", "sha3_384", "sha3_256", "sha3_224",
            "shake_128", "shake_256", NULL
        };
        const char **p;
        for (p = hashes; *p; p++)
            PyModule_AddObject(m, *p, get_flex_base_type());
    }
    return m;
}

STUB_MODULE(_crypto, "_crypto")
STUB_MODULE(_counter, "_counter")
static PyMethodDef _strxor_methods[] = {
    NOP_VARARGS(strxor), NOP_VARARGS(strxor_c), END
};
STUB_MODULE_METHODS(_strxor, "_strxor", _strxor_methods)
STUB_MODULE(_AES, "_AES")
STUB_MODULE(_galois, "_galois")
static PyMethodDef _cpuid_methods[] = {
    NOP_FALSE(have_aes_ni), NOP_FALSE(have_clmul), END
};
STUB_MODULE_METHODS(_cpuid, "_cpuid", _cpuid_methods)
STUB_MODULE(_AESNI, "_AESNI")
STUB_MODULE(_raw_aes, "_raw_aes")
STUB_MODULE(_raw_aesni, "_raw_aesni")
STUB_MODULE(_websocket, "_websocket")

/* ── util package ─────────────────────────────────────────────────────── */

STUB_MODULE(util, "util")
static PyMethodDef util_UIMath_methods[] = {
    NOP_VARARGS(convertUserName), NOP_VARARGS(normalizedAccountName), END
};
STUB_MODULE_METHODS(util_UIMath, "util.UIMath", util_UIMath_methods)

/* ── Pure empty stubs ─────────────────────────────────────────────────── */

STUB_MODULE(debug_utils, "debug_utils")
STUB_MODULE(chat_shared, "chat_shared")
STUB_MODULE(bwobsolete_helpers, "bwobsolete_helpers")
STUB_MODULE(account_helpers, "account_helpers")
STUB_MODULE(items, "items")
STUB_MODULE(nations, "nations")
STUB_MODULE(ArenaType, "ArenaType")
STUB_MODULE(MapType, "MapType")
STUB_MODULE(ClientArena, "ClientArena")
STUB_MODULE(SoundGroups, "SoundGroups")
STUB_MODULE(Vivox, "Vivox")
STUB_MODULE(PixieBG, "PixieBG")
STUB_MODULE(PostProcessing, "PostProcessing")
STUB_MODULE(ConnectionManager, "ConnectionManager")
STUB_MODULE(predefined_hosts, "predefined_hosts")
STUB_MODULE(WebBrowser, "WebBrowser")
STUB_MODULE(CollisionMath, "CollisionMath")
STUB_MODULE(ProfileParser, "ProfileParser")
STUB_MODULE(Core, "Core")
STUB_MODULE(Vary, "Vary")
STUB_MODULE(CAT, "CAT")
STUB_MODULE(Trails, "Trails")
STUB_MODULE(server_constants, "server_constants")
STUB_MODULE(FlagsPreloader, "FlagsPreloader")
STUB_MODULE(PhysicsManager, "PhysicsManager")
STUB_MODULE(StaticSceneMgr, "StaticSceneMgr")
STUB_MODULE(WorldEventManager, "WorldEventManager")
STUB_MODULE(WorldObjectManager, "WorldObjectManager")
STUB_MODULE(WaveSystem, "WaveSystem")
STUB_MODULE(ManyObjects, "ManyObjects")
STUB_MODULE(WeatherApi, "WeatherApi")
STUB_MODULE(Notification, "Notification")
