CFLAGS=-m32 -fPIC
DEBUG_FLAGS=-DDEBUG_PRINT -g

TARGET = bin/hound
OBJECT = src/driver.o
SOURCE = src/driver.c

all:	clean $(OBJECT)
	-mkdir bin/
	$(CC) $(CFLAGS) $(OBJECT) -o $(TARGET)

debug:	clean $(SOURCE)
	-mkdir bin/
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $(SOURCE) -o $(OBJECT)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(OBJECT) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECT)
	rm -rf bin/
