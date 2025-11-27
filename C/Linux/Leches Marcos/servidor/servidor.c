#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <glib.h>
#include <uuid/uuid.h>
#include <ctype.h>

// ==================== ESTRUCTURAS COMPARTIDAS ====================
#define MAX_NAME 100
#define MAX_DESC 200
#define MAX_URL 300
#define MAX_CAT_NAME 50
#define MAX_SUBCAT_NAME 50
#define MAX_SUBCATS_POR_CAT 20

// Estructuras para GLib (internas del servidor)
typedef struct {
    char nombre[MAX_CAT_NAME];
    GArray *subcategorias;
} CategoriaGLib;

typedef struct {
    char id[37];
    char nombre[MAX_NAME];
    char descripcion[MAX_DESC];
    float precio;
    int stock;
    char categoria[MAX_CAT_NAME];
    char subcategoria[MAX_SUBCAT_NAME];
    char imagen_url[MAX_URL];
} ProductoGLib;

// Estructuras para red (planas)
typedef struct {
    char nombre[MAX_CAT_NAME];
    char subcategorias[MAX_SUBCATS_POR_CAT][MAX_SUBCAT_NAME];
    int num_subcategorias;
} CategoriaPlana;

typedef struct {
    char id[37];
    char nombre[MAX_NAME];
    char descripcion[MAX_DESC];
    float precio;
    int stock;
    char categoria[MAX_CAT_NAME];
    char subcategoria[MAX_SUBCAT_NAME];
    char imagen_url[MAX_URL];
} ProductoPlano;

// ==================== VARIABLES GLOBALES COMPARTIDAS ====================
GArray *categorias = NULL;
GArray *productos = NULL;

// ==================== FUNCIONES DE PERSISTENCIA (COPIADAS DEL ADMIN) ====================

void guardar_datos() {
    // Guardar productos
    FILE *f_prod = fopen("productos.bin", "wb");
    if (f_prod) {
        int num_prod = productos->len;
        fwrite(&num_prod, sizeof(int), 1, f_prod);
        fwrite(productos->data, sizeof(ProductoGLib), num_prod, f_prod);
        fclose(f_prod);
    }
    
    // Guardar categorías
    FILE *f_cat = fopen("categorias.bin", "wb");
    if (f_cat) {
        int num_cat = categorias->len;
        fwrite(&num_cat, sizeof(int), 1, f_cat);
        
        for (int i = 0; i < num_cat; i++) {
            CategoriaGLib *cat = &g_array_index(categorias, CategoriaGLib, i);
            
            // Guardar categoría base (sin el GArray*)
            fwrite(cat, sizeof(CategoriaGLib) - sizeof(GArray*), 1, f_cat);
            
            // Guardar número de subcategorías
            int num_sub = cat->subcategorias->len;
            fwrite(&num_sub, sizeof(int), 1, f_cat);
            
            // Guardar cada subcategoría
            for (int j = 0; j < num_sub; j++) {
                char *subcat = g_array_index(cat->subcategorias, char*, j);
                fwrite(subcat, MAX_SUBCAT_NAME, 1, f_cat);
            }
        }
        fclose(f_cat);
    }
    
    printf("Datos guardados\n");
}

void cargar_datos() {
    // Inicializar arrays GLib si no existen
    if (!productos) {
        productos = g_array_new(FALSE, FALSE, sizeof(ProductoGLib));
    }
    if (!categorias) {
        categorias = g_array_new(FALSE, FALSE, sizeof(CategoriaGLib));
    }
    
    // Cargar productos
    FILE *f_prod = fopen("productos.bin", "rb");
    if (f_prod) {
        int num_prod;
        fread(&num_prod, sizeof(int), 1, f_prod);
        g_array_set_size(productos, num_prod);
        fread(productos->data, sizeof(ProductoGLib), num_prod, f_prod);
        fclose(f_prod);
        printf("Cargados %d productos\n", num_prod);
    }
    
    // Cargar categorías
    FILE *f_cat = fopen("categorias.bin", "rb");
    if (f_cat) {
        int num_cat;
        fread(&num_cat, sizeof(int), 1, f_cat);
        
        for (int i = 0; i < num_cat; i++) {
            CategoriaGLib cat;
            cat.subcategorias = g_array_new(FALSE, FALSE, sizeof(char*));
            
            // Cargar categoría base (sin el GArray*)
            fread(&cat, sizeof(CategoriaGLib) - sizeof(GArray*), 1, f_cat);
            
            // Cargar subcategorías
            int num_sub;
            fread(&num_sub, sizeof(int), 1, f_cat);
            
            for (int j = 0; j < num_sub; j++) {
                char subcat[MAX_SUBCAT_NAME];
                fread(subcat, MAX_SUBCAT_NAME, 1, f_cat);
                
                char *subcat_ptr = g_strdup(subcat);
                g_array_append_val(cat.subcategorias, subcat_ptr);
            }
            
            g_array_append_val(categorias, cat);
        }
        fclose(f_cat);
        printf("Cargadas %d categorías\n", num_cat);
    }
}

// ==================== FUNCIONES AUXILIARES ====================

void a_minusculas(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// ==================== CONVERSIÓN DE ESTRUCTURAS ====================

int convertir_categorias_a_planas(CategoriaPlana **categorias_planas) {
    int num_categorias = categorias->len;
    *categorias_planas = malloc(num_categorias * sizeof(CategoriaPlana));
    
    for (int i = 0; i < num_categorias; i++) {
        CategoriaGLib *cat_glib = &g_array_index(categorias, CategoriaGLib, i);
        CategoriaPlana *cat_plana = &(*categorias_planas)[i];
        
        strcpy(cat_plana->nombre, cat_glib->nombre);
        cat_plana->num_subcategorias = cat_glib->subcategorias->len;
        
        for (int j = 0; j < cat_glib->subcategorias->len && j < MAX_SUBCATS_POR_CAT; j++) {
            char *subcat = g_array_index(cat_glib->subcategorias, char*, j);
            strcpy(cat_plana->subcategorias[j], subcat);
        }
    }
    
    return num_categorias;
}

int convertir_productos_a_planos(ProductoPlano **productos_planos) {
    int num_productos = productos->len;
    *productos_planos = malloc(num_productos * sizeof(ProductoPlano));
    
    for (int i = 0; i < num_productos; i++) {
        ProductoGLib *prod_glib = &g_array_index(productos, ProductoGLib, i);
        ProductoPlano *prod_plano = &(*productos_planos)[i];
        
        strcpy(prod_plano->id, prod_glib->id);
        strcpy(prod_plano->nombre, prod_glib->nombre);
        strcpy(prod_plano->descripcion, prod_glib->descripcion);
        prod_plano->precio = prod_glib->precio;
        prod_plano->stock = prod_glib->stock;
        strcpy(prod_plano->categoria, prod_glib->categoria);
        strcpy(prod_plano->subcategoria, prod_glib->subcategoria);
        strcpy(prod_plano->imagen_url, prod_glib->imagen_url);
    }
    
    return num_productos;
}

// ==================== FUNCIÓN PARA COMPRAR PRODUCTO ====================

int comprar_producto(const char *id_producto, int cantidad) {
    // Buscar producto por ID
    for (int i = 0; i < productos->len; i++) {
        ProductoGLib *prod = &g_array_index(productos, ProductoGLib, i);
        if (strcmp(prod->id, id_producto) == 0) {
            if (prod->stock >= cantidad) {
                prod->stock -= cantidad;
                printf("Compra realizada: %d unidades de %s. Stock restante: %d\n", 
                       cantidad, prod->nombre, prod->stock);
                guardar_datos(); // Guardar cambios en disco
                return 1; // Éxito
            } else {
                printf("Stock insuficiente: %s (solicitado: %d, disponible: %d)\n", 
                       prod->nombre, cantidad, prod->stock);
                return 0; // Stock insuficiente
            }
        }
    }
    printf("Producto no encontrado: %s\n", id_producto);
    return -1; // No encontrado
}

int obtener_stock_producto(const char *id_producto) {
    // Buscar producto por ID
    for (int i = 0; i < productos->len; i++) {
        ProductoGLib *prod = &g_array_index(productos, ProductoGLib, i);
        if (strcmp(prod->id, id_producto) == 0) {
            return prod->stock;
        }
    }
    return -1; // No encontrado
}

// ==================== MANEJO DE COMANDOS ====================

void enviar_categorias(int client_socket) {
    CategoriaPlana *categorias_planas;
    int num_categorias = convertir_categorias_a_planas(&categorias_planas);
    
    send(client_socket, &num_categorias, sizeof(int), 0);
    if (num_categorias > 0) {
        send(client_socket, categorias_planas, num_categorias * sizeof(CategoriaPlana), 0);
    }
    
    free(categorias_planas);
    printf("Enviadas %d categorías\n", num_categorias);
}

void enviar_productos(int client_socket) {
    ProductoPlano *productos_planos;
    int num_productos = convertir_productos_a_planos(&productos_planos);
    
    send(client_socket, &num_productos, sizeof(int), 0);
    if (num_productos > 0) {
        send(client_socket, productos_planos, num_productos * sizeof(ProductoPlano), 0);
    }
    
    free(productos_planos);
    printf("Enviados %d productos\n", num_productos);
}

void manejar_compra(int client_socket, const char *comando) {
    // Formato: COMPRAR_PRODUCTO:id_producto:cantidad
    char id_producto[37];
    int cantidad;
    
    if (sscanf(comando, "COMPRAR_PRODUCTO:%36[^:]:%d", id_producto, &cantidad) == 2) {
        int resultado = comprar_producto(id_producto, cantidad);
        send(client_socket, &resultado, sizeof(int), 0);
    } else {
        int resultado = -2; // Formato inválido
        send(client_socket, &resultado, sizeof(int), 0);
    }
}

void manejar_verificar_stock(int client_socket, const char *comando) {
    // Formato: VERIFICAR_STOCK:product_id:cantidad
    char product_id[37];
    int cantidad;
    
    if (sscanf(comando, "VERIFICAR_STOCK:%36[^:]:%d", product_id, &cantidad) == 2) {
        int stock_disponible = obtener_stock_producto(product_id);
        send(client_socket, &stock_disponible, sizeof(int), 0);
    }
}

void manejar_comando(int client_socket, const char *comando) {
    printf("Comando recibido: %s\n", comando);
    
    if (strcmp(comando, "GET_PRODUCTOS") == 0) {
        enviar_productos(client_socket);
    } else if (strcmp(comando, "GET_CATEGORIAS") == 0) {
        enviar_categorias(client_socket);
    } else if (strncmp(comando, "COMPRAR_PRODUCTO", 16) == 0) {
        manejar_compra(client_socket, comando);
    } else if (strncmp(comando, "VERIFICAR_STOCK", 15) == 0) {
        manejar_verificar_stock(client_socket, comando);
    } else if (strcmp(comando, "EXIT") == 0) {
        printf("Cliente solicitó desconexión\n");
    } else {
        printf("Comando desconocido: %s\n", comando);
        char respuesta[100] = "ERROR: Comando desconocido";
        send(client_socket, respuesta, strlen(respuesta), 0);
    }
}
// ==================== MANEJO DE CLIENTE CON FORK ====================

void atender_cliente(int client_socket) {
    char buffer[256]; // Buffer para recibir comandos
    ssize_t bytes_leidos; // Bytes leídos del socket
    
    printf("Cliente conectado (PID: %d)\n", getpid());
    
    // ciclar y recibir comandos hasta que el cliente se desconecte (comnando EXIT)
    while ((bytes_leidos = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_leidos] = '\0'; // terminar en null la cadena recibida
        buffer[strcspn(buffer, "\n\r")] = '\0'; // eliminar saltos de línea y retorno de carro poniendo null en el primer caracter de esos tipos
        
        if (strlen(buffer) == 0) continue; // ignorar comandos vacíos
        
        if (strcmp(buffer, "EXIT") == 0) {
            break;
        }
        
        manejar_comando(client_socket, buffer);
    }
    
    close(client_socket);
    printf("Cliente desconectado (PID: %d)\n", getpid());
    exit(0);
}

// ==================== MAIN DEL SERVIDOR ====================

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    socklen_t addr_len = sizeof(server_addr);
    
    // Inicializar GLib y cargar datos
    categorias = g_array_new(FALSE, FALSE, sizeof(CategoriaGLib));
    productos = g_array_new(FALSE, FALSE, sizeof(ProductoGLib));
    cargar_datos();
    
    // Crear socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error al crear socket");
        exit(1);
    }
    
    // Configurar dirección
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(57277);
    
    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(server_socket);
        exit(1);
    }

    // Obtener el puerto asignado por el sistema
    if (getsockname(server_socket, (struct sockaddr*)&server_addr, &addr_len) < 0) {
        perror("Error obteniendo información del socket");
        close(server_socket);
        exit(1);
    }
    
    // Listen
    if (listen(server_socket, 10) < 0) {
        perror("Error en listen");
        close(server_socket);
        exit(1);
    }
    
    printf("Servidor iniciado en puerto %d\n", ntohs(server_addr.sin_port));
    printf("Esperando conexiones...\n");
    
    // Evitar procesos zombie
    // Cuando un proceso hijo termina, el padre ignora la señal SIGCHLD generada por el sistema operativo
    // cuando un hijo termina, el sistema operativo envía una señal SIGCHLD al proceso padre
    // al ignorar esta señal, el sistema operativo automáticamente limpia los recursos del proceso hijo terminado
    signal(SIGCHLD, SIG_IGN);
    
    // Bucle principal
    while (1) {
        // esperar conexión el el socket server_socket, si se recibe una conexión, crear un nuevo socket para el cliente
        // con su direccion y puerto
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Error en accept");
            continue;
        }
        
        // Crear proceso hijo para manejar al cliente especifico
        pid_t pid = fork();
        if (pid == 0) {
            // Proceso hijo
            close(server_socket);
            atender_cliente(client_socket);
        } else if (pid > 0) {
            // Proceso padre
            close(client_socket);
        } else {
            perror("Error en fork");
            close(client_socket);
        }
    }
    
    close(server_socket);
    return 0;
}