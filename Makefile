locale=ENG

CC=g++ -D_LOCALIZED_ -D_LOCALE_$(locale)_ -Wall

ifeq ($(OS),Windows_NT)

LIB_EXT=.dll

BOOST_PATH_WIN=C:/boost
SDL_PATH_WIN=C:/SDL2/x86_64-w64-mingw32

BOOST_VERSION_WIN=1_73
SUFFIX_WIN=-mgw8-mt-x64-$(BOOST_VERSION_WIN)

SDL_IMAGE_PATH_WIN=C:/SDL2_image/x86_64-w64-mingw32
SDL_FLAGS=-I"$(SDL_PATH_WIN)/include" -I"$(SDL_IMAGE_PATH_WIN)/include/SDL2" -L"$(SDL_PATH_WIN)/lib" -L"$(SDL_IMAGE_PATH_WIN)/lib" -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

CFLAGS=-O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN) --std=gnu++17
LFLAGS=-shared -O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN) --std=gnu++17

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
CFLAGS=-O3 -lboost_filesystem -lboost_system --std=gnu++17 -ldl
LFLAGS=-fPIC -shared -O3 -lboost_filesystem --std=gnu++17 -ldl

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

bin/ruota.exe: $(DIR)/Main.o $(DIR)/Ruota.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Lexer.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Object.o $(DIR)/Token.o $(DIR)/Signature.o
	$(CC) -o bin/ruota.exe $(DIR)/Main.o $(DIR)/Ruota.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Lexer.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Object.o $(DIR)/Token.o $(DIR)/Signature.o $(CFLAGS)

bin/ruota: $(DIR)/Main.o $(DIR)/Ruota.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Lexer.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Object.o $(DIR)/Token.o $(DIR)/Signature.o
	$(CC) -o bin/ruota $(DIR)/Main.o $(DIR)/Ruota.o $(DIR)/Node.o $(DIR)/NodeParser.o $(DIR)/Lexer.o $(DIR)/Parser.o $(DIR)/Scope.o $(DIR)/Function.o $(DIR)/Object.o $(DIR)/Token.o $(DIR)/Signature.o $(CFLAGS)

bin/lib/libstd$(LIB_EXT): src/ext/libstd.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) -o bin/lib/libstd$(LIB_EXT) src/ext/libstd.cpp $(LFLAGS)

bin/lib/libfs$(LIB_EXT): src/ext/libfs.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) -o bin/lib/libfs$(LIB_EXT) src/ext/libfs.cpp $(LFLAGS)

bin/lib/libnet$(LIB_EXT): src/ext/libnet.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) -o bin/lib/libnet$(LIB_EXT) src/ext/libnet.cpp $(LFLAGS) $(LIBNET_FLAGS)

bin/lib/libsdl$(LIB_EXT): src/ext/libsdl.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) -o bin/lib/libsdl$(LIB_EXT) src/ext/libsdl.cpp $(LFLAGS) $(SDL_FLAGS)

$(DIR)/Main.o: src/Main.cpp src/ruota/Node.h src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/Main.cpp -o $(DIR)/Main.o -c $(CFLAGS)

$(DIR)/Ruota.o: src/ruota/Ruota.cpp src/ruota/Node.h src/ruota/NodeParser.h src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Ruota.cpp -o $(DIR)/Ruota.o -c $(CFLAGS)

$(DIR)/Node.o: src/ruota/Node.cpp src/ruota/Node.h src/ruota/Parser.h src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Node.cpp -o $(DIR)/Node.o -c $(CFLAGS)

$(DIR)/NodeParser.o: src/ruota/NodeParser.cpp src/ruota/NodeParser.h src/ruota/Library.h src/ruota/Node.h src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/NodeParser.cpp -o $(DIR)/NodeParser.o -c $(CFLAGS)

$(DIR)/Lexer.o: src/ruota/Lexer.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Lexer.cpp -o $(DIR)/Lexer.o -c $(CFLAGS)

$(DIR)/Parser.o: src/ruota/Parser.cpp src/ruota/Parser.h src/ruota/Library.h src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Parser.cpp -o $(DIR)/Parser.o -c $(CFLAGS)

$(DIR)/Scope.o: src/ruota/Scope.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Scope.cpp -o $(DIR)/Scope.o -c $(CFLAGS)

$(DIR)/Function.o: src/ruota/Function.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Function.cpp -o $(DIR)/Function.o -c $(CFLAGS)

$(DIR)/Object.o: src/ruota/Object.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Object.cpp -o $(DIR)/Object.o -c $(CFLAGS)

$(DIR)/Token.o: src/ruota/Token.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Token.cpp -o $(DIR)/Token.o -c $(CFLAGS)

$(DIR)/Signature.o: src/ruota/Signature.cpp src/ruota/Ruota.h src/ruota/Locale.h src/ruota/Declarations.h src/ruota/CNumber.h
	$(CC) src/ruota/Signature.cpp -o $(DIR)/Signature.o -c $(CFLAGS)