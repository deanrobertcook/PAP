CFLAGS=-Wall -g

SOURCES := main.c
EXECUTABLES := $(SOURCES:.c=)

all: $(EXECUTABLES)

clean:
	rm -f $(EXECUTABLES)
	rm -rf *.dSYM

%: %.c
	$(CC) $(CFLAGS) $< -o $@

