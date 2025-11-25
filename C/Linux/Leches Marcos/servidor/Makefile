CC = gcc
CFLAGS = -pthread -lncurses
LIBS = -luuid
TARGET = tienda_server

$(TARGET): admin.c
	$(CC) admin.c -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
