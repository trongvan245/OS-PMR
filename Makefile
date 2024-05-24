# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Targets and dependencies
all: OJ

OJ: main.o judger.o
	$(CXX) $(CXXFLAGS) -o OJ main.o judger.o

main.o: main.cpp judger.h
	$(CXX) $(CXXFLAGS) -c main.cpp

judger.o: judger.cpp judger.h
	$(CXX) $(CXXFLAGS) -c judger.cpp

clean:
	rm -f *.o OJ
