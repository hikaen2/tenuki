CXX = clang++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++1z -O0 -g
#CXXFLAGS = -Wall -Wextra -pedantic -std=c++1z -O3 -DNDEBUG

tenuki: main.o position.o ponder.o move.o
	$(CXX) -lboost_system -o tenuki main.o position.o ponder.o move.o

#clean:
#	$(RM) hello
