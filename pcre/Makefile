CC =	cc
CFLAGS =  -O0 -g3
LIBS = -lpcre -ldlbase -lpcre2-8

O_CMD = `find ./ -name '*.o'`

CORE_INCS = -I /usr/include/dlbase

build: pcre.o pcre2.o
	$(CC) -o pcre.out pcre.o $(LIBS)
	$(CC) -o pcre2.out pcre2.o $(LIBS)
	rm -f $(O_CMD)


pcre.o:
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o pcre.o pcre.c

pcre2.o:
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o pcre2.o pcre2.c

clean:
	rm -f $(O_CMD)
	rm -f '*.out'
