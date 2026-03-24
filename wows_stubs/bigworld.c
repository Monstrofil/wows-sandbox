/*
 * wows_stubs/bigworld.c — BigWorld engine module stub.
 */
#include "common.h"

/* Forward declaration — Event_Event is defined in event.c */
PyObject *Event_Event(PyObject *self, PyObject *args, PyObject *kwargs);

static PyMethodDef BigWorld_methods[] = {
    NOP_VARARGS(callback),
    NOP_VARARGS(cancelCallback),
    NOP_VARARGS(player),
    NOP_VARARGS(entities),
    NOP_VARARGS(entity),
    NOP_VARARGS(time),
    NOP_VARARGS(serverTime),
    NOP_VARARGS(worldDrawEnabled),
    NOP_VARARGS(projection),
    NOP_VARARGS(camera),
    NOP_VARARGS(dcursor),
    NOP_VARARGS(wg_openCodedSection),
    NOP_VARARGS(wg_openSection),
    NOP_VARARGS(screenSize),
    NOP_VARARGS(screenWidth),
    NOP_VARARGS(screenHeight),
    NOP_VARARGS(createEntity),
    NOP_VARARGS(destroyEntity),
    NOP_VARARGS(isPlayerName),
    NOP_VARARGS(PyResMgr),
    NOP_VARARGS(logInfo),
    NOP_VARARGS(logWarning),
    NOP_VARARGS(logError),
    NOP_VARARGS(logTrace),
    NOP_VARARGS(logHack),
    NOP_VARARGS(logCritical),
    NOP_VARARGS(cachedEntities),
    NOP_VARARGS(WGC_getLoginInfo),
    NOP_VARARGS(WGC_setLoginInfo),
    NOP_VARARGS(fetchURL),
    NOP_VARARGS(connect),
    NOP_VARARGS(disconnect),
    NOP_VARARGS(quit),
    NOP_VARARGS(savePreferences),
    NOP_VARARGS(loadPreferences),
    NOP_VARARGS(clearEntitiesAndSpaces),
    NOP_VARARGS(createSpace),
    NOP_VARARGS(addSpaceGeometryMapping),
    NOP_VARARGS(getSpaceDataFirstForKey),
    NOP_VARARGS(wg_binarySection),
    NOP_VARARGS(wg_resolveRelativePath),
    NOP_VARARGS(wg_resolveRelativePathForWrite),
    {"buildConfiguration", (PyCFunction)nop_str_consumer, METH_VARARGS, NULL},
    NOP_VARARGS(setPhysicsParams),
    NOP_VARARGS(setupNavigation),
    NOP_VARARGS(setupBlueLine),
    NOP_VARARGS(loadLineTexture),
    NOP_VARARGS(getCollisionMaterialsNum),
    NOP_STR(getClientRealm),
    NOP_VARARGS(getAllProperties),
    NOP_VARARGS(PyGunRotator),
    NOP_VARARGS(GraphicsSettingCallback),
    NOP_VARARGS(getGraphicsSetting),
    NOP_VARARGS(isControllingCamera),
    NOP_VARARGS(getCursorPos),
    NOP_VARARGS(findNearestToCursorPositionIndex),
    NOP_VARARGS(findNearestSquadron2D),
    NOP_VARARGS(debugEnabled),
    NOP_VARARGS(ballistics_trajectory),
    NOP_VARARGS(soundSystem),
    {"getUrlById", (PyCFunction)nop_str_url, METH_VARARGS, NULL},
    NOP_VARARGS(createPersistentDecalProto),
    NOP_VARARGS(initDynamicDecals),
    NOP_VARARGS(createDecalInst),
    NOP_FALSE(isSteamPresent),
    NOP_FALSE(isWinStorePresent),
    NOP_FALSE(isEpicStorePresent),
    NOP_FALSE(isEpicPresent),
    NOP_FALSE(isGogPresent),
    NOP_FALSE(isWGCPresent),
    NOP_FALSE(isVKPlayPresent),
    NOP_STR(getAcceptLangs),
    NOP_VARARGS(precacheResourcesFromBG),
    END
};

PyObject *
init_BigWorld(void)
{
    PyObject *m = make_module("BigWorld", BigWorld_methods, "BigWorld engine stub");
    if (m == NULL) return NULL;

    add_int(m, "platform", 0);
    add_int(m, "isPublicBuild", 1);
    PyModule_AddObject(m, "player", (Py_INCREF(Py_None), Py_None));

    /* component — string "client" that is also callable */
    {
        PyObject *d = PyDict_New();
        PyObject *bi = PyImport_ImportModule("__builtin__");
        PyDict_SetItemString(d, "__builtins__", bi);
        Py_XDECREF(bi);
        PyRun_String(
            "class _Component(str):\n"
            "    def __call__(self, *a, **kw): return self\n",
            Py_file_input, d, d);
        PyObject *cls = PyDict_GetItemString(d, "_Component");
        gc_untrack_class_and_dict(cls, d);
        if (cls) {
            PyObject *val = PyObject_CallFunction(cls, "s", "client");
            if (val) PyModule_AddObject(m, "component", val);
        }
    }

    /* Engine Event objects */
    {
        static const char *evt_names[] = {
            "gLostConnection", "gScaleUpdated",
            "onStreamComplete", "onCameraChange", NULL
        };
        const char **n;
        for (n = evt_names; *n; n++) {
            PyObject *inst = Event_Event(NULL, NULL, NULL);
            if (inst) PyModule_AddObject(m, *n, inst);
            else PyErr_Clear();
        }
    }

    /* Engine types — FlexBase so game code can inherit */
    {
        static const char *types[] = {
            "Entity", "LatencyInfo", "ScriptComponent",
            "DogWatch", "PyGunRotator", "PhysicsBodyWrapper",
            "UserDataObject", "Future", "DecalProperties", NULL
        };
        const char **p;
        for (p = types; *p; p++)
            PyModule_AddObject(m, *p, get_flex_base_type());
    }

    /* Engine instances — FlexBase instances for enum-like objects */
    {
        static const char *insts[] = {
            "MISC_TYPE", "DECAL_INFLUENCE", "DECAL_FLIP",
            "DECAL_TECHNIQUE", "SHOT_DECALS_PROTO_LIST",
            "GROUND_DECALS_PROTO_LIST", "FIRE_DECALS_PROTO_LIST",
            "HEAT_DECALS_PROTO_LIST", NULL
        };
        const char **n;
        for (n = insts; *n; n++) {
            PyObject *cls = get_flex_base_type();
            if (cls) {
                PyObject *inst = PyObject_CallObject(cls, NULL);
                if (inst) PyModule_AddObject(m, *n, inst);
                Py_DECREF(cls);
            }
        }
    }

    PyModule_AddObject(m, "loadLineTexture", (Py_INCREF(Py_None), Py_None));
    add_str(m, "getClientRealm", "");

    /* Coordinate conversion */
    PyModule_AddObject(m, "BW_TO_BALLISTIC", PyFloat_FromDouble(1.0));
    PyModule_AddObject(m, "BALLISTIC_TO_BW", PyFloat_FromDouble(1.0));
    PyModule_AddObject(m, "BW_TO_SHIP", PyFloat_FromDouble(1.0));
    PyModule_AddObject(m, "SHIP_TO_BW", PyFloat_FromDouble(1.0));

    /* Platform response codes */
    add_int(m, "WIN_STORE_ACCOUNT_RESPONSE_FAIL", 0);
    add_int(m, "WIN_STORE_ACCOUNT_RESPONSE_SUCCESS", 1);
    add_int(m, "WIN_STORE_ACCOUNT_RESPONSE_INTERNAL_ERROR", 2);
    add_int(m, "WIN_STORE_ACCOUNT_RESPONSE_TIMEOUT", 3);
    add_int(m, "WIN_STORE_ACCOUNT_RESPONSE_CANCEL", 4);
    add_int(m, "WIN_STORE_ACCOUNT_RESPONSE_NOT_FOUND", 5);

    return m;
}
