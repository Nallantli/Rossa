locale=ENG

GCC=g++

CV=--std=c++17
CC=$(GCC) -D_LOCALIZED_ -D_LOCALE_$(locale)_ -Wall $(CV) -O2

ifeq ($(OS),Windows_NT)

LIB_EXT=.dll

CFLAGS=
LFLAGS=-shared
OFLAGS=$(CFLAGS)

LIBNET_FLAGS=-lwsock32 -lws2_32 -lboost_system-mt
LIBFS_FLAGS=-lzip
LIBSDL_FLAGS=-lmingw32 -lgdi32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
LIBNCURSES_FLAGS=-lncurses

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

LIBNET_FLAGS=-lboost_system
LIBFS_FLAGS=-lzip
LIBSDL_FLAGS=-lSDL2 -lSDL2_image -lSDL2_ttf
LIBNCURSES_FLAGS=-lncurses

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

libs: libstd libfs libnet libgraphics libsdl libncurses

libstd: bin/lib/libstd$(LIB_EXT)

libfs: bin/lib/libfs$(LIB_EXT)

libnet: bin/lib/libnet$(LIB_EXT)

libgraphics: bin/lib/libgraphics$(LIB_EXT)

libsdl: bin/lib/libsdl$(LIB_EXT)

libncurses: bin/lib/libncurses$(LIB_EXT)

bin/lib/libstd$(LIB_EXT): libstd/libstd.cpp $(DIR)/librossa.a
	$(CC) -o $@ libstd/libstd.cpp $(DIR)/librossa.a $(LFLAGS)

bin/lib/libfs$(LIB_EXT): libfs/libfs.cpp $(DIR)/librossa.a
	$(CC) -o $@ libfs/libfs.cpp $(DIR)/librossa.a $(LFLAGS) $(LIBFS_FLAGS)

bin/lib/libnet$(LIB_EXT): libnet/libnet.cpp $(DIR)/librossa.a
	$(CC) -o $@ libnet/libnet.cpp $(DIR)/librossa.a $(LFLAGS) $(LIBNET_FLAGS)

bin/lib/libgraphics$(LIB_EXT): libgraphics/libgraphics.cpp $(DIR)/librossa.a
	$(CC) -o $@ libgraphics/libgraphics.cpp $(DIR)/librossa.a $(LFLAGS) $(LIBSDL_FLAGS)

bin/lib/libsdl$(LIB_EXT): libsdl/libsdl.cpp $(DIR)/librossa.a
	$(CC) -o $@ libsdl/libsdl.cpp $(DIR)/librossa.a $(LFLAGS) $(LIBSDL_FLAGS)

bin/lib/libncurses$(LIB_EXT): libncurses/libncurses.cpp $(DIR)/librossa.a
	$(CC) -o $@ libncurses/libncurses.cpp $(DIR)/librossa.a $(LFLAGS) $(LIBNCURSES_FLAGS)

bin/rossa.exe: main/Main.cpp $(DIR)/librossa.a
	$(CC) -o $@ main/Main.cpp $(DIR)/librossa.a $(CFLAGS)

bin/rossa: main/Main.cpp $(DIR)/librossa.a
	$(CC) -o $@ main/Main.cpp $(DIR)/librossa.a $(CFLAGS)

$(DIR)/librossa.a: $(DIR)/Rossa.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Signature.o $(DIR)/Value.o $(DIR)/Symbol.o $(DIR)/RTError.o $(DIR)/Operator.o
	ar rcs $@ $(DIR)/Rossa.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Signature.o $(DIR)/Value.o $(DIR)/Symbol.o $(DIR)/RTError.o $(DIR)/Operator.o

$(DIR)/Rossa.o: main/lang/Rossa.cpp
	$(CC) -o $@ main/lang/Rossa.cpp -c $(OFLAGS)

$(DIR)/Node.o: main/lang/Node.cpp
	$(CC) -o $@ main/lang/Node.cpp -c $(OFLAGS)

$(DIR)/NodeParser.o: main/lang/NodeParser.cpp
	$(CC) -o $@ main/lang/NodeParser.cpp -c $(OFLAGS)

$(DIR)/Parser.o: main/lang/Parser.cpp
	$(CC) -o $@ main/lang/Parser.cpp -c $(OFLAGS)

$(DIR)/Scope.o: main/lang/Scope.cpp
	$(CC) -o $@ main/lang/Scope.cpp -c $(OFLAGS)

$(DIR)/Function.o: main/lang/Function.cpp
	$(CC) -o $@ main/lang/Function.cpp -c $(OFLAGS)

$(DIR)/Signature.o: main/lang/Signature.cpp
	$(CC) -o $@ main/lang/Signature.cpp -c $(OFLAGS)

$(DIR)/Value.o: main/lang/Value.cpp
	$(CC) -o $@ main/lang/Value.cpp -c $(OFLAGS)

$(DIR)/Symbol.o: main/lang/Symbol.cpp
	$(CC) -o $@ main/lang/Symbol.cpp -c $(OFLAGS)

$(DIR)/RTError.o: main/lang/RTError.cpp
	$(CC) -o $@ main/lang/RTError.cpp -c $(OFLAGS)

$(DIR)/Operator.o: main/lang/Operator.cpp
	$(CC) -o $@ main/lang/Operator.cpp -c $(OFLAGS)