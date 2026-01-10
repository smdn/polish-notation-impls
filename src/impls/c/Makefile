CC = gcc
#CC = clang
CFLAGS = -std=c17 -O2 -Wall

all: polish

polish: polish.o
	$(CC) polish.o -o polish

polish.o: polish.c
	$(CC) $(CFLAGS) -c polish.c

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
	../../../tests/impls/run-tests.ps1 --target-impl c
