SRC:=main.cpp
BIN:=dpll-sat
all:
	clang++ -std=c++14 -O3 ${SRC} -o ${BIN}
debug:
	g++-6 -std=c++14 ${SRC} -O0 -g -fsanitize=address,undefined -D_GLIB_CXX_DEBGUG -o ${BIN}
clean:
	rm ${BIN}
