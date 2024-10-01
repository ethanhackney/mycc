CFLAGS  = -std=c++11 -Wall -Werror -pedantic -fsanitize=address,undefined
SRC     = Main.cc Token.cc Error.cc Lexer.cc \
	  Ast.cc Parser.cc RegStk.cc CodeGen.cc Sym.cc SymTab.cc Type.cc
CC      = g++

all: $(SRC)
	$(CC) $(CFLAGS) $^
