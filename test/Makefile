CXX = clang++
#CXXFLAGS = -Wall -Wextra -pedantic -std=c++14 -O0 -g
CXXFLAGS = -Wall -Wextra -pedantic -std=c++14 -Ofast -march=native -DNDEBUG

test: test.o ../position.o ../ponder.o ../move.o
	$(CXX) -lboost_system -lpthread -o test test.o ../position.o ../ponder.o ../move.o

test2: test2.o ../position.o ../ponder.o ../move.o
	$(CXX) -lboost_system -lpthread -o test2 test2.o ../position.o ../ponder.o ../move.o

#clean:
#	$(RM) hello
