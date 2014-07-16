SRC := $(wildcard *.cpp) $(wildcard network/*.cpp) $(wildcard comm/*.cpp) 
INC := -I../src
LIB := -lpthread third_party/jemalloc-3.6.0/lib/libjemalloc.a
OUT := libfishfcgi.a
OBJ := $(notdir $(patsubst %.cpp,%.o,$(SRC)))
CXXFLAGS := -ggdb -std=c++0x -Wall -O3
.PHONY:all clean .depend
all:
	$(MAKE) .depend
	$(MAKE) $(OUT)

$(OUT):$(OBJ)
	ar -rc $(OUT) $(OBJ)

include .depend
	
%.o:
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(INC)

.depend:$(SRC)
	$(CXX) -MM $(SRC) $(INC) > .depend

clean:
	-rm $(OUT) *.o .depend
