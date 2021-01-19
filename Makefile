IDIR=../include
CC=gcc
CFLAGS=-I$(IDIR) -O2 -Wall -Wno-parentheses

ODIR=obj

LIBS=-lwiringPi -lm

_DEPS=lib.h canlib.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=lib.o canlib.o lvhv_routine.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
