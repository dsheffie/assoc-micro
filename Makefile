OBJ = assoc_micro.o
CXX = g++
EXE = assoc_micro
OPT = -O2
CXXFLAGS = -std=c++11 -g $(OPT)
DEP = $(OBJ:.o=.d)

.PHONY: all clean

all: $(EXE)

$(EXE) : $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LIBS) -o $(EXE)

%.o: %.cc
	$(CXX) -MMD $(CXXFLAGS) -c $< 

-include $(DEP)

clean:
	rm -f $(EXE) $(OBJ) $(DEP) *.csv *~
