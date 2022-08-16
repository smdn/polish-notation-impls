CC     = g++
#CC     = clang++
CFLAGS = -std=c++2a -O4 -Wall -I/usr/local/include

all: polish

polish: polish.o
	$(CC) polish.o -o polish

polish.o: polish.cpp
	$(CC) $(CFLAGS) -c polish.cpp

clean:
	rm -f *.o polish

run: polish
	@if [ -z "${INPUT}" ]; then \
		./polish; \
	fi

	@if [ -n "${INPUT}" ]; then \
		echo ${INPUT} | ./polish; \
	fi
