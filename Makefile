CXX = clang++
#CXXFLAGS = -Wall -Wextra -pedantic -std=c++14 -O0 -g
CXXFLAGS = -Wall -Wextra -pedantic -std=c++14 -O3 -DNDEBUG

tenuki: main.o position.o ponder.o move.o
	$(CXX) -lboost_system -lpthread -o tenuki main.o position.o ponder.o move.o

#clean:
#	$(RM) hello
