CXX = g++
#CXX = clang++
CXXFLAGS = -std=c++2a -O2 -Wall

all: polish

polish: polish.o
	$(CXX) polish.o -o polish

polish.o: polish.cpp
	$(CXX) $(CXXFLAGS) -c polish.cpp

clean:
	rm -f *.o polish

run: polish
	@if [ -z "${INPUT}" ]; then \
		./polish; \
	fi

	@if [ -n "${INPUT}" ]; then \
		echo ${INPUT} | ./polish; \
	fi

test:
	../../../tests/impls/run-tests.ps1 --target-impl cpp
