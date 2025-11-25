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
    
    // Struct para pasar datos a la función asíncrona de carga de imágenes
    typedef struct {
        char *image_url;
        GtkPicture *picture;
        char *cache_key; // clave para caché
        SoupMessage *msg; // mensaje de la solicitud
    } SolicitudImagen;

    // Sesión global para todas las imágenes
    extern SoupSession *image_session;

    int conectar_servidor(int puerto);
    void cargar_imagen(const char* url, GtkPicture *picture);
    void aplicar_imagen(GObject *source_object, GAsyncResult *res, gpointer user_data);
    void limpiar_sesion_imagenes();

#endif // LECHES_SOCKET_H