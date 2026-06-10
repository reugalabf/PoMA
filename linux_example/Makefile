#poma_example: main.c poma.c poma.h
#	gcc -o poma_example main.c poma_core.c -I.


CC = gcc
CFLAGS = -Wall -Wextra -I.

TARGET = poma_example
OBJ = main.o poma_tcpconnector.o poma_core.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Explicit header dependencies
# This ensures that if a header changes, the dependent .c file is recompiled
main.o: main.c poma_tcpconnector.h
poma_tcpconnector.o: poma_tcpconnector.c poma_core.h poma_core.h
poma_core.o: poma_core.c poma_core.h

# Generic rule for compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean