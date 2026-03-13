COM = gcc
COMFLAGS = -Wall
TARGET = app

all: $(TARGET)

$(TARGET):
	$(COM) $(COMFLAGS) -o $(TARGET) app.c server.c

clean:
	rm -f *.o $(TARGET)
