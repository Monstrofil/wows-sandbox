#!/bin/bash
# Patch Modules/Setup to enable static C extensions needed by wows_shell.
# Run from the cpython source directory after ./configure.
set -e

SETUP="Modules/Setup"
if [ ! -f "$SETUP" ]; then
    echo "Error: $SETUP not found. Run ./configure first." >&2
    exit 1
fi

# Uncomment standard modules
sed -i 's/^#array /array /' "$SETUP"
sed -i 's/^#math /math /' "$SETUP"
sed -i 's/^#_struct /_struct /' "$SETUP"
sed -i 's/^#time /time /' "$SETUP"
sed -i 's/^#operator /operator /' "$SETUP"
sed -i 's/^#_random /_random /' "$SETUP"
sed -i 's/^#_collections /_collections /' "$SETUP"
sed -i 's/^#_heapq /_heapq /' "$SETUP"
sed -i 's/^#itertools /itertools /' "$SETUP"
sed -i 's/^#_functools /_functools /' "$SETUP"
sed -i 's/^#datetime /datetime /' "$SETUP"
sed -i 's/^#unicodedata /unicodedata /' "$SETUP"
sed -i 's/^#_io /_io /' "$SETUP"
sed -i 's/^#fcntl /fcntl /' "$SETUP"
sed -i 's/^#select /select /' "$SETUP"
sed -i 's/^#_socket /_socket /' "$SETUP"
sed -i 's/^#_md5 /_md5 /' "$SETUP"
sed -i 's/^#_sha /_sha /' "$SETUP"
sed -i 's/^#_sha256 /_sha256 /' "$SETUP"
sed -i 's/^#_sha512 /_sha512 /' "$SETUP"
sed -i 's/^#binascii /binascii /' "$SETUP"
sed -i 's/^#cStringIO /cStringIO /' "$SETUP"
sed -i 's/^#cPickle /cPickle /' "$SETUP"
sed -i 's|^#zlib |zlib |' "$SETUP"

# Add modules not in default Setup
grep -q '^_ssl ' "$SETUP" || echo '_ssl _ssl.c -DUSE_SSL -I/usr/include/openssl -lssl -lcrypto' >> "$SETUP"
grep -q '^_hashlib ' "$SETUP" || echo '_hashlib _hashopenssl.c -lssl -lcrypto' >> "$SETUP"
grep -q '^_json ' "$SETUP" || echo '_json _json.c' >> "$SETUP"
grep -q '^_ctypes ' "$SETUP" || echo '_ctypes _ctypes/_ctypes.c _ctypes/callbacks.c _ctypes/callproc.c _ctypes/stgdict.c _ctypes/cfield.c -lffi' >> "$SETUP"

echo "Modules/Setup patched for static build."
