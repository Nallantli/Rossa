CC=g++

BOOST_PATH_WIN=C:/Program Files/boost
BOOST_VERSION_WIN=1_73
SUFFIX_WIN=-mgw8-mt-x64-$(BOOST_VERSION_WIN)

CFLAGS_WIN=-O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN) --std=gnu++17
CFLAGS=-O3 -lboost_filesystem --std=gnu++17 -ldl
LFLAGS_WIN=-shared -O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN) --std=gnu++17
LFLAGS=-fPIC -shared -O3 -lboost_filesystem --std=gnu++17 -ldl

win: bin/ruota.exe plugins-win

nix: bin/ruota plugins-nix

plugins-win: bin/lib/libstd.dll bin/lib/libfile.dll

plugins-nix: bin/lib/libstd.so bin/lib/libfile.so

bin/lib/libstd.dll: src/ext/libstd.cpp
	$(CC) -o bin/lib/libstd.dll src/ext/libstd.cpp $(LFLAGS_WIN)

bin/lib/libfile.dll: src/ext/libfile.cpp
	$(CC) -o bin/lib/libfile.dll src/ext/libfile.cpp $(LFLAGS_WIN)

bin/lib/libstd.so: src/ext/libstd.cpp
	$(CC) -o bin/lib/libstd.so src/ext/libstd.cpp $(LFLAGS)

bin/lib/libfile.so: src/ext/libfile.cpp
	$(CC) -o bin/lib/libfile.so src/ext/libfile.cpp $(LFLAGS)

bin/ruota.exe: build/win/Main.o build/win/Ruota.o build/win/Node.o build/win/NodeParser.o build/win/Lexer.o build/win/Parser.o build/win/Scope.o build/win/Function.o build/win/Object.o
	$(CC) -o bin/ruota.exe build/win/Main.o build/win/Ruota.o build/win/Node.o build/win/NodeParser.o build/win/Lexer.o build/win/Parser.o build/win/Scope.o build/win/Function.o build/win/Object.o $(CFLAGS_WIN)

bin/ruota: build/nix/Main.o build/nix/Ruota.o build/nix/Node.o build/nix/NodeParser.o build/nix/Lexer.o build/nix/Parser.o build/nix/Scope.o build/nix/Function.o build/nix/Object.o
	$(CC) -o bin/ruota build/nix/Main.o build/nix/Ruota.o build/nix/Node.o build/nix/NodeParser.o build/nix/Lexer.o build/nix/Parser.o build/nix/Scope.o build/nix/Function.o build/nix/Object.o $(CFLAGS)

build/win/Main.o: src/Main.cpp
	$(CC) src/Main.cpp -o build/win/Main.o -c $(CFLAGS_WIN)

build/win/Ruota.o: src/ruota/Ruota.cpp
	$(CC) src/ruota/Ruota.cpp -o build/win/Ruota.o -c $(CFLAGS_WIN)

build/win/Node.o: src/ruota/Node.cpp
	$(CC) src/ruota/Node.cpp -o build/win/Node.o -c $(CFLAGS_WIN)

build/win/NodeParser.o: src/ruota/NodeParser.cpp
	$(CC) src/ruota/NodeParser.cpp -o build/win/NodeParser.o -c $(CFLAGS_WIN)

build/win/Lexer.o: src/ruota/Lexer.cpp
	$(CC) src/ruota/Lexer.cpp -o build/win/Lexer.o -c $(CFLAGS_WIN)

build/win/Parser.o: src/ruota/Parser.cpp
	$(CC) src/ruota/Parser.cpp -o build/win/Parser.o -c $(CFLAGS_WIN)

build/win/Scope.o: src/ruota/Scope.cpp
	$(CC) src/ruota/Scope.cpp -o build/win/Scope.o -c $(CFLAGS_WIN)

build/win/Function.o: src/ruota/Function.cpp
	$(CC) src/ruota/Function.cpp -o build/win/Function.o -c $(CFLAGS_WIN)

build/win/Object.o: src/ruota/Object.cpp
	$(CC) src/ruota/Object.cpp -o build/win/Object.o -c $(CFLAGS_WIN)

build/nix/Main.o: src/Main.cpp
	$(CC) src/Main.cpp -o build/nix/Main.o -c $(CFLAGS)

build/nix/Ruota.o: src/ruota/Ruota.cpp
	$(CC) src/ruota/Ruota.cpp -o build/nix/Ruota.o -c $(CFLAGS)

build/nix/Node.o: src/ruota/Node.cpp
	$(CC) src/ruota/Node.cpp -o build/nix/Node.o -c $(CFLAGS)

build/nix/NodeParser.o: src/ruota/NodeParser.cpp
	$(CC) src/ruota/NodeParser.cpp -o build/nix/NodeParser.o -c $(CFLAGS)

build/nix/Lexer.o: src/ruota/Lexer.cpp
	$(CC) src/ruota/Lexer.cpp -o build/nix/Lexer.o -c $(CFLAGS)

build/nix/Parser.o: src/ruota/Parser.cpp
	$(CC) src/ruota/Parser.cpp -o build/nix/Parser.o -c $(CFLAGS)

build/nix/Scope.o: src/ruota/Scope.cpp
	$(CC) src/ruota/Scope.cpp -o build/nix/Scope.o -c $(CFLAGS)

build/nix/Function.o: src/ruota/Function.cpp
	$(CC) src/ruota/Function.cpp -o build/nix/Function.o -c $(CFLAGS)

build/nix/Object.o: src/ruota/Object.cpp
	$(CC) src/ruota/Object.cpp -o build/nix/Object.o -c $(CFLAGS)