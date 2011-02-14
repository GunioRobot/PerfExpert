TARGET = bin/sniffer
OBJECT = src/driver.o

all:	clean $(OBJECT)
	-mkdir bin/
	$(CC) $(LDFLAGS) $(OBJECT) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECT)
	rm -f lcpi.properties experiment.header.tmp
	rm -rf bin/
