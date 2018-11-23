CC =	cc
CFLAGS =  -O0 -g3
LIBS = -lsqlite3 -lpthread -ldl

O_CMD = `find ./ -name '*.o'`

CORE_INCS =

build: create_insert.o prepare_insert.o prepare_select.o prepare_select2.o
	$(CC) -o create_insert create_insert.o $(LIBS)
	$(CC) -o prepare_insert prepare_insert.o $(LIBS)
	$(CC) -o prepare_select prepare_select.o $(LIBS)
	$(CC) -o prepare_select2 prepare_select2.o $(LIBS)
	rm -f $(O_CMD)


create_insert.o:
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o create_insert.o create_insert.c

prepare_insert.o:
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o prepare_insert.o prepare_insert.c

prepare_select.o:
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o prepare_select.o prepare_select.c

prepare_select2.o:
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o prepare_select2.o prepare_select2.c

clean:
	rm -f $(O_CMD)
	rm -f create_insert prepare_insert prepare_select prepare_select2
