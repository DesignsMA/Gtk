# Variables
RESOURCES_XML = leches.gresource.xml
RESOURCES_C = leches-resources.c
RESOURCES_H = leches-resources.h
TARGET = leches
SOURCES = leches.c leches_socket.c leches_dynamicLists.c producto.c $(RESOURCES_C)

# Compilador y flags
CC = gcc
CFLAGS = -std=c11 `pkg-config --cflags gtk4`
LIBS = `pkg-config --libs gtk4 fontconfig glib-2.0 gmodule-export-2.0` -Wl,--export-dynamic

# Regla principal - limpia y recompila TODO
all: clean-all $(TARGET)

# Limpieza completa antes de recompilar
clean-all:
	@echo "Limpiando archivos anteriores..."
	rm -f $(TARGET) $(RESOURCES_C) $(RESOURCES_H) *.o

# Generar recursos
$(RESOURCES_C): $(RESOURCES_XML)
	@echo "Generando recursos C..."
	glib-compile-resources --generate-source --target=$(RESOURCES_C) $(RESOURCES_XML)

$(RESOURCES_H): $(RESOURCES_XML)
	@echo "Generando recursos H..."
	glib-compile-resources --generate-header --target=$(RESOURCES_H) $(RESOURCES_XML)

# Compilar ejecutable
$(TARGET): $(SOURCES) $(RESOURCES_H)
	@echo "Compilando ejecutable..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	@echo "Compilación completada: $(TARGET)"

# Limpiar solo archivos generados
clean:
	@echo "Limpiando archivos compilados..."
	rm -f $(TARGET) $(RESOURCES_C) $(RESOURCES_H) *.o

# Forzar recompilación de recursos
resources: clean-resources $(RESOURCES_C) $(RESOURCES_H)

clean-resources:
	@echo "Limpiando recursos..."
	rm -f $(RESOURCES_C) $(RESOURCES_H)

# Recompilación forzada
rebuild: clean-all all

# Ejecutar la aplicación
run: $(TARGET)
	./$(TARGET)

# Depuración
debug: CFLAGS += -g -DDEBUG
debug: rebuild

.PHONY: all clean clean-all resources clean-resources rebuild run debug