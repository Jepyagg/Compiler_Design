ASTDIR 		:= src/AST/
AST 		:= $(shell find $(ASTDIR) -name '*.cpp')

VSTDIR		:= src/VST/
VST 		:= $(shell find $(VSTDIR) -name '*.cpp')

CPP_FILES 	:= $(AST) $(VST)

all: parser

parser: parser.c scanner.c $(CPP_FILES)
	g++ -o parser parser.c scanner.c $(CPP_FILES) -Iinclude -lfl -ly 

parser.c: parser.y
	bison -o parser.c --defines=parser.h -v -d parser.y

scanner.c: scanner.l
	flex -o scanner.c scanner.l

clean:
	rm parser.c scanner.c parser.output parser parser.h