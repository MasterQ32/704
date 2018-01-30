
PREFIX=704
PGM_ASM=$(PREFIX)-as



all: $(PGM_ASM)


$(PGM_ASM): main.cpp opcodes.cpp parser.cpp tokens.cpp
	$(CXX) -g -o $@ $^

opcodes.cpp: opcodes.gen ../docs/opcodes.txt
	lua $^ > $@

parser.cpp: parser.l
	flex -8 -i \
		--header-file=$(basename $@).hpp \
		--yylineno \
		--prefix=parser_ \
		--stdinit \
		--noyywrap \
		-o $@ $<