CC		=	gcc -Wall
CFLAGS  =   -I../../allegro/allegro4/include -I./mpq -Wall -g  -I/usr/local/include
LIBS    =   /usr/local/lib/libiksemel.a ../../allegro/allegro4/lib/*.a -lm -lpthread -lX11 -lXcursor -lXcursor -ldl -lGL -L/usr/local/lib
OBJS    =   ./mpq/dcl_tbl.o ./mpq/explode.o ./mpq/mpqview.o ./mpq/wav_unp.o  ds1misc.o dt1_draw.o dt1misc.o gfx_custom.o inicreat.o iniread.o interfac.o main.o misc.o txtread.o wpreview.o
all:$(OBJS)
	$(CC) $(OBJS)  $(CFLAGS) $(LIBS) -o ../bin/mapedit
test:
	make test -C ../bin
debug:
	make debug -C ../bin
clean:
	rm  *.o ./mpq/*.o ../bin/mapedit -f
$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
