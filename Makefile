locale=ENG

CV=--std=c++17

CC=g++ -D_LOCALIZED_ -D_LOCALE_$(locale)_ -Wall $(CV) -O3

ifeq ($(OS),Windows_NT)

LIB_EXT=.dll

BOOST_PATH_WIN=C:/boost
SDL_PATH_WIN=C:/SDL2/x86_64-w64-mingw32

BOOST_VERSION_WIN=1_74
SUFFIX_WIN=-mgw8-mt-x64-$(BOOST_VERSION_WIN)

SDL_IMAGE_PATH_WIN=C:/SDL2_image/x86_64-w64-mingw32
SDL_FLAGS=-I"$(SDL_PATH_WIN)/include/SDL2" -I"$(SDL_IMAGE_PATH_WIN)/include/SDL2" -L"$(SDL_PATH_WIN)/lib" -L"$(SDL_IMAGE_PATH_WIN)/lib" -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

CFLAGS=-L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN)
LFLAGS=-shared -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN)
OFLAGS=$(CFLAGS)

LIBNET_FLAGS=-lwsock32 -lws2_32 -lboost_system$(SUFFIX_WIN)

DIR=build\\win\\$(locale)

dir_target = $(DIR)-$(wildcard $(DIR))
dir_present = $(DIR)-$(DIR)
dir_absent = $(DIR)-

all: | $(dir_target)

$(dir_present): bin/rossa.exe bin/lib/libstd$(LIB_EXT)

$(dir_absent): $(DIR) bin/rossa.exe bin/lib/libstd$(LIB_EXT)

$(DIR):
	mkdir $@

else

LIB_EXT=.so

SDL_FLAGS=-lSDL2 -lSDL2_image
CFLAGS=-lboost_filesystem -lboost_system -ldl
LFLAGS=-fPIC -shared -lboost_filesystem -ldl
OFLAGS=-fPIC $(CFLAGS)

LIBNET_FLAGS=-lboost_system

DIR=build/nix/$(locale)

dir_target = $(DIR)-$(wildcard $(DIR))
dir_present = $(DIR)-$(DIR)
dir_absent = $(DIR)-

all: | $(dir_target)

$(dir_present): bin/rossa bin/lib/libstd$(LIB_EXT)

$(dir_absent): $(DIR) bin/rossa bin/lib/libstd$(LIB_EXT)

$(DIR):
	mkdir -p $@

endif

libs: libfs libnet libsdl

libfs: bin/lib/libfs$(LIB_EXT)

libnet: bin/lib/libnet$(LIB_EXT)

libsdl: bin/lib/libsdl$(LIB_EXT)

bin/lib/libstd$(LIB_EXT): src/ext/libstd.cpp $(DIR)/librossa.a
	$(CC) -o $@ src/ext/libstd.cpp $(DIR)/librossa.a $(LFLAGS)

bin/lib/libfs$(LIB_EXT): src/ext/libfs.cpp $(DIR)/librossa.a
	$(CC) -o $@ src/ext/libfs.cpp $(DIR)/librossa.a $(LFLAGS)

bin/lib/libnet$(LIB_EXT): src/ext/libnet.cpp $(DIR)/librossa.a
	$(CC) -o $@ src/ext/libnet.cpp $(DIR)/librossa.a $(LFLAGS) $(LIBNET_FLAGS)

bin/lib/libsdl$(LIB_EXT): src/ext/libsdl.cpp $(DIR)/librossa.a
	$(CC) -o $@ src/ext/libsdl.cpp $(DIR)/librossa.a $(LFLAGS) $(SDL_FLAGS)

bin/rossa.exe: src/Main.cpp $(DIR)/librossa.a
	$(CC) -o $@ src/Main.cpp $(DIR)/librossa.a $(CFLAGS)

bin/rossa: src/Main.cpp $(DIR)/librossa.a
	$(CC) -o $@ src/Main.cpp $(DIR)/librossa.a $(CFLAGS)

$(DIR)/librossa.a: $(DIR)/Rossa.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Signature.o $(DIR)/Value.o $(DIR)/Symbol.o $(DIR)/RTError.o
	ar rcs $@ $(DIR)/Rossa.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Signature.o $(DIR)/Value.o $(DIR)/Symbol.o $(DIR)/RTError.o

$(DIR)/Rossa.o: src/rossa/Rossa.cpp
	$(CC) -o $@ src/rossa/Rossa.cpp -c $(OFLAGS)

$(DIR)/Node.o: src/rossa/Node.cpp
	$(CC) -o $@ src/rossa/Node.cpp -c $(OFLAGS)

$(DIR)/NodeParser.o: src/rossa/NodeParser.cpp
	$(CC) -o $@ src/rossa/NodeParser.cpp -c $(OFLAGS)

$(DIR)/Parser.o: src/rossa/Parser.cpp
	$(CC) -o $@ src/rossa/Parser.cpp -c $(OFLAGS)

$(DIR)/Scope.o: src/rossa/Scope.cpp
	$(CC) -o $@ src/rossa/Scope.cpp -c $(OFLAGS)

$(DIR)/Function.o: src/rossa/Function.cpp
	$(CC) -o $@ src/rossa/Function.cpp -c $(OFLAGS)

$(DIR)/Signature.o: src/rossa/Signature.cpp
	$(CC) -o $@ src/rossa/Signature.cpp -c $(OFLAGS)

$(DIR)/Value.o: src/rossa/Value.cpp
	$(CC) -o $@ src/rossa/Value.cpp -c $(OFLAGS)

$(DIR)/Symbol.o: src/rossa/Symbol.cpp
	$(CC) -o $@ src/rossa/Symbol.cpp -c $(OFLAGS)

$(DIR)/RTError.o: src/rossa/RTError.cpp
	$(CC) -o $@ src/rossa/RTError.cpp -c $(OFLAGS)