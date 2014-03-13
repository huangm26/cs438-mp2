MANAGER_EXE = Manager
MANAGER_OBJS = Manager.o
LINKSTATE_EXE = linkstate
LINKSTATE_OBJS = linkstate.o
DISTVEC_EXE = distvec
DISTVEC_OBJS = distvec.o

COMPILER = g++
COMPILER_OPTS = -c -g -O0 -Wall -Werror -lpthread
LINKER = g++
LINKER_OPTS = -lpng -lpthread

.PHONY: all clean
all : $(MANAGER_EXE) $(LINKSTATE_EXE) $(DISTVEC_EXE)

$(MANAGER_EXE) : $(MANAGER_OBJS)
	$(LINKER) $(MANAGER_OBJS) $(LINKER_OPTS) -o $(MANAGER_EXE)

$(LINKSTATE_EXE) : $(LINKSTATE_OBJS)
	$(LINKER) $(LINKSTATE_OBJS) $(LINKER_OPTS) -o $(LINKSTATE_EXE)

$(DISTVEC_EXE) : $(DISTVEC_OBJS)
	$(LINKER) $(DISTVEC_OBJS) $(LINKER_OPTS) -o $(DISTVEC_EXE)

Manager.o : Manager.cpp 
	$(COMPILER) $(COMPILER_OPTS) Manager.cpp

linkstate.o : linkstate.cpp 
	$(COMPILER) $(COMPILER_OPTS) linkstate.cpp

distvec.o : distvec.cpp 
	$(COMPILER) $(COMPILER_OPTS) distvec.cpp


clean :
	-rm -f *.o $(MANAGER_EXE) $(LINKSTATE_EXE) $(DISTVEC_EXE) LINK
