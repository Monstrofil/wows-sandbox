# -*- coding: utf-8 -*-
"""
wows_helpers.py - Built-in tools for wows_shell.

Loaded automatically at startup. Provides search helpers for navigating
the obfuscated game scripts without knowing the mangled module names.

Usage in REPL:
    >>> find_module("TaskType")
    >>> find_class("SkillTypeEnum")
    >>> find_attr("ID_TO_NAME")
    >>> find_func("getPreferences")
"""
import sys
import types


def find_module(pattern):
    """Find loaded modules whose name contains the pattern (case-insensitive)."""
    pattern_lower = pattern.lower()
    results = []
    for name, mod in sorted(sys.modules.items()):
        if mod is None:
            continue
        if pattern_lower in name.lower():
            results.append(name)
    if results:
        for name in results:
            print "  %s" % name
    else:
        print "  (no modules matching '%s')" % pattern
    return results


def find_class(name):
    """Find a class by name across all loaded modules.

    Searches every loaded module for an attribute with the given name
    that is a class (type). Returns list of (module_name, class) tuples.
    """
    results = []
    for mod_name, mod in sorted(sys.modules.items()):
        if mod is None:
            continue
        obj = getattr(mod, name, None)
        if obj is not None and isinstance(obj, type):
            results.append((mod_name, obj))
    if results:
        for mod_name, cls in results:
            attrs = [a for a in dir(cls) if not a.startswith('_')]
            print "  %s.%s  (%d attrs)" % (mod_name, name, len(attrs))
    else:
        print "  (no class '%s' found)" % name
    return results


def find_attr(name):
    """Find any attribute by name across all loaded modules.

    Returns list of (module_name, value) tuples.
    """
    results = []
    for mod_name, mod in sorted(sys.modules.items()):
        if mod is None:
            continue
        obj = getattr(mod, name, None)
        if obj is not None:
            results.append((mod_name, obj))
    if results:
        for mod_name, obj in results:
            t = type(obj).__name__
            preview = repr(obj)
            if len(preview) > 80:
                preview = preview[:77] + "..."
            print "  %s.%s  (%s) = %s" % (mod_name, name, t, preview)
    else:
        print "  (no attribute '%s' found)" % name
    return results


def find_func(name):
    """Find a function/method by name across all loaded modules."""
    results = []
    for mod_name, mod in sorted(sys.modules.items()):
        if mod is None:
            continue
        obj = getattr(mod, name, None)
        if obj is not None and callable(obj):
            results.append((mod_name, obj))
    if results:
        for mod_name, obj in results:
            t = type(obj).__name__
            print "  %s.%s  (%s)" % (mod_name, name, t)
    else:
        print "  (no callable '%s' found)" % name
    return results


def dump(obj, max_attrs=50):
    """Pretty-print an object's public attributes."""
    attrs = [(a, getattr(obj, a, '?')) for a in dir(obj) if not a.startswith('_')]
    for i, (name, val) in enumerate(attrs):
        if i >= max_attrs:
            print "  ... (%d more)" % (len(attrs) - max_attrs)
            break
        preview = repr(val)
        if len(preview) > 70:
            preview = preview[:67] + "..."
        print "  %s = %s" % (name, preview)


def grep_modules(pattern):
    """Search module co_names for a pattern. Finds which zip modules
    reference a given name even if they're not imported yet.

    Scans the first N modules from the importer's index.
    """
    pattern_lower = pattern.lower()
    imp = sys.meta_path[0] if sys.meta_path else None
    if imp is None:
        print "  (no WoWS importer found)"
        return []
    results = []
    # Scan loaded modules' code objects
    for mod_name, mod in sorted(sys.modules.items()):
        if mod is None or not hasattr(mod, '__loader__'):
            continue
        try:
            code = imp.get_code(mod_name)
        except:
            continue
        names = code.co_names
        for n in names:
            if pattern_lower in n.lower() and ' ' not in n:
                results.append((mod_name, n))
                break
    if results:
        for mod_name, matched in results:
            print "  %s  (has '%s')" % (mod_name, matched)
    else:
        print "  (no loaded module references '%s')" % pattern
    return results


def setup():
    """Install helpers into __builtin__ so they're available in REPL."""
    import __builtin__
    __builtin__.find_module = find_module
    __builtin__.find_class = find_class
    __builtin__.find_attr = find_attr
    __builtin__.find_func = find_func
    __builtin__.dump = dump
    __builtin__.grep_modules = grep_modules
