PYTHON_SRC = /tmp/Python-2.7.18
CC = gcc
CFLAGS = -I$(PYTHON_SRC)/Include -I$(PYTHON_SRC) -I. -O2 -Wall
LDFLAGS = -Wl,--export-dynamic -L$(PYTHON_SRC) -lpython2.7 -lpthread -ldl -lutil -lm -lz -lssl -lcrypto -lffi

STUBS_SRC = wows_stubs/common.c wows_stubs/bigworld.c wows_stubs/resmgr.c \
            wows_stubs/math.c wows_stubs/event.c wows_stubs/lesta.c \
            wows_stubs/modules.c wows_stubs/install.c
STUBS_OBJ = $(STUBS_SRC:.c=.o)
STUBS_HDR = wows_stubs/common.h wows_stubs/wows_stubs.h

IMPORTER_SRC = wows_importer/wows_importer.c
IMPORTER_OBJ = $(IMPORTER_SRC:.c=.o)

DECRYPT_SRC = wows_stubs/wows_decrypt.c
DECRYPT_OBJ = $(DECRYPT_SRC:.c=.o)

ZIP_SRC = zip_reader/zip_reader.c
ZIP_OBJ = $(ZIP_SRC:.c=.o)

OBJS = main.o $(IMPORTER_OBJ) $(STUBS_OBJ) $(DECRYPT_OBJ) $(ZIP_OBJ)

wows_shell: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c wows_stubs/wows_stubs.h wows_importer/wows_importer.h
$(IMPORTER_OBJ): wows_importer/wows_importer.h wows_stubs/wows_decrypt.h zip_reader/zip_reader.h
$(STUBS_OBJ): $(STUBS_HDR)
$(DECRYPT_OBJ): wows_stubs/wows_decrypt.h
$(ZIP_OBJ): zip_reader/zip_reader.h

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) wows_shell

.PHONY: clean
