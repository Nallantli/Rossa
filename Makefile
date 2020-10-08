locale=ENG

CC=g++ -D_LOCALIZED_ -D_LOCALE_$(locale)_

WINDIR=build\\win\\$(locale)
NIXDIR=build/nix/$(locale)

BOOST_PATH_WIN=C:\\boost
BOOST_VERSION_WIN=1_73
SUFFIX_WIN=-mgw8-mt-x64-$(BOOST_VERSION_WIN)

SDL_PATH_WIN=C:\\SDL2\\x86_64-w64-mingw32
SDL_FLAGS_WIN=-I"$(SDL_PATH_WIN)/include" -L"$(SDL_PATH_WIN)/lib" -lmingw32 -lSDL2main -lSDL2
SDL_FLAGS=-lSDL2

CFLAGS_WIN=-O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN) --std=gnu++17
CFLAGS=-O3 -lboost_filesystem -lboost_system --std=gnu++17 -ldl
LFLAGS_WIN=-shared -O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN) --std=gnu++17
LFLAGS=-fPIC -shared -O3 -lboost_filesystem --std=gnu++17 -ldl

LIBNET_FLAGS_WIN=-lwsock32 -lws2_32 -lboost_system$(SUFFIX_WIN)
LIBNET_FLAGS_NIX=-lboost_system

ifeq ($(OS),Windows_NT)

dir_target = $(WINDIR)-$(wildcard $(WINDIR))
dir_present = $(WINDIR)-$(WINDIR)
dir_absent = $(WINDIR)-

all: | $(dir_target)

$(dir_present): win

$(dir_absent): $(WINDIR) win

$(WINDIR):
	mkdir $@

else

dir_target = $(NIXDIR)-$(wildcard $(NIXDIR))
dir_present = $(NIXDIR)-$(NIXDIR)
dir_absent = $(NIXDIR)-

all: | $(dir_target)

$(dir_present): nix

$(dir_absent): $(NIXDIR) nix

$(NIXDIR):
	mkdir -p $@

endif

win: bin/ruota.exe plugins-win

nix: bin/ruota plugins-nix

plugins-win: bin/lib/libstd.dll bin/lib/libfs.dll bin/lib/libnet.dll bin/lib/libsdl.dll

plugins-nix: bin/lib/libstd.so bin/lib/libfs.so bin/lib/libnet.so bin/lib/libsdl.so

bin/lib/libstd.dll: src/ext/libstd.cpp
	$(CC) -o bin/lib/libstd.dll src/ext/libstd.cpp $(LFLAGS_WIN)

bin/lib/libfs.dll: src/ext/libfs.cpp
	$(CC) -o bin/lib/libfs.dll src/ext/libfs.cpp $(LFLAGS_WIN)

bin/lib/libnet.dll: src/ext/libnet.cpp
	$(CC) -o bin/lib/libnet.dll src/ext/libnet.cpp $(LFLAGS_WIN) $(LIBNET_FLAGS_WIN)

bin/lib/libsdl.dll: src/ext/libsdl.cpp
	$(CC) -o bin/lib/libsdl.dll src/ext/libsdl.cpp $(LFLAGS_WIN) $(SDL_FLAGS_WIN)

bin/lib/libstd.so: src/ext/libstd.cpp
	$(CC) -o bin/lib/libstd.so src/ext/libstd.cpp $(LFLAGS)

bin/lib/libfs.so: src/ext/libfs.cpp
	$(CC) -o bin/lib/libfs.so src/ext/libfs.cpp $(LFLAGS)

bin/lib/libnet.so: src/ext/libnet.cpp
	$(CC) -o bin/lib/libnet.so src/ext/libnet.cpp $(LFLAGS) $(LIBNET_FLAGS_NIX)

bin/lib/libsdl.so: src/ext/libsdl.cpp
	$(CC) -o bin/lib/libsdl.so src/ext/libsdl.cpp $(LFLAGS) $(SDL_FLAGS)

bin/ruota.exe: $(WINDIR)/Main.o $(WINDIR)/Ruota.o $(WINDIR)/Node.o $(WINDIR)/NodeParser.o $(WINDIR)/Lexer.o $(WINDIR)/Parser.o $(WINDIR)/Scope.o $(WINDIR)/Function.o $(WINDIR)/Object.o $(WINDIR)/Token.o $(WINDIR)/Signature.o
	$(CC) -o bin/ruota.exe $(WINDIR)/Main.o $(WINDIR)/Ruota.o $(WINDIR)/Node.o $(WINDIR)/NodeParser.o $(WINDIR)/Lexer.o $(WINDIR)/Parser.o $(WINDIR)/Scope.o $(WINDIR)/Function.o $(WINDIR)/Object.o $(WINDIR)/Token.o $(WINDIR)/Signature.o $(CFLAGS_WIN)

bin/ruota: $(NIXDIR)/Main.o $(NIXDIR)/Ruota.o $(NIXDIR)/Node.o $(NIXDIR)/NodeParser.o $(NIXDIR)/Lexer.o $(NIXDIR)/Parser.o $(NIXDIR)/Scope.o $(NIXDIR)/Function.o $(NIXDIR)/Object.o $(NIXDIR)/Token.o $(NIXDIR)/Signature.o
	$(CC) -o bin/ruota $(NIXDIR)/Main.o $(NIXDIR)/Ruota.o $(NIXDIR)/Node.o $(NIXDIR)/NodeParser.o $(NIXDIR)/Lexer.o $(NIXDIR)/Parser.o $(NIXDIR)/Scope.o $(NIXDIR)/Function.o $(NIXDIR)/Object.o $(NIXDIR)/Token.o $(NIXDIR)/Signature.o $(CFLAGS)

$(WINDIR)/Main.o: src/Main.cpp
	$(CC) src/Main.cpp -o $(WINDIR)/Main.o -c $(CFLAGS_WIN)

$(WINDIR)/Ruota.o: src/ruota/Ruota.cpp
	$(CC) src/ruota/Ruota.cpp -o $(WINDIR)/Ruota.o -c $(CFLAGS_WIN)

$(WINDIR)/Node.o: src/ruota/Node.cpp
	$(CC) src/ruota/Node.cpp -o $(WINDIR)/Node.o -c $(CFLAGS_WIN)

$(WINDIR)/NodeParser.o: src/ruota/NodeParser.cpp
	$(CC) src/ruota/NodeParser.cpp -o $(WINDIR)/NodeParser.o -c $(CFLAGS_WIN)

$(WINDIR)/Lexer.o: src/ruota/Lexer.cpp
	$(CC) src/ruota/Lexer.cpp -o $(WINDIR)/Lexer.o -c $(CFLAGS_WIN)

$(WINDIR)/Parser.o: src/ruota/Parser.cpp
	$(CC) src/ruota/Parser.cpp -o $(WINDIR)/Parser.o -c $(CFLAGS_WIN)

$(WINDIR)/Scope.o: src/ruota/Scope.cpp
	$(CC) src/ruota/Scope.cpp -o $(WINDIR)/Scope.o -c $(CFLAGS_WIN)

$(WINDIR)/Function.o: src/ruota/Function.cpp
	$(CC) src/ruota/Function.cpp -o $(WINDIR)/Function.o -c $(CFLAGS_WIN)

$(WINDIR)/Object.o: src/ruota/Object.cpp
	$(CC) src/ruota/Object.cpp -o $(WINDIR)/Object.o -c $(CFLAGS_WIN)

$(WINDIR)/Token.o: src/ruota/Token.cpp
	$(CC) src/ruota/Token.cpp -o $(WINDIR)/Token.o -c $(CFLAGS_WIN)

$(WINDIR)/Signature.o: src/ruota/Signature.cpp
	$(CC) src/ruota/Signature.cpp -o $(WINDIR)/Signature.o -c $(CFLAGS_WIN)


$(NIXDIR)/Main.o: src/Main.cpp
	$(CC) src/Main.cpp -o $(NIXDIR)/Main.o -c $(CFLAGS)

$(NIXDIR)/Ruota.o: src/ruota/Ruota.cpp
	$(CC) src/ruota/Ruota.cpp -o $(NIXDIR)/Ruota.o -c $(CFLAGS)

$(NIXDIR)/Node.o: src/ruota/Node.cpp
	$(CC) src/ruota/Node.cpp -o $(NIXDIR)/Node.o -c $(CFLAGS)

$(NIXDIR)/NodeParser.o: src/ruota/NodeParser.cpp
	$(CC) src/ruota/NodeParser.cpp -o $(NIXDIR)/NodeParser.o -c $(CFLAGS)

$(NIXDIR)/Lexer.o: src/ruota/Lexer.cpp
	$(CC) src/ruota/Lexer.cpp -o $(NIXDIR)/Lexer.o -c $(CFLAGS)

$(NIXDIR)/Parser.o: src/ruota/Parser.cpp
	$(CC) src/ruota/Parser.cpp -o $(NIXDIR)/Parser.o -c $(CFLAGS)

$(NIXDIR)/Scope.o: src/ruota/Scope.cpp
	$(CC) src/ruota/Scope.cpp -o $(NIXDIR)/Scope.o -c $(CFLAGS)

$(NIXDIR)/Function.o: src/ruota/Function.cpp
	$(CC) src/ruota/Function.cpp -o $(NIXDIR)/Function.o -c $(CFLAGS)

$(NIXDIR)/Object.o: src/ruota/Object.cpp
	$(CC) src/ruota/Object.cpp -o $(NIXDIR)/Object.o -c $(CFLAGS)

$(NIXDIR)/Token.o: src/ruota/Token.cpp
	$(CC) src/ruota/Token.cpp -o $(NIXDIR)/Token.o -c $(CFLAGS)

$(NIXDIR)/Signature.o: src/ruota/Signature.cpp
	$(CC) src/ruota/Signature.cpp -o $(NIXDIR)/Signature.o -c $(CFLAGS)