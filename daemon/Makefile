EXE := cdptool
SRC := $(shell find . -type f -name '*.c') $(shell find ../libcdp -type f -name '*.c')
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)

# Preprocessor flags here
CPPFLAGS	:= -MMD -MP -I.
# Compiler flags here
CFLAGS		:= -std=c99 -Wall -Werror-implicit-function-declaration

.PHONY: all debug run clean

all: CPPFLAGS += -DNDEBUG
all: $(EXE)

debug: CPPFLAGS += -DDEBUG
debug: CFLAGS += -g
debug: $(EXE)

$(EXE): $(OBJ)
		@echo "Compilation complete!"
		$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
		@echo "Linking complete!"

-include $(DEP)

run:
		./$(EXE)

clean:
		@$(RM) $(EXE) $(OBJ) $(DEP) *.dSYM
		@echo "Cleanup complete!"
