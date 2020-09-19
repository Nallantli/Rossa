CC=g++

BOOST_PATH_WIN=C:/Program Files/boost
BOOST_VERSION_WIN=1_73
SUFFIX_WIN=-mgw8-mt-x64-$(BOOST_VERSION_WIN)

CFLAGS_WIN=-O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN)
CFLAGS=-O3
LFLAGS_WIN=-shared -O3 -L"$(BOOST_PATH_WIN)/lib" -I"$(BOOST_PATH_WIN)/include/boost-$(BOOST_VERSION_WIN)" -lboost_filesystem$(SUFFIX_WIN)
LFLAGS=-fPIC -shared -O3

all: win

win: bin/ruota.exe plugins

plugins: bin/lib/libsystem.dll

bin/lib/libsystem.dll: src/ext/libsystem.cpp
	$(CC) -o bin/lib/libsystem.dll src/ext/libsystem.cpp $(LFLAGS_WIN)

bin/ruota.exe: build/win/Main.o build/win/Ruota.o build/win/Node.o build/win/Lexer.o build/win/Parser.o build/win/Scope.o build/win/Function.o
	$(CC) -o bin/ruota.exe build/win/Main.o build/win/Ruota.o build/win/Node.o build/win/Lexer.o build/win/Parser.o build/win/Scope.o build/win/Function.o $(CFLAGS_WIN)

build/win/Main.o: src/Main.cpp
	$(CC) src/Main.cpp -o build/win/Main.o -c $(CFLAGS_WIN)

build/win/Ruota.o: src/ruota/Ruota.cpp
	$(CC) src/ruota/Ruota.cpp -o build/win/Ruota.o -c $(CFLAGS_WIN)

build/win/Node.o: src/ruota/Node.cpp
	$(CC) src/ruota/Node.cpp -o build/win/Node.o -c $(CFLAGS_WIN)

build/win/Lexer.o: src/ruota/Lexer.cpp
	$(CC) src/ruota/Lexer.cpp -o build/win/Lexer.o -c $(CFLAGS_WIN)

build/win/Parser.o: src/ruota/Parser.cpp
	$(CC) src/ruota/Parser.cpp -o build/win/Parser.o -c $(CFLAGS_WIN)

build/win/Scope.o: src/ruota/Scope.cpp
	$(CC) src/ruota/Scope.cpp -o build/win/Scope.o -c $(CFLAGS_WIN)

build/win/Function.o: src/ruota/Function.cpp
	$(CC) src/ruota/Function.cpp -o build/win/Function.o -c $(CFLAGS_WIN)