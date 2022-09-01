CC=gcc
CFLAGS=-Wall -Wextra -g

EXE = bin/workouts
TST = src/tst/test
BIN_SRC = src/main.c
TST_SRC = src/tst/tests.c
SRC = $(filter-out $(BIN_SRC) $(TST_SRC), $(wildcard src/*.c))
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
DEPS = $(wildcard src/*.h)

.PHONY: default all clean

default: $(EXE) $(TST)

all: default

obj/%.o: src/%.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)


src/tst/tests.o: testing/tests.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

.PRECIOUS: $(EXE) $(OBJ)

$(EXE): $(OBJ) $(BIN_SRC)
		$(CC) -o $@ $^ $(CFLAGS)
		rm -f *.o

$(TST):  $(OBJ) $(TST_SRC)
		$(CC) -o $@ $^ $(CFLAGS)

clean:
		rm -f *~ src/*~ obj/*.o $(EXE) $(TST)

test:
		$(TST)
