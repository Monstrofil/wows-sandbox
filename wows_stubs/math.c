/*
 * wows_stubs/math.c — Math module stub with MathObj (Vector/Matrix).
 */
#include "common.h"

static PyObject *MathObj_class = NULL;

static PyObject *
Math_create(PyObject *self, PyObject *args, PyObject *kwargs)
{
    if (MathObj_class == NULL) {
        PyObject *d = PyDict_New();
        PyObject *bi = PyImport_ImportModule("__builtin__");
        PyDict_SetItemString(d, "__builtins__", bi);
        Py_XDECREF(bi);
        PyRun_String(
            "class MathObj(object):\n"
            "    x = y = z = w = 0.0\n"
            "    def __init__(self, *a, **kw):\n"
            "        if a:\n"
            "            coords = list(a) + [0.0]*4\n"
            "            self.x, self.y, self.z, self.w = coords[:4]\n"
            "    def setIdentity(self): pass\n"
            "    def setRotateYPR(self, *a): pass\n"
            "    def setTranslate(self, *a): pass\n"
            "    def invert(self): return self\n"
            "    def __getitem__(self, i): return 0.0\n"
            "    def __setitem__(self, i, v): pass\n"
            "    def __mul__(self, o): return self\n"
            "    def __rmul__(self, o): return self\n"
            "    def __div__(self, o): return self\n"
            "    def __rdiv__(self, o): return self\n"
            "    def __truediv__(self, o): return self\n"
            "    def __add__(self, o): return self\n"
            "    def __radd__(self, o): return self\n"
            "    def __sub__(self, o): return self\n"
            "    def __rsub__(self, o): return self\n"
            "    def __neg__(self): return self\n"
            "    def __len__(self): return 4\n"
            "    def __iter__(self): return iter((self.x,self.y,self.z,self.w))\n"
            "    def __repr__(self): return 'MathObj(%s,%s,%s)' % (self.x,self.y,self.z)\n"
            "    def list(self): return [self.x, self.y, self.z]\n"
            "    def scale(self, s): return self\n"
            "    def dot(self, o): return 0.0\n"
            "    def cross(self, o): return MathObj()\n"
            "    def length(self): return 0.0\n"
            "    def lengthSquared(self): return 0.0\n"
            "    def distTo(self, o): return 0.0\n"
            "    def flatDistTo(self, o): return 0.0\n"
            "    def unitVector(self): return MathObj(0,0,1)\n"
            "    def normalise(self): return self\n"
            "    def normalize(self): return self\n"
            "    def lookAt(self, *a): pass\n"
            "    def applyPoint(self, p): return MathObj()\n"
            "    def translation(self): return MathObj()\n"
            "    def yaw(self): return 0.0\n"
            "    def pitch(self): return 0.0\n"
            "    def roll(self): return 0.0\n",
            Py_file_input, d, d);
        MathObj_class = PyDict_GetItemString(d, "MathObj");
        Py_XINCREF(MathObj_class);
        gc_untrack_class_and_dict(MathObj_class, d);
    }
    if (MathObj_class == NULL) Py_RETURN_NONE;
    return PyObject_Call(MathObj_class, args ? args : PyTuple_New(0), kwargs);
}

static PyMethodDef Math_methods[] = {
    NOP_VARARGS(slerp),
    NOP_VARARGS(lerp),
    NOP_VARARGS(clamp),
    END
};

PyObject *init_Math(void)
{
    PyObject *m = make_module("Math", Math_methods, "Math stub");
    if (m == NULL) return NULL;

    Math_create(NULL, NULL, NULL);
    if (MathObj_class == NULL) return NULL;

    static const char *types[] = {
        "Vector2", "Vector3", "Vector4", "Matrix",
        "MatrixProduct", "MatrixAnimation", "MatrixProvider",
        "Vector", NULL
    };
    const char **n;
    for (n = types; *n; n++) {
        Py_INCREF(MathObj_class);
        PyModule_AddObject(m, *n, MathObj_class);
    }

    return m;
}
