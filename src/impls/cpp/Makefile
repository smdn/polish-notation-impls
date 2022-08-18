CXX = g++
#CXX = clang++
CFLAGS = -std=c++2a -O4 -Wall -I/usr/local/include

all: polish

polish: polish.o
	$(CXX) polish.o -o polish

polish.o: polish.cpp
	$(CXX) $(CFLAGS) -c polish.cpp

clean:
	rm -f *.o polish

run: polish
	@if [ -z "${INPUT}" ]; then \
		./polish; \
	fi

	@if [ -n "${INPUT}" ]; then \
		echo ${INPUT} | ./polish; \
	fi
