PKGS=gtk+-3.0 glib-2.0 gmodule-2.0 
CFLAGS=-g3 `pkg-config --cflags $(PKGS)` -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED
CC=gcc
LDLIBS=`pkg-config --libs $(PKGS)` -lm

CFILES_WIDGETS=$(shell find . -iname 'giw*.c')
LIBRARY_NAMES_WIDGET=$(join $(dir $(CFILES_WIDGETS)), $(patsubst %.c, lib%.so, $(notdir $(CFILES_WIDGETS))))
CFILES_EXAMPLES=$(shell find . -iname '*example.c')
EXAMPLES_BIN=$(patsubst %.c, %, $(CFILES_EXAMPLES))

OBJ=$(patsubst %.c, %.o, $(CFILES_WIDGETS) $(CFILES_EXAMPLES))

all: $(EXAMPLES_BIN) $(LIBRARY_NAMES_WIDGET)

define make-examples
$1: $1.o $2
	@echo $1 $1.o $2
	gcc -o $$@ $$^ $(LDLIBS)
endef

define make-libraries
$1: $2
	@echo $1 $2
	gcc -shared -o $$@ $$^ $(LDLIBS)
endef

###############################################
# expand rules for examples
###############################################
$(foreach i, $(CFILES_EXAMPLES),\
	$(eval $(call make-examples,\
		$(basename $i),\
		$(patsubst %.c, %.o, $(wildcard $(dir $i)giw*.c)))\
	)\
)

###############################################
# expand rules for shared libraries of widgets
###############################################
$(foreach i, $(CFILES_WIDGETS),\
	$(eval $(call make-libraries,\
		$(join\
			$(dir $i),\
			$(patsubst %.c, lib%.so, $(notdir $i))\
		),\
		$(patsubst %.c, %.o, $i))\
	)\
)

clean:
	rm -f $(OBJ) $(EXAMPLES_BIN) $(LIBRARY_NAMES_WIDGET)
