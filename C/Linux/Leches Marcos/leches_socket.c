#include "leches_socket.h"

SoupSession *image_session = NULL;

int conectar_servidor(int puerto) {
    const char *IP = "127.0.0.1";
    int sock;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto);

    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor en %s:%d\n", IP, puerto);
}

void aplicar_imagen(GObject *source_object, GAsyncResult *res, gpointer user_data) 
{
    SolicitudImagen *data = (SolicitudImagen *)user_data; // Recuperar datos pasados
    GError *error = NULL; // Para manejar errores

    // Obtener datos descargados desde la respuesta de la solicitud http (funcion asincrona ejecutada internamente por libsoup)
    GBytes *bytes = soup_session_send_and_read_finish(SOUP_SESSION(source_object), res, &error); 

    // Si no hubo error, guardar la imagen en caché y actualizar el GtkPicture
    if (bytes && !error) {
        // Creamos ruta hacia donde queremos guardar la imagen en caché
        char *cache_dir = g_build_filename(g_get_user_cache_dir(), "leches-cache", NULL);

        g_mkdir_with_parents(cache_dir, 0755); // Crear directorio si no existe , tambien los padres si es necesario con permisos rwxr-xr-x

        SoupMessageHeaders *response_headers = soup_message_get_response_headers(data->msg);
        
        const char *content_type = soup_message_headers_get_content_type(response_headers, NULL);
        
        const char *extension = "img";
        if (content_type) {
            if (g_str_has_prefix(content_type, "image/png")) extension = "png";
            else if (g_str_has_prefix(content_type, "image/jpeg")) extension = "jpg";
            else if (g_str_has_prefix(content_type, "image/gif")) extension = "gif";
            else if (g_str_has_prefix(content_type, "image/webp")) extension = "webp";
        }
        
        // Guardar con extensión detectada
        // Directorio espeficífico para esta imagen usando la clave única (hash MD5)
        // Nombre de archivo: "hash.extension"
        char *filename = g_strdup_printf("%s.%s", data->cache_key, extension);
        char *cache_path = g_build_filename(cache_dir, filename, NULL);
        
        // Guardar datos de la imagen en el archivo de caché
        g_file_set_contents(cache_path, g_bytes_get_data(bytes, NULL), g_bytes_get_size(bytes), NULL);
        
        // Reemplazar placeholder con imagen real
        GdkTexture *texture = gdk_texture_new_from_bytes(bytes, NULL);
        if (texture) {
            gtk_picture_set_paintable(GTK_PICTURE(data->picture), GDK_PAINTABLE(texture));
            g_object_unref(texture);
        }

        g_free(filename);
        g_free(cache_path);
        g_free(cache_dir);
    }
    
    // LIMPIAR MEMORIA
    if (bytes) g_bytes_unref(bytes);
    if (error) g_error_free(error);
    
    g_free(data->image_url);
    g_free(data->cache_key);
    g_object_unref(data->msg);
    g_free(data);
}

void cargar_imagen(const char* url, GtkPicture *picture) {
    SolicitudImagen *data = g_new0(SolicitudImagen, 1);
    data->image_url = g_strdup(url);
    data->picture = picture;
    data->cache_key = g_compute_checksum_for_string(G_CHECKSUM_MD5, url, -1);
    
    // Verificar cache con extensiones
    const char *extensions[] = {"png", "jpg", "jpeg", "gif", "webp", "img", NULL};
    char *found_cache_path = NULL;
    
    for (int i = 0; extensions[i]; i++) {
        char *filename = g_strdup_printf("%s.%s", data->cache_key, extensions[i]);
        char *test_path = g_build_filename(g_get_user_cache_dir(), "leches-cache", filename, NULL);
        
        if (g_file_test(test_path, G_FILE_TEST_EXISTS)) {
            found_cache_path = test_path;
            g_free(filename);
            break;
        }
        g_free(filename);
        g_free(test_path);
    }
    
    // Si encontramos en cache, cargar y salir
    if (found_cache_path) {
        GdkTexture *texture = gdk_texture_new_from_filename(found_cache_path, NULL);
        if (texture) {
            gtk_picture_set_paintable(GTK_PICTURE(picture), GDK_PAINTABLE(texture));
            g_object_unref(texture);
        }
        g_free(found_cache_path);
        g_free(data);
        return;
    }
    
    // Continuar con descarga
    if (!image_session) {
        image_session = soup_session_new();
    }

    SoupMessage *msg = soup_message_new("GET", url);
    
    // Verificar que el mensaje se creó correctamente
    if (!msg) {
        g_free(data->image_url);
        g_free(data->cache_key);
        g_free(data);
        return;
    }
    
    data->msg = msg;
    
    // Verificar que la sesión es válida
    if (!SOUP_IS_SESSION(image_session)) {
        g_warning("Sesión de imagen no válida");
        g_object_unref(msg);
        g_free(data->image_url);
        g_free(data->cache_key);
        g_free(data);
        return;
    }
    
    soup_session_send_and_read_async(
        image_session, msg, G_PRIORITY_DEFAULT, NULL, 
        aplicar_imagen, 
        data);
}

void limpiar_sesion_imagenes() {
    if (image_session) {
        g_object_unref(image_session);
        image_session = NULL;
    }
}