/*
 * wows_importer.h — PEP 302 meta_path importer for WoWS scripts.zip
 */
#ifndef WOWS_IMPORTER_H
#define WOWS_IMPORTER_H

#include "Python.h"

/*
 * Create a WoWSImporter for the given zip path and insert it at
 * position 0 in sys.meta_path.
 * Returns 0 on success, -1 on error (with Python exception set).
 */
int wows_importer_install(const char *zip_path);

#endif /* WOWS_IMPORTER_H */
