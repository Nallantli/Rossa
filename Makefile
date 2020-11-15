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

$(dir_present): bin/ruota.exe plugins

$(dir_absent): $(DIR) bin/ruota.exe plugins

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

$(dir_present): bin/ruota plugins

$(dir_absent): $(DIR) bin/ruota plugins

$(DIR):
	mkdir -p $@

endif

plugins: bin/lib/libstd$(LIB_EXT) bin/lib/libfs$(LIB_EXT) bin/lib/libnet$(LIB_EXT) bin/lib/libsdl$(LIB_EXT)

bin/lib/libstd$(LIB_EXT): src/ext/libstd.cpp $(DIR)/libruota.a
	$(CC) -o $@ src/ext/libstd.cpp $(DIR)/libruota.a $(LFLAGS)

bin/lib/libfs$(LIB_EXT): src/ext/libfs.cpp $(DIR)/libruota.a
	$(CC) -o $@ src/ext/libfs.cpp $(DIR)/libruota.a $(LFLAGS)

bin/lib/libnet$(LIB_EXT): src/ext/libnet.cpp $(DIR)/libruota.a
	$(CC) -o $@ src/ext/libnet.cpp $(DIR)/libruota.a $(LFLAGS) $(LIBNET_FLAGS)

bin/lib/libsdl$(LIB_EXT): src/ext/libsdl.cpp $(DIR)/libruota.a
	$(CC) -o $@ src/ext/libsdl.cpp $(DIR)/libruota.a $(LFLAGS) $(SDL_FLAGS)

bin/ruota.exe: src/Main.cpp $(DIR)/libruota.a
	$(CC) -o $@ src/Main.cpp $(DIR)/libruota.a $(CFLAGS)

bin/ruota: src/Main.cpp $(DIR)/libruota.a
	$(CC) -o $@ src/Main.cpp $(DIR)/libruota.a $(CFLAGS)

$(DIR)/libruota.a: $(DIR)/Ruota.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Lexer.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Signature.o
	ar rcs $@ $(DIR)/Ruota.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Lexer.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Signature.o

$(DIR)/Ruota.o: src/ruota/Ruota.cpp
	$(CC) -o $@ src/ruota/Ruota.cpp -c $(OFLAGS)

$(DIR)/Node.o: src/ruota/Node.cpp
	$(CC) -o $@ src/ruota/Node.cpp -c $(OFLAGS)

$(DIR)/NodeParser.o: src/ruota/NodeParser.cpp
	$(CC) -o $@ src/ruota/NodeParser.cpp -c $(OFLAGS)

$(DIR)/Lexer.o: src/ruota/Lexer.cpp
	$(CC) -o $@ src/ruota/Lexer.cpp -c $(OFLAGS)

$(DIR)/Parser.o: src/ruota/Parser.cpp
	$(CC) -o $@ src/ruota/Parser.cpp -c $(OFLAGS)

$(DIR)/Scope.o: src/ruota/Scope.cpp
	$(CC) -o $@ src/ruota/Scope.cpp -c $(OFLAGS)

$(DIR)/Function.o: src/ruota/Function.cpp
	$(CC) -o $@ src/ruota/Function.cpp -c $(OFLAGS)

$(DIR)/Signature.o: src/ruota/Signature.cpp
	$(CC) -o $@ src/ruota/Signature.cpp -c $(OFLAGS)