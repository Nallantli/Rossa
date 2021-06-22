locale=ENG

GCC=g++

CV=--std=c++17
CC=$(GCC) -D_LOCALIZED_ -D_LOCALE_$(locale)_ -Wall $(CV) -O2

ifeq ($(OS),Windows_NT)

LIB_EXT=.dll

CFLAGS=
LFLAGS=-shared
OFLAGS=$(CFLAGS)

LIB_NET_FLAGS=-lwsock32 -lws2_32 -lboost_system-mt
LIB_FS_FLAGS=-lzip
LIB_SDL_FLAGS=-lmingw32 -lgdi32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
LIB_NCURSES_FLAGS=-lncurses
LIB_ARBITRARY_FLAGS=-lgmp -lgmpxx

DIR=build/win/$(locale)

dir_target = $(DIR)-$(wildcard $(DIR))
dir_present = $(DIR)-$(DIR)
dir_absent = $(DIR)-

all: | $(dir_target)

$(dir_present): bin/rossa.exe

$(dir_absent): $(DIR) bin/rossa.exe

$(DIR):
	mkdir -p $@

else

LIB_EXT=.so

CFLAGS=-ldl -pthread
LFLAGS=-fPIC -shared -ldl -pthread
OFLAGS=-fPIC $(CFLAGS)

LIB_NET_FLAGS=-lboost_system
LIB_FS_FLAGS=-lzip
LIB_SDL_FLAGS=-lSDL2 -lSDL2_image -lSDL2_ttf
LIB_NCURSES_FLAGS=-lncurses
LIB_ARBITRARY_FLAGS=-lgmp -lgmpxx

DIR=build/nix/$(locale)

dir_target = $(DIR)-$(wildcard $(DIR))
dir_present = $(DIR)-$(DIR)
dir_absent = $(DIR)-

all: | $(dir_target)

$(dir_present): bin/rossa

$(dir_absent): $(DIR) bin/rossa

$(DIR):
	mkdir -p $@

endif

dirs: $(DIR)

libs: lib_standard lib_fs lib_net lib_graphics lib_SDL lib_ncurses lib_Arbitrary

lib_standard: bin/lib/lib_standard$(LIB_EXT)

lib_fs: bin/lib/lib_fs$(LIB_EXT)

lib_net: bin/lib/lib_net$(LIB_EXT)

lib_graphics: bin/lib/lib_graphics$(LIB_EXT)

lib_SDL: bin/lib/lib_SDL$(LIB_EXT)

lib_ncurses: bin/lib/lib_ncurses$(LIB_EXT)

lib_Arbitrary: bin/lib/lib_Arbitrary$(LIB_EXT)

bin/lib/lib_standard$(LIB_EXT): lib_standard/lib_standard.cpp $(DIR)/librossa.a
	$(CC) -o $@ lib_standard/lib_standard.cpp $(DIR)/librossa.a $(LFLAGS)

bin/lib/lib_fs$(LIB_EXT): lib_fs/lib_fs.cpp $(DIR)/librossa.a
	$(CC) -o $@ lib_fs/lib_fs.cpp $(DIR)/librossa.a $(LFLAGS) $(LIB_FS_FLAGS)

bin/lib/lib_net$(LIB_EXT): lib_net/lib_net.cpp $(DIR)/librossa.a
	$(CC) -o $@ lib_net/lib_net.cpp $(DIR)/librossa.a $(LFLAGS) $(LIB_NET_FLAGS)

bin/lib/lib_graphics$(LIB_EXT): lib_graphics/lib_graphics.cpp $(DIR)/librossa.a
	$(CC) -o $@ lib_graphics/lib_graphics.cpp $(DIR)/librossa.a $(LFLAGS) $(LIB_SDL_FLAGS)

bin/lib/lib_SDL$(LIB_EXT): lib_SDL/lib_SDL.cpp $(DIR)/librossa.a
	$(CC) -o $@ lib_SDL/lib_SDL.cpp $(DIR)/librossa.a $(LFLAGS) $(LIB_SDL_FLAGS)

bin/lib/lib_ncurses$(LIB_EXT): lib_ncurses/lib_ncurses.cpp $(DIR)/librossa.a
	$(CC) -o $@ lib_ncurses/lib_ncurses.cpp $(DIR)/librossa.a $(LFLAGS) $(LIB_NCURSES_FLAGS)

bin/lib/lib_Arbitrary$(LIB_EXT): lib_Arbitrary/lib_Arbitrary.cpp $(DIR)/librossa.a
	$(CC) -o $@ lib_Arbitrary/lib_Arbitrary.cpp $(DIR)/librossa.a $(LFLAGS) $(LIB_ARBITRARY_FLAGS)

bin/rossa.exe: main/Main.cpp $(DIR)/librossa.a
	$(CC) -o $@ main/Main.cpp $(DIR)/librossa.a $(CFLAGS)

bin/rossa: main/Main.cpp $(DIR)/librossa.a
	$(CC) -o $@ main/Main.cpp $(DIR)/librossa.a $(CFLAGS)

$(DIR)/librossa.a: $(DIR)/parser.o $(DIR)/function.o $(DIR)/instruction.o $(DIR)/global.o $(DIR)/node.o $(DIR)/node_parser.o $(DIR)/object.o $(DIR)/operation.o $(DIR)/parameter.o $(DIR)/scope.o $(DIR)/signature.o $(DIR)/symbol.o $(DIR)/value.o $(DIR)/wrapper.o $(DIR)/rossa_error.o $(DIR)/number.o
	ar rcs $@ $(DIR)/parser.o $(DIR)/function.o $(DIR)/instruction.o $(DIR)/global.o $(DIR)/node.o $(DIR)/node_parser.o $(DIR)/object.o $(DIR)/operation.o $(DIR)/parameter.o $(DIR)/scope.o $(DIR)/signature.o $(DIR)/symbol.o $(DIR)/value.o $(DIR)/wrapper.o $(DIR)/rossa_error.o $(DIR)/number.o

$(DIR)/parser.o: main/rossa/parser/parser.cpp
	$(CC) -o $@ main/rossa/parser/parser.cpp -c $(OFLAGS)

$(DIR)/function.o: main/rossa/function/function.cpp
	$(CC) -o $@ main/rossa/function/function.cpp -c $(OFLAGS)

$(DIR)/instruction.o: main/rossa/instruction/instruction.cpp
	$(CC) -o $@ main/rossa/instruction/instruction.cpp -c $(OFLAGS)

$(DIR)/global.o: main/rossa/global/global.cpp
	$(CC) -o $@ main/rossa/global/global.cpp -c $(OFLAGS)

$(DIR)/node.o: main/rossa/node/node.cpp
	$(CC) -o $@ main/rossa/node/node.cpp -c $(OFLAGS)

$(DIR)/node_parser.o: main/rossa/node_parser/node_parser.cpp
	$(CC) -o $@ main/rossa/node_parser/node_parser.cpp -c $(OFLAGS)

$(DIR)/object.o: main/rossa/object/object.cpp
	$(CC) -o $@ main/rossa/object/object.cpp -c $(OFLAGS)

$(DIR)/operation.o: main/rossa/operation/operation.cpp
	$(CC) -o $@ main/rossa/operation/operation.cpp -c $(OFLAGS)

$(DIR)/parameter.o: main/rossa/parameter/parameter.cpp
	$(CC) -o $@ main/rossa/parameter/parameter.cpp -c $(OFLAGS)

$(DIR)/scope.o: main/rossa/scope/scope.cpp
	$(CC) -o $@ main/rossa/scope/scope.cpp -c $(OFLAGS)

$(DIR)/signature.o: main/rossa/signature/signature.cpp
	$(CC) -o $@ main/rossa/signature/signature.cpp -c $(OFLAGS)

$(DIR)/symbol.o: main/rossa/symbol/symbol.cpp
	$(CC) -o $@ main/rossa/symbol/symbol.cpp -c $(OFLAGS)

$(DIR)/value.o: main/rossa/value/value.cpp
	$(CC) -o $@ main/rossa/value/value.cpp -c $(OFLAGS)

$(DIR)/wrapper.o: main/rossa/wrapper/wrapper.cpp
	$(CC) -o $@ main/rossa/wrapper/wrapper.cpp -c $(OFLAGS)

$(DIR)/rossa_error.o: main/rossa/rossa_error/rossa_error.cpp
	$(CC) -o $@ main/rossa/rossa_error/rossa_error.cpp -c $(OFLAGS)

$(DIR)/number.o: main/rossa/number/number.cpp
	$(CC) -o $@ main/rossa/number/number.cpp -c $(OFLAGS)