COM = gcc
COMFLAGS = -Wall #-g debug flags
TARGET = app.exe

all: clean $(TARGET) run

$(TARGET):
	$(COM) $(COMFLAGS) -o $(TARGET) app.c server.c 

run:
	./$(TARGET)
clean:
	clear && rm -f  $(TARGET)
