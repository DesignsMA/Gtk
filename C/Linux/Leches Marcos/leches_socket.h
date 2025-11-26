#ifndef LECHES_SOCKET_H
    #define LECHES_SOCKET_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <libsoup/soup.h>
    #include <gtk/gtk.h>
    #define SIZE 2048
    #define MAX_NAME 100
    #define MAX_DESC 200
    #define MAX_URL 300
    #define MAX_CAT_NAME 50
    #define MAX_SUBCAT_NAME 50
    #define MAX_SUBCATS_POR_CAT 20
    #define MAX_CATEGORIAS 10
    #define MAX_PRODUCTOS 200
    
    // Struct para pasar datos a la función asíncrona de carga de imágenes
    typedef struct {
        char *image_url;
        GtkPicture *picture;
        char *cache_key;
        SoupMessage *msg;
        int max_width;
        int max_height;
    } SolicitudImagen;

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

    extern int socket_servidor;
    extern struct sockaddr_in servidor_addr;
    // Sesión global para todas las imágenes
    extern SoupSession *image_session;

    extern Categoria *categorias;
    extern Producto *productos;

    extern size_t num_categorias;
    extern size_t num_productos;

    int enviar_comando_servidor(const char *comando);
    int recibir_int_servidor();
    int cargar_categorias_desde_servidor();
    int cargar_productos_desde_servidor();
    int verificar_stock_servidor(const char *product_id, int cantidad);
    int comprar_producto_servidor(const char *product_id, int cantidad);
    void cargar_datos_desde_servidor();
    void liberar_datos_servidor();

    int conectar_servidor(const char *server_ip, const char *server_port);
    void cargar_imagen(const char* url, GtkPicture *picture, int max_width, int max_height);
    void aplicar_imagen(GObject *source_object, GAsyncResult *res, gpointer user_data);
    void limpiar_sesion_imagenes();

#endif // LECHES_SOCKET_H