.PHONY: all clean

all: main.exe

clean:
	rm -f main.exe

main.exe: main.c
	x86_64-w64-mingw32-gcc -O3 -Wall -std=c89 -s -o main.exe main.c -lxinput -lwinmm
