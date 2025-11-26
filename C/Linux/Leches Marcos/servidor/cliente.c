#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ==================== ESTRUCTURAS PARA RED ====================
#define MAX_NAME 100
#define MAX_DESC 200
#define MAX_URL 300
#define MAX_CAT_NAME 50
#define MAX_SUBCAT_NAME 50
#define MAX_SUBCATS_POR_CAT 20

typedef struct {
    char nombre[MAX_CAT_NAME];
    char subcategorias[MAX_SUBCATS_POR_CAT][MAX_SUBCAT_NAME];
    int num_subcategorias;
} Categoria;

typedef struct {
    char id[37];
    char nombre[MAX_NAME];
    char descripcion[MAX_DESC];
    float precio;
    int stock;
    char categoria[MAX_CAT_NAME];
    char subcategoria[MAX_SUBCAT_NAME];
    char imagen_url[MAX_URL];
} Producto;

// ==================== FUNCIONES PARA RECIBIR DATOS ====================

void recibir_y_mostrar_categorias(int sock) {
    int num_categorias;
    
    // Recibir número de categorías
    if (recv(sock, &num_categorias, sizeof(int), 0) <= 0) {
        printf("Error al recibir número de categorías\n");
        return;
    }
    
    printf("Recibiendo %d categorías...\n", num_categorias);
    
    if (num_categorias > 0) {
        Categoria *categorias = malloc(num_categorias * sizeof(Categoria));
        
        // Recibir categorías
        if (recv(sock, categorias, num_categorias * sizeof(Categoria), 0) <= 0) {
            printf("Error al recibir categorías\n");
            free(categorias);
            return;
        }
        
        // Mostrar categorías
        printf("\n=== CATEGORÍAS RECIBIDAS ===\n");
        for (int i = 0; i < num_categorias; i++) {
            printf("Categoría: %s\n", categorias[i].nombre);
            printf("Subcategorías (%d): ", categorias[i].num_subcategorias);
            for (int j = 0; j < categorias[i].num_subcategorias; j++) {
                printf("%s", categorias[i].subcategorias[j]);
                if (j < categorias[i].num_subcategorias - 1) printf(", ");
            }
            printf("\n---\n");
        }
        
        free(categorias);
    }
}

void recibir_y_mostrar_productos(int sock) {
    int num_productos;
    
    // Recibir número de productos
    if (recv(sock, &num_productos, sizeof(int), 0) <= 0) {
        printf("Error al recibir número de productos\n");
        return;
    }
    
    printf("Recibiendo %d productos...\n", num_productos);
    
    if (num_productos > 0) {
        Producto *productos = malloc(num_productos * sizeof(Producto));
        
        // Recibir productos
        if (recv(sock, productos, num_productos * sizeof(Producto), 0) <= 0) {
            printf("Error al recibir productos\n");
            free(productos);
            return;
        }
        
        // Mostrar productos
        printf("\n=== PRODUCTOS RECIBIDOS ===\n");
        for (int i = 0; i < num_productos; i++) {
            printf("ID: %s\n", productos[i].id);
            printf("Nombre: %s\n", productos[i].nombre);
            printf("Descripción: %s\n", productos[i].descripcion);
            printf("Precio: $%.2f | Stock: %d\n", productos[i].precio, productos[i].stock);
            printf("Categoría: %s | Subcategoría: %s\n", productos[i].categoria, productos[i].subcategoria);
            printf("Imagen: %s\n", productos[i].imagen_url);
            printf("---\n");
        }
        
        free(productos);
    }
}

// ==================== FUNCIÓN PARA COMPRAR PRODUCTO ====================

void comprar_producto_cliente(int sock) {
    char id_producto[37];
    int cantidad;
    
    printf("ID del producto a comprar: ");
    fgets(id_producto, sizeof(id_producto), stdin);
    id_producto[strcspn(id_producto, "\n\r")] = '\0';
    
    printf("Cantidad: ");
    scanf("%d", &cantidad);
    getchar(); // Limpiar buffer
    
    // Enviar comando de compra
    char comando[100];
    snprintf(comando, sizeof(comando), "COMPRAR_PRODUCTO:%s:%d", id_producto, cantidad);
    send(sock, comando, strlen(comando), 0);
    
    // Recibir resultado
    int resultado;
    if (recv(sock, &resultado, sizeof(int), 0) > 0) {
        switch (resultado) {
            case 1:
                printf("✅ Compra realizada exitosamente\n");
                break;
            case 0:
                printf("❌ Stock insuficiente\n");
                break;
            case -1:
                printf("❌ Producto no encontrado\n");
                break;
            case -2:
                printf("❌ Error en formato de comando\n");
                break;
            default:
                printf("❌ Error desconocido\n");
        }
    }
}

// ==================== MAIN DEL CLIENTE ====================

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char comando[256];
    
    // ==================== CONEXIÓN AL SERVIDOR ====================
    
    // Crear socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error al crear socket");
        exit(1);
    }
    
    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Puerto del servidor
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost
    
    // Conectar al servidor
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(sock);
        exit(1);
    }
    
    printf("✅ Conectado al servidor\n");
    
    // ==================== BUCLE PRINCIPAL DEL CLIENTE ====================
    
    while (1) {
        printf("\nComandos disponibles:\n");
        printf("GET_PRODUCTOS - Obtener lista de productos\n");
        printf("GET_CATEGORIAS - Obtener lista de categorías\n");
        printf("COMPRAR - Comprar producto\n");
        printf("EXIT - Salir\n");
        printf("Ingrese comando: ");
        
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }
        
        // Limpiar newline
        comando[strcspn(comando, "\n")] = '\0';
        comando[strcspn(comando, "\r")] = '\0';
        
        if (strlen(comando) == 0) continue;
        
        // Procesar comando
        if (strcmp(comando, "EXIT") == 0) {
            send(sock, "EXIT", 4, 0);
            break;
        } else if (strcmp(comando, "GET_PRODUCTOS") == 0) {
            send(sock, "GET_PRODUCTOS", 13, 0);
            recibir_y_mostrar_productos(sock);
        } else if (strcmp(comando, "GET_CATEGORIAS") == 0) {
            send(sock, "GET_CATEGORIAS", 14, 0);
            recibir_y_mostrar_categorias(sock);
        } else if (strcmp(comando, "COMPRAR") == 0) {
            comprar_producto_cliente(sock);
        } else {
            printf("❌ Comando desconocido: %s\n", comando);
        }
    }
    
    // ==================== LIMPIEZA ====================
    close(sock);
    printf("👋 Desconectado del servidor\n");
    return 0;
}