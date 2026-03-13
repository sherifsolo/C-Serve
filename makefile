COM = gcc
COMFLAGS =
#change target to your programs
TARGET = app.exe

#change main to your programs name
all: $(TARGET)

$(TARGET):
	$(COM) -o $(TARGET) app.c server.c

clean:
	rm -f *.o $(TARGET)
