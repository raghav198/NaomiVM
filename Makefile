CFLAGS = -std=c99 -g -Wall -Wshadow --pedantic -Wvla -Wunreachable-code
OBJS = main.o nmi.o utils.o
HEADERS = nmi.h utils.h
all: naomi
naomi: $(OBJS) main.o
	gcc $(CFLAGS) *.o -o naomi
test: naomi
	./naomi

clean:
	/bin/rm -rf *.o
	/bin/rm -rf naomi
	/bin/rm -rf a.out*
%.o: %.c $(HEADERS)
	gcc -c $< -o $@ $(CFLAGS) $(tests)