OBJS=main.o util.o cli_gfx.o main_cli.o main_gui.o colors.o shader.o debug.o

DEPS=glu glfw3 glew freetype2 glm
CFLAGS=`pkgconf --cflags $(DEPS)` -Wno-write-strings -O3 -DDEBUG_MODE
LIBS=`pkgconf --static --libs $(DEPS)`
CC=g++

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)
main:	$(OBJS)
	$(CC) $(LIBS) $(OBJS) -o main $(CFLAGS)
all:	main
clean:
	rm -fv main *.o
list:
	echo list, all, main, ld, clean
