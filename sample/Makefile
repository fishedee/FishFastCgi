SRC := main.cpp
INC := -I../src
LIB := -L../src -lfishfcgi -lpthread
OUT := fastcgi
OBJ := $(notdir $(patsubst %.cpp,%.o,$(SRC)))
CXXFLAGS := -ggdb -std=c++0x -Wall -O3
.PHONY:all clean
all:
	$(MAKE) .depend
	$(MAKE) $(OUT)

$(OUT):$(OBJ)
	$(CXX) $(OBJ) -o $(OUT) $(CXXFLAGS) $(LIB)

include .depend
	
%.o:
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(INC)

.depend:$(SRC)
	$(CXX) -MM $(SRC) $(INC) > .depend

clean:
	-rm $(OUT) *.o
