
CXX	= g++
CFLAGS	= -w -g -std=c++11

hw1 : hw1.cpp
	$(CXX) -o $@ $(CFLAGS) $< 

clean:
	rm -f *~ $(PROGS)
