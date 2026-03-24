/*
 * wows_stubs.h — Install WoWS engine stub modules.
 */
#ifndef WOWS_STUBS_H
#define WOWS_STUBS_H

#include "Python.h"

/*
 * Install all stub modules (BigWorld, ResMgr, etc.) into sys.modules,
 * add _NamedConstantsType to __builtin__, and install the fallback
 * importer at the end of sys.meta_path.
 * Returns 0 on success, -1 on error.
 */
int wows_stubs_install(void);

#endif /* WOWS_STUBS_H */
