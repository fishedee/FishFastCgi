SRC := main.cpp ../src/FastCgi.cpp ../src/FastCgiSerialize.cpp
INC := -I../include -I../src
LIB := -lpthread
OUT := fastcgi
OBJ := $(notdir $(patsubst %.cpp,%.o,$(SRC)))
CXXFLAGS := -ggdb -std=c++0x
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
