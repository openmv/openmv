BINARY=openmv
CFLAGS= -Wall -O0 -ggdb `pkg-config --cflags sdl libusb-1.0` -I../src/
LIBS= -lusb-1.0 -lSDL -lSDL_ttf
SRCS= main.c

all:main.c
	gcc $(CFLAGS) $(SRCS) -o $(BINARY) $(LIBS) 
clean:
	rm -f $(BINARY)
