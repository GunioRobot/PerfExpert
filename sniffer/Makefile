CFLAGS=-fPIC -lpapi

TARGET = bin/sniffer
OBJECT = src/driver.o

all:	clean $(OBJECT)
	-mkdir bin/
	$(CC) $(CFLAGS) $(OBJECT) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECT)
	rm -rf bin/
