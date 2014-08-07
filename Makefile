INCLUDE	+= -Iparser/
SRC	+= parser/*.c
CFLAGS	+= -Wall --std=c99 -O $(INCLUDE)

.PHONY: all clean install
all:
	$(CC) $(CFLAGS) -o HSDL *.c $(SRC) $(LDFLAGS)

clean:
	rm -f HSDL
