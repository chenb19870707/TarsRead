.PHONY=clean all
CPP=g++
CPPFLAGS=-o3
BIN=example_tc_autoptr
OBJS=example_tc_autoptr.o tc_ex.o
$(BIN):$(OBJS)
	$(CPP) $(CPPFLAGS)  $^ -o $@ $(LIBS)
%.o:%.c
	$(CPP) $(CPPFLAGS) -c $< -o $@ 
clean:
	rm -rf *.o $(BIN)