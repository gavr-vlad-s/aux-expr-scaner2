LINKER        = g++
LINKERFLAGS   =  -s
COMPILER      = g++
COMPILERFLAGS =  -std=c++14 -Wall
BIN           = aux-scaner-test
LIBS          = -lboost_filesystem -lboost_system
vpath %.cpp src
vpath %.o build
OBJ           = aux-scaner-test.o get_init_state.o search_char.o error_count.o aux_expr_scaner.o char_conv.o char_trie.o aux_expr_scaner_classes_table.o
LINKOBJ       = build/aux-scaner-test.o build/get_init_state.o build/search_char.o build/error_count.o build/aux_expr_scaner.o build/char_conv.o build/char_trie.o build/aux_expr_scaner_classes_table.o

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom 
	rm -f ./build/*.o
	rm -f ./build/$(BIN)

.cpp.o:
	$(COMPILER) -c $< -o $@ $(COMPILERFLAGS) 
	mv $@ ./build

$(BIN):$(OBJ)
	$(LINKER) -o $(BIN) $(LINKOBJ) $(LIBS) $(LINKERFLAGS)
	mv $(BIN) ./build