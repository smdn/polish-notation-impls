CXX = g++
#CXX = clang++
CXXFLAGS = -std=c++23 -O2 -Wall

# For MinGW (Windows), specify `-lstdc++exp` to use std::print.
# ref: https://www.reddit.com/r/cpp_questions/comments/1bsh7nd/undefined_reference_to_std_open_terminal/
CXX_TARGET := $(shell $(CXX) -dumpmachine)

ifneq (,$(findstring w64-mingw32,$(CXX_TARGET)))
LDFLAGS += -lstdc++exp
endif

all: polish

polish: polish.o
	$(CXX) polish.o -o polish $(LDFLAGS)

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
