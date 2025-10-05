CC=gcc -std=gnu11
SRC=src
OBJ=obj
OPT=-g
CFLAGS=-Wall -Wunused $(OPT)
LIB=-lm

# Todos los archivos .c y sus .o
SRCFILES := $(shell find $(SRC) -name '*.c')
OBJFILES := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCFILES))
HDRFILES := $(shell find $(SRC) -name '*.h')
OBJDIRS := $(sort $(dir $(OBJFILES)))

# Programas a compilar
PROGRAMS=os_memory

all: $(OBJDIRS) $(PROGRAMS)
	@echo "done compiling"

clean:
	@rm -rf $(OBJ) $(PROGRAMS) && echo "done cleaning"

# Crear directorios de objetos si no existen
$(OBJDIRS):
	@mkdir -p $@

# Compilar cada .c a .o
$(OBJ)/%.o: $(SRC)/%.c $(HDRFILES) Makefile
	@$(CC) $(CFLAGS) -c $< -o $@

# Enlazar todos los .o en el programa
$(PROGRAMS): $(OBJFILES)
	@$(CC) $(CFLAGS) $^ -o $@ $(LIB)
	@echo "compiled '$@'"
