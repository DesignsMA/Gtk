#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <uuid/uuid.h>
#include <ncurses.h>
#include <time.h>

// Estructuras
#define MAX_NAME 100
#define MAX_DESC 200
#define MAX_URL 150
#define MAX_CAT_NAME 50
#define MAX_SUBCAT_NAME 50
#define MAX_SUBCATS_POR_CAT 20
#define MAX_LOG_ENTRIES 100
#define MAX_LOG_LENGTH 200

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

// Variables globales compartidas
Categoria *categorias = NULL;
Producto *productos = NULL;
int num_categorias = 0;
int num_productos = 0;
pthread_mutex_t mutex_datos = PTHREAD_MUTEX_INITIALIZER;

// Sistema de logs para ncurses
char logs[MAX_LOG_ENTRIES][MAX_LOG_LENGTH];
int log_count = 0;
int log_start = 0;
pthread_mutex_t mutex_logs = PTHREAD_MUTEX_INITIALIZER;

// Función para agregar log
void agregar_log(const char* mensaje) {
    pthread_mutex_lock(&mutex_logs);
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, 20, "%H:%M:%S", tm_info);
    
    char log_entry[MAX_LOG_LENGTH];
    snprintf(log_entry, MAX_LOG_LENGTH, "[%s] %s", time_str, mensaje);
    
    if (log_count < MAX_LOG_ENTRIES) {
        strcpy(logs[log_count], log_entry);
        log_count++;
    } else {
        // Rotar logs
        for (int i = 0; i < MAX_LOG_ENTRIES - 1; i++) {
            strcpy(logs[i], logs[i + 1]);
        }
        strcpy(logs[MAX_LOG_ENTRIES - 1], log_entry);
        log_start = 0;
    }
    
    pthread_mutex_unlock(&mutex_logs);
}

// Función para generar UUID
void generar_id(char *buffer) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, buffer);
}

// Función para convertir a minúsculas
void a_minusculas(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 32;
        }
    }
}

// ==================== FUNCIONES DE DATOS ====================

void agregar_producto_seguro(Producto *nuevo) {
    pthread_mutex_lock(&mutex_datos);
    
    char log_msg[100];
    snprintf(log_msg, 100, "AGREGAR: %s (Categoría: %s)", nuevo->nombre, nuevo->categoria);
    agregar_log(log_msg);
    
    productos = realloc(productos, (num_productos + 1) * sizeof(Producto));
    productos[num_productos] = *nuevo;
    num_productos++;
    
    agregar_log("✅ Producto guardado en sistema");
    
    pthread_mutex_unlock(&mutex_datos);
}

void eliminar_producto_seguro(const char *id) {
    pthread_mutex_lock(&mutex_datos);
    
    char log_msg[100];
    snprintf(log_msg, 100, "ELIMINAR: Buscando ID %s", id);
    agregar_log(log_msg);
    
    int encontrado = 0;
    for (int i = 0; i < num_productos; i++) {
        if (strcmp(productos[i].id, id) == 0) {
            snprintf(log_msg, 100, "ELIMINAR: Encontrado %s", productos[i].nombre);
            agregar_log(log_msg);
            
            for (int j = i; j < num_productos - 1; j++) {
                productos[j] = productos[j + 1];
            }
            num_productos--;
            encontrado = 1;
            break;
        }
    }
    
    if (encontrado) {
        agregar_log("✅ Producto eliminado del sistema");
    } else {
        agregar_log("❌ Producto no encontrado");
    }
    
    pthread_mutex_unlock(&mutex_datos);
}

void modificar_stock_seguro(const char *id, int cantidad) {
    pthread_mutex_lock(&mutex_datos);
    
    char log_msg[100];
    snprintf(log_msg, 100, "STOCK: Modificando ID %s en %d unidades", id, cantidad);
    agregar_log(log_msg);
    
    int encontrado = 0;
    for (int i = 0; i < num_productos; i++) {
        if (strcmp(productos[i].id, id) == 0) {
            snprintf(log_msg, 100, "STOCK: %s - Antes: %d", productos[i].nombre, productos[i].stock);
            agregar_log(log_msg);
            
            productos[i].stock += cantidad;
            
            snprintf(log_msg, 100, "STOCK: %s - Después: %d", productos[i].nombre, productos[i].stock);
            agregar_log(log_msg);
            
            encontrado = 1;
            break;
        }
    }
    
    if (encontrado) {
        agregar_log("✅ Stock actualizado");
    } else {
        agregar_log("❌ Producto no encontrado");
    }
    
    pthread_mutex_unlock(&mutex_datos);
}

// ==================== FUNCIONES DE CONSOLA ADMIN CON NCURSES ====================

void mostrar_panel_logs(WINDOW *log_win) {
    pthread_mutex_lock(&mutex_logs);
    
    werase(log_win);
    wborder(log_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(log_win, 0, 2, " LOGS DEL SISTEMA ");
    
    int max_y, max_x;
    getmaxyx(log_win, max_y, max_x);
    
    int lineas_disponibles = max_y - 2;
    int inicio = (log_count > lineas_disponibles) ? log_count - lineas_disponibles : 0;
    
    for (int i = inicio; i < log_count && (i - inicio) < lineas_disponibles; i++) {
        mvwprintw(log_win, (i - inicio) + 1, 1, "%-*.*s", max_x - 2, max_x - 2, logs[i]);
    }
    
    wrefresh(log_win);
    pthread_mutex_unlock(&mutex_logs);
}

void mostrar_panel_estado(WINDOW *status_win) {
    pthread_mutex_lock(&mutex_datos);
    
    werase(status_win);
    wborder(status_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(status_win, 0, 2, " ESTADO DEL SISTEMA ");
    
    mvwprintw(status_win, 1, 2, "📊 Productos en sistema: %d", num_productos);
    mvwprintw(status_win, 2, 2, "📁 Categorías en sistema: %d", num_categorias);
    mvwprintw(status_win, 3, 2, "🌐 Servidor: ACTIVO (puerto 8080)");
    mvwprintw(status_win, 4, 2, "🔄 Heartbeat: cada 5 segundos");
    
    // Resumen por categoría
    if (num_productos > 0) {
        int leches = 0, quesos = 0, otros = 0;
        for (int i = 0; i < num_productos; i++) {
            if (strcmp(productos[i].categoria, "leches") == 0) leches++;
            else if (strcmp(productos[i].categoria, "quesos") == 0) quesos++;
            else otros++;
        }
        mvwprintw(status_win, 6, 2, "📦 Resumen por categoría:");
        mvwprintw(status_win, 7, 4, "🥛 Leches: %d productos", leches);
        mvwprintw(status_win, 8, 4, "🧀 Quesos: %d productos", quesos);
        mvwprintw(status_win, 9, 4, "📦 Otros: %d productos", otros);
    }
    
    wrefresh(status_win);
    pthread_mutex_unlock(&mutex_datos);
}

void agregar_producto_desde_consola(WINDOW *main_win) {
    Producto nuevo;
    char buffer[256];
    
    werase(main_win);
    wborder(main_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(main_win, 0, 2, " AGREGAR NUEVO PRODUCTO ");
    
    mvwprintw(main_win, 2, 2, "Nombre: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 255);
    noecho();
    strcpy(nuevo.nombre, buffer);
    a_minusculas(nuevo.nombre);
    
    mvwprintw(main_win, 3, 2, "Descripción: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 255);
    noecho();
    strcpy(nuevo.descripcion, buffer);
    a_minusculas(nuevo.descripcion);
    
    mvwprintw(main_win, 4, 2, "Precio: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 255);
    noecho();
    nuevo.precio = atof(buffer);
    
    mvwprintw(main_win, 5, 2, "Stock inicial: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 255);
    noecho();
    nuevo.stock = atoi(buffer);
    
    mvwprintw(main_win, 6, 2, "Categoría: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 255);
    noecho();
    strcpy(nuevo.categoria, buffer);
    a_minusculas(nuevo.categoria);
    
    mvwprintw(main_win, 7, 2, "Subcategoría: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 255);
    noecho();
    strcpy(nuevo.subcategoria, buffer);
    a_minusculas(nuevo.subcategoria);
    
    mvwprintw(main_win, 8, 2, "URL imagen: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 255);
    noecho();
    strcpy(nuevo.imagen_url, buffer);
    
    generar_id(nuevo.id);
    
    agregar_producto_seguro(&nuevo);
    
    mvwprintw(main_win, 10, 2, "✅ Producto agregado exitosamente! ID: %s", nuevo.id);
    mvwprintw(main_win, 12, 2, "Presiona cualquier tecla para continuar...");
    wrefresh(main_win);
    wgetch(main_win);
}

void eliminar_producto_desde_consola(WINDOW *main_win) {
    char id[37];
    
    werase(main_win);
    wborder(main_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(main_win, 0, 2, " ELIMINAR PRODUCTO ");
    
    mvwprintw(main_win, 2, 2, "ID del producto a eliminar: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, id, 36);
    noecho();
    
    eliminar_producto_seguro(id);
    
    mvwprintw(main_win, 4, 2, "✅ Operación completada");
    mvwprintw(main_win, 6, 2, "Presiona cualquier tecla para continuar...");
    wrefresh(main_win);
    wgetch(main_win);
}

void modificar_stock_desde_consola(WINDOW *main_win) {
    char id[37];
    char buffer[20];
    int cantidad;
    
    werase(main_win);
    wborder(main_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(main_win, 0, 2, " MODIFICAR STOCK ");
    
    mvwprintw(main_win, 2, 2, "ID del producto: ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, id, 36);
    noecho();
    
    mvwprintw(main_win, 3, 2, "Cantidad a agregar/restar (ej: 10 o -5): ");
    wrefresh(main_win);
    echo();
    wgetnstr(main_win, buffer, 19);
    noecho();
    cantidad = atoi(buffer);
    
    modificar_stock_seguro(id, cantidad);
    
    mvwprintw(main_win, 5, 2, "✅ Stock modificado");
    mvwprintw(main_win, 7, 2, "Presiona cualquier tecla para continuar...");
    wrefresh(main_win);
    wgetch(main_win);
}

void mostrar_productos_consola(WINDOW *main_win) {
    werase(main_win);
    wborder(main_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(main_win, 0, 2, " PRODUCTOS ACTUALES ");
    
    int max_y, max_x;
    getmaxyx(main_win, max_y, max_x);
    int linea_actual = 2;
    
    pthread_mutex_lock(&mutex_datos);
    
    for (int i = 0; i < num_productos && linea_actual < max_y - 2; i++) {
        mvwprintw(main_win, linea_actual++, 2, "ID: %s", productos[i].id);
        mvwprintw(main_win, linea_actual++, 2, "Nombre: %s", productos[i].nombre);
        mvwprintw(main_win, linea_actual++, 2, "Precio: $%.2f | Stock: %d", productos[i].precio, productos[i].stock);
        mvwprintw(main_win, linea_actual++, 2, "Categoría: %s | Subcategoría: %s", 
                 productos[i].categoria, productos[i].subcategoria);
        
        if (linea_actual < max_y - 2) {
            mvwprintw(main_win, linea_actual++, 2, "---");
        }
    }
    
    mvwprintw(main_win, max_y - 2, 2, "Total de productos: %d", num_productos);
    
    pthread_mutex_unlock(&mutex_datos);
    
    mvwprintw(main_win, max_y - 1, 2, "Presiona cualquier tecla para continuar...");
    wrefresh(main_win);
    wgetch(main_win);
}

void iniciar_consola_admin() {
    // Inicializar ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // Configurar colores si están disponibles
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Crear ventanas
    WINDOW *status_win = newwin(12, max_x, 0, 0);
    WINDOW *main_win = newwin(max_y - 12 - 10, max_x, 12, 0);
    WINDOW *log_win = newwin(10, max_x, max_y - 10, 0);
    
    // Panel de logs inicial
    agregar_log("=== SISTEMA INICIADO ===");
    agregar_log("Consola administrativa activa");
    agregar_log("Servidor listo en puerto 8080");
    
    while(1) {
        // Actualizar paneles
        mostrar_panel_estado(status_win);
        mostrar_panel_logs(log_win);
        
        // Menú principal
        werase(main_win);
        wborder(main_win, '|', '|', '-', '-', '+', '+', '+', '+');
        mvwprintw(main_win, 0, 2, " MENU PRINCIPAL ");
        
        mvwprintw(main_win, 2, 4, "1. Agregar producto");
        mvwprintw(main_win, 3, 4, "2. Eliminar producto");
        mvwprintw(main_win, 4, 4, "3. Modificar stock");
        mvwprintw(main_win, 5, 4, "4. Ver productos actuales");
        mvwprintw(main_win, 6, 4, "5. Actualizar vista");
        mvwprintw(main_win, 7, 4, "6. Salir");
        mvwprintw(main_win, 9, 4, "Selecciona opción: ");
        
        wrefresh(main_win);
        
        int opcion = wgetch(main_win) - '0';
        
        switch(opcion) {
            case 1:
                agregar_producto_desde_consola(main_win);
                break;
            case 2:
                eliminar_producto_desde_consola(main_win);
                break;
            case 3:
                modificar_stock_desde_consola(main_win);
                break;
            case 4:
                mostrar_productos_consola(main_win);
                break;
            case 5:
                // Solo actualizar la vista
                agregar_log("Vista actualizada");
                break;
            case 6:
                agregar_log("Saliendo del sistema...");
                wrefresh(main_win);
                sleep(1);
                endwin();
                return;
            default:
                mvwprintw(main_win, 11, 4, "❌ Opción inválida. Presiona cualquier tecla...");
                wrefresh(main_win);
                wgetch(main_win);
        }
    }
}

// ==================== FUNCIONES DE SERVIDOR ====================

void enviar_datos_completos(int client_sock) {
    pthread_mutex_lock(&mutex_datos);
    
    char log_msg[100];
    snprintf(log_msg, 100, "Enviando datos a cliente - %d productos", num_productos);
    agregar_log(log_msg);
    
    // Simular envío de datos
    send(client_sock, &num_productos, sizeof(int), 0);
    
    pthread_mutex_unlock(&mutex_datos);
    
    agregar_log("✅ Datos enviados al cliente");
}

void* manejar_cliente_tienda(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_sock, (struct sockaddr*)&client_addr, &addr_len);
    
    char log_msg[100];
    snprintf(log_msg, 100, "Cliente conectado desde %s:%d", 
             inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    agregar_log(log_msg);
    
    enviar_datos_completos(client_sock);
    
    agregar_log("Cliente listo para realizar compras");
    
    close(client_sock);
    agregar_log("Cliente desconectado");
    
    return NULL;
}

// ==================== FUNCIÓN DE HEARTBEAT ====================

void* mostrar_heartbeat(void* arg) {
    while(1) {
        sleep(5);
        
        pthread_mutex_lock(&mutex_datos);
        
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_str[20];
        strftime(time_str, 20, "%H:%M:%S", tm_info);
        
        char heartbeat_msg[100];
        snprintf(heartbeat_msg, 100, "HEARTBEAT - Productos: %d, Categorías: %d", 
                 num_productos, num_categorias);
        agregar_log(heartbeat_msg);
        
        pthread_mutex_unlock(&mutex_datos);
    }
    return NULL;
}

// ==================== SERVIDOR ====================

void* iniciar_servidor(void* arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    agregar_log("Iniciando servidor en puerto 8080...");
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        agregar_log("❌ Error en socket");
        pthread_exit(NULL);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        agregar_log("❌ Error en setsockopt");
        pthread_exit(NULL);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        agregar_log("❌ Error en bind");
        pthread_exit(NULL);
    }
    
    if (listen(server_fd, 3) < 0) {
        agregar_log("❌ Error en listen");
        pthread_exit(NULL);
    }
    
    agregar_log("✅ Servidor listo en puerto 8080");
    agregar_log("Esperando conexiones de clientes...");
    
    int client_count = 0;
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                               (socklen_t*)&addrlen)) < 0) {
            agregar_log("❌ Error en accept");
            continue;
        }
        
        client_count++;
        char log_msg[100];
        snprintf(log_msg, 100, "🎯 Cliente %d conectado desde %s:%d", 
                 client_count, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        agregar_log(log_msg);
        
        pthread_t client_thread;
        int *client_sock = malloc(sizeof(int));
        *client_sock = new_socket;
        pthread_create(&client_thread, NULL, manejar_cliente_tienda, client_sock);
        pthread_detach(client_thread);
    }
    
    return NULL;
}

// ==================== DATOS INICIALES ====================

void cargar_datos_iniciales() {
    agregar_log("Cargando datos iniciales...");
    
    // Productos iniciales
    Producto p1, p2;
    
    generar_id(p1.id);
    strcpy(p1.nombre, "leche entera");
    strcpy(p1.descripcion, "leche entera fresca");
    p1.precio = 25.50;
    p1.stock = 100;
    strcpy(p1.categoria, "leches");
    strcpy(p1.subcategoria, "entera");
    strcpy(p1.imagen_url, "imgs/leche_entera.jpg");
    
    generar_id(p2.id);
    strcpy(p2.nombre, "queso fresco");
    strcpy(p2.descripcion, "queso fresco natural");
    p2.precio = 45.00;
    p2.stock = 50;
    strcpy(p2.categoria, "quesos");
    strcpy(p2.subcategoria, "fresco");
    strcpy(p2.imagen_url, "imgs/queso_fresco.jpg");
    
    productos = malloc(2 * sizeof(Producto));
    productos[0] = p1;
    productos[1] = p2;
    num_productos = 2;
    
    agregar_log("✅ Datos iniciales cargados (2 productos)");
}

// ==================== MAIN ====================

int main(int argc, char *argv[]) {
    printf("Iniciando sistema...\n");
    
    cargar_datos_iniciales();
    
    // Iniciar servidor en segundo plano
    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, iniciar_servidor, NULL);
    
    // Iniciar heartbeat en segundo plano
    pthread_t hilo_heartbeat;
    pthread_create(&hilo_heartbeat, NULL, mostrar_heartbeat, NULL);
    
    // Pequeña pausa para que el servidor se inicialice
    sleep(2);
    
    // Iniciar interfaz ncurses (esto bloqueará hasta que salgas)
    iniciar_consola_admin();
    
    // Limpiar al salir
    agregar_log("Cerrando servidor...");
    pthread_cancel(hilo_servidor);
    pthread_cancel(hilo_heartbeat);
    
    free(productos);
    free(categorias);
    pthread_mutex_destroy(&mutex_datos);
    pthread_mutex_destroy(&mutex_logs);
    
    printf("Sistema terminado\n");
    return 0;
}   