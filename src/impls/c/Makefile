CC     = gcc
#CC     = clang
CFLAGS = -O4 -Wall -I/usr/local/include -Wno-unused-result

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
