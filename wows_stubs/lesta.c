/*
 * wows_stubs/lesta.c — Lesta engine module stub.
 *
 * Lesta provides physics, math utilities, ballistics, and screen-space
 * functions used throughout WoWS client code.
 */
#include "common.h"

static PyMethodDef Lesta_methods[] = {
    NOP_VARARGS(isLesta),
    NOP_FALSE(isLesta),
    NOP_VARARGS(setPhysicsParams),
    NOP_VARARGS(angleBetweenVectors),
    NOP_VARARGS(averagePoint),
    NOP_VARARGS(averagePoint2D),
    NOP_VARARGS(checkPointInPolygon),
    NOP_VARARGS(decay),
    NOP_VARARGS(getDirectionFromPitchYaw),
    NOP_VARARGS(getDirectionFromYaw),
    NOP_VARARGS(lineLineIntersection2D),
    NOP_VARARGS(projectionToSegment),
    NOP_VARARGS(setupBlueLine),
    NOP_VARARGS(setupNavigation),
    NOP_VARARGS(loadLineTexture),
    NOP_VARARGS(getCollisionMaterialsNum),
    NOP_VARARGS(getAllProperties),
    NOP_TUPLE2(getShipPhysicsDragPower),
    NOP_VARARGS(ballistics_trajectory),
    NOP_VARARGS(pyFlattenTrajHeight),
    NOP_VARARGS(getTrajectoryDist),
    NOP_VARARGS(shotTracerSetConstants),
    NOP_VARARGS(shellModelGlobalSettings),
    NOP_STR(getClientRealm),
    NOP_TUPLE3(getScreenPositionByWorldPosition),
    NOP_TUPLE3(getScreenPositionByWorldPositionExtended),
    NOP_FALSE(isOnScreen),
    NOP_VARARGS(bigworldMetersToUI),
    NOP_VARARGS(normaliseAngle),
    NOP_VARARGS(getRayIntersectWithScreenHorizontalLine),
    NOP_LIST(getPointsOfWorldLineOnScreen),
    NOP_VARARGS(getLinesIntersectionSp),
    NOP_VARARGS(rotateVectorAround),
    NOP_VARARGS(taSetDecalsTexture),
    NOP_VARARGS(taSetDecalsParams),
    NOP_VARARGS(taSetNavigationEnabled),
    NOP_VARARGS(taSetNavigationParams),
    NOP_VARARGS(taSetNavigationTexture),
    NOP_INT(addTimer),
    NOP_VARARGS(cancelTimer),
    NOP_VARARGS(setBallicticScale),
    NOP_VARARGS(setBallisticFlattening),
    NOP_VARARGS(setBallisticGravity),
    NOP_VARARGS(setBallisticParams),
    NOP_VARARGS(setBallisticScale),
    NOP_VARARGS(setShotSpeed),
    NOP_VARARGS(setAvatarFilterParams),
    END
};

PyObject *init_Lesta(void)
{
    PyObject *m = make_module("Lesta", Lesta_methods, "Lesta stub");
    if (m) {
        PyModule_AddObject(m, "IS_LESTA", (Py_INCREF(Py_False), Py_False));
        PyModule_AddObject(m, "PyGunRotator", get_flex_base_type());
        PyModule_AddObject(m, "ShotTrajectory", get_flex_base_type());
        PyModule_AddObject(m, "ModelMovableParts", get_flex_base_type());
    }
    return m;
}
