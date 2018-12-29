CC =	cc
CFLAGS =  -O0 -g3
LIBS = -lpcre -ldlbase

O_CMD = `find ./ -name '*.o'`

CORE_INCS = -I /usr/include/dlbase

build: pcre.o
	$(CC) -o pcre.out pcre.o $(LIBS)
	rm -f $(O_CMD)


pcre.o:
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o pcre.o pcre.c

clean:
	rm -f $(O_CMD)
	rm -f '*.out'
