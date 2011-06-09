SRC	= time.c

OBJ	= malloc.o firstfit.o bestfit.o worstfit.o quickfit.o

BIN	= time time_system time_firstfit time_bestfit time_worstfit time_quickfit

CFLAGS	= -g -Wall -ansi

CC	= gcc 


all: time_system time_firstfit time_bestfit time_worstfit time_quickfit 
     .PHONY : all

time_system:
	$(CC) $(CFLAGS) -Werror -o $@ time.c
time_firstfit: firstfit.o
	$(CC) $(CFLAGS) -Werror -o $@ firstfit.o time.c
time_bestfit: bestfit.o
	$(CC) $(CFLAGS) -Werror -o $@ bestfit.o time.c
time_worstfit: worstfit.o
	$(CC) $(CFLAGS) -Werror -o $@ worstfit.o time.c
time_quickfit: quickfit.o
	$(CC) $(CFLAGS) -Werror -o $@ quickfit.o time.c

firstfit.o:
	$(CC) $(CFLAGS) -c -Werror -o $@ -DSTRATEGY=1 ../malloc.c
bestfit.o:
	$(CC) $(CFLAGS) -c -Werror -o $@ -DSTRATEGY=2 ../malloc.c
worstfit.o:
	$(CC) $(CFLAGS) -c -Werror -o $@ -DSTRATEGY=3 ../malloc.c
quickfit.o:
	$(CC) $(CFLAGS) -c -Werror -o $@ -DSTRATEGY=4 ../malloc.c

clean:
	\rm -f $(BIN) $(OBJ) core

cleanall: clean
	\rm -f *~

