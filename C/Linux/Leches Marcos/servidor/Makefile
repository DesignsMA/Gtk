CC = gcc
CFLAGS = -pthread `pkg-config --cflags glib-2.0`
LIBS = -luuid `pkg-config --libs glib-2.0`

# Compilar todo
all: admin servidor

admin: admin.c
	$(CC) $< -o $@ $(CFLAGS) $(LIBS) 

servidor: servidor.c
	$(CC) $< -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -f admin servidor

.PHONY: all clean