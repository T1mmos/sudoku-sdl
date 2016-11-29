.DEFAULT_GOAL := sudoku
CC = g++
COMPILER_FLAGS = -std=c++11
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf
SRC=src
OBJ=obj

OBJ_FILES = \
    $(OBJ)/main.o \
	$(OBJ)/Area.o \

$(OBJ)/main.o : $(SRC)/main.cpp $(SRC)/CellData.h $(SRC)/Area.h
	${CC} $< $(COMPILER_FLAGS) -c $(LINKER_FLAGS) -o $@
	
$(OBJ)/Area.o : $(SRC)/Area.cpp $(SRC)/Area.h $(SRC)/CellData.h
	${CC} $< $(COMPILER_FLAGS) -c $(LINKER_FLAGS) -o $@

sudoku : $(OBJ_FILES)
	${CC} ${COMPILER_FLAGS} $^ ${LDFLAGS} $(LINKER_FLAGS) -o sudoku