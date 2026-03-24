# WoWS Sandbox

A standalone Linux binary that loads and executes World of Warships game scripts from encrypted `scripts.zip` archives. Embeds a patched Python 2.7 interpreter with NOP stubs for the BigWorld engine APIs, allowing game modules to be imported and inspected without the game client.

## What it does

The WoWS game client embeds Python 2.7 and ships its scripts as obfuscated `.pyc` files inside `scripts.zip`. This tool:

- **Decrypts** the 4-stage obfuscation (XOR + base64 + zlib + marshal + swap cipher)
- **Loads** modules via a PEP 302 meta_path importer (pure C, no Python stdlib deps)
- **Stubs** the BigWorld engine APIs (BigWorld, Lesta, ResMgr, Math, Event, GUI, etc.)
- **Fakes** a Windows environment so Windows-specific game code runs on Linux

The result: `BWPersonality` (the game's main entry point) loads with ~3000 modules and 164+ public attributes, ready for inspection.

## Quick start

### Prerequisites

Build requires GCC, zlib, libssl, libffi development headers.

### Build

```bash
git clone --recurse-submodules https://github.com/Monstrofil/wows-sandbox.git
cd wows-sandbox

# Build Python 2.7.18 static library (one-time, from submodule)
make python

# Build wows_shell
make
```

### Run

Place your `scripts.zip` in the `data/` directory, then:

```bash
# Interactive REPL
PYTHONHOME=3rdparty/cpython ./wows_shell

# Execute a script
PYTHONHOME=3rdparty/cpython ./wows_shell test_extract.py

# One-liner
PYTHONHOME=3rdparty/cpython ./wows_shell -c 'import BWPersonality; print dir(BWPersonality)'

# Use a different scripts.zip
PYTHONHOME=3rdparty/cpython ./wows_shell --zip /path/to/scripts.zip
```

## REPL helpers

The interactive shell comes with built-in search tools for navigating obfuscated module names:

```python
>>> find_class("TaskType")
  CommonBattleLogicClasses.m796427e0.TaskType  (9 attrs)
  m92ea29c6.TaskSystem.TaskType  (9 attrs)

>>> find_attr("ID_TO_NAME")
  CrewModifiers.SkillTypeEnum.ID_TO_NAME  (dict) = {0: 'NoneSkill', 1: ...}

>>> find_module("Crew")
  CrewModifiers
  CrewSkill
  ClientCrewSkills
  ...

>>> dump(CrewModifiers.ShipTypes)
  AIRCARRIER = 'AirCarrier'
  BATTLESHIP = 'Battleship'
  CRUISER = 'Cruiser'
  ...

>>> find_func("getPreferences")
  ModsShell.getPreferences  (function)

>>> grep_modules("ctypes")
  uuid  (has 'ctypes')
```

## Project structure

```
wows-sandbox/
├── main.c                       Entry point (REPL, -c, script modes)
├── Makefile                     Build system
├── data/
│   └── scripts.zip              Game scripts (not tracked)
├── helpers/
│   └── wows_helpers.py          REPL search tools
├── wows_importer/
│   ├── wows_importer.c          PEP 302 meta_path importer
│   └── wows_importer.h
├── wows_stubs/
│   ├── common.h                 Macros (NOP_VARARGS, STUB_MODULE, FlexBase)
│   ├── common.c                 FlexBase type implementation
│   ├── bigworld.c               BigWorld module (~70 methods + types)
│   ├── lesta.c                  Lesta module (physics, ballistics)
│   ├── resmgr.c                 ResMgr + DataSection
│   ├── math.c                   Math + MathObj (Vector/Matrix)
│   ├── event.c                  Event (observer pattern)
│   ├── modules.c                All smaller stubs (~50 modules)
│   ├── install.c                Registration + Windows env setup
│   ├── wows_stubs.h             Public API
│   ├── wows_decrypt.c           4-stage .pyc decryption
│   └── wows_decrypt.h
├── zip_reader/
│   ├── zip_reader.c             Pure C zip archive reader
│   └── zip_reader.h
└── test_extract.py              Smoke test
```

## How it works

### Decryption pipeline

Each `.pyc` in the zip is encrypted with a 4-stage pipeline:

1. **Stage 1**: XOR the last `co_consts` entry with `co_code` as key, base64-decode, zlib-decompress, unmarshal
2. **Stage 2**: Apply a byte substitution table (`swap_map`) from `co_consts[8].co_consts[1]`
3. **Stage 3**: Bit cipher (XOR 38, rotate bits 0/7, XOR 89), reverse, unmarshal
4. **Stage 4**: Split by `<<<>>>` delimiter, reverse payload, base64, zlib, unmarshal to final code object

### Engine stubs

The game's Python scripts import engine-provided C modules (`BigWorld`, `Lesta`, `Math`, `ResMgr`, etc.) that don't exist outside the game client. We provide NOP implementations:

- **FlexBase**: a universal base type whose `__getattr__` returns new instances (handles arbitrary attribute access)
- **DataSection**: returned by `ResMgr.openSection()`, supports `readString()`, `readInt()`, `children()`, etc.
- **MathObj**: used as Vector2/3/4, Matrix — supports arithmetic operators
- **Event**: observer pattern with `__iadd__`/`__isub__`/`__call__`

### Windows environment

The game scripts are the Windows build. We fake the platform:
- `os.name = "nt"`, `sys.platform = "win32"`
- Patch `_ctypes`, `_socket`, `_ssl` with Windows-only symbols
- Stub `msvcrt`, `_subprocess`, `_winreg`
- Export dummy `GetLastError`/`SetLastError` symbols

## License

This tool is for research and educational purposes. Game scripts (`scripts.zip`) are property of Wargaming.net / Lesta Studio.
