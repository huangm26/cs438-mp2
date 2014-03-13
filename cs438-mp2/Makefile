EXE = Manager
OBJS = Manager.o

COMPILER = g++
COMPILER_OPTS = -c -g -O0 -Wall -Werror -lpthread
LINKER = g++
LINKER_OPTS = -lpng -lpthread

all : $(EXE)

$(EXE) : $(OBJS)
	$(LINKER) $(OBJS) $(LINKER_OPTS) -o $(EXE)

Manager.o : Manager.cpp 
	$(COMPILER) $(COMPILER_OPTS) Manager.cpp


clean :
	-rm -f *.o $(EXE)
