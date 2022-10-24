CXX=g++
CXXFLAGS=-g -pthread -std=c++20
RM=rm -rf

all: clean ti.o
	$(CXX) $(CXXFLAGS) ti_api.o ti.o tb.o tb_api.o db_app.o ../../db/cs647_db.o

ti.o: ti.s
	$(CXX) $(CXXFLAGS) -c ti_api.s ti.s tb.s tb_api.s db_app.s

ti.s:
	$(CXX) $(CXXFLAGS) -S ti_api.cpp ti.cpp tb.cpp tb_api.cpp db_app.cpp

clean:
	$(RM) *.out *.s *.o
