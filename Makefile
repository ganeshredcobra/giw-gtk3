PKGS=gtk+-3.0 glib-2.0 gmodule-2.0 
CFLAGS=-g3 `pkg-config --cflags $(PKGS)`
CC=gcc
LDLIBS=`pkg-config --libs $(PKGS)` -lm

CFILES_LIB=$(shell find . -iname 'giw*.c')
CFILES_EXAMPLES=$(shell find . -iname '*example.c')
EXAMPLES_BIN=$(patsubst %.c, %, $(CFILES_EXAMPLES))

OBJ=$(patsubst %.c, %.o, $(CFILES_LIB) $(CFILES_EXAMPLES))

all: $(EXAMPLES_BIN)

define make-examples
$1: $1.o $2
	@echo $1 $1.o $2
	gcc -o $$@ $$^ $(LDLIBS)
endef

$(foreach i, $(CFILES_EXAMPLES), $(eval $(call make-examples, $(basename $i), $(patsubst %.c, %.o, $(wildcard $(dir $i)giw*.c)))))

clean:
	rm -f $(OBJ) $(EXAMPLES_BIN)
