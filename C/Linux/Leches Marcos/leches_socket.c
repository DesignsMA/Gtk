#include "leches_socket.h"

SoupSession *image_session = NULL;
int socket_servidor = -1;
struct sockaddr_in servidor_addr = {0};
size_t num_categorias = 0;
size_t num_productos = 0;

Categoria *categorias = NULL;
Producto *productos = NULL;

// ==================== FUNCIONES DE COMUNICACIÓN CON EL SERVIDOR ====================

int enviar_comando_servidor(const char *comando) {
    if (send(socket_servidor, comando, strlen(comando), 0) < 0) {
        perror("Error enviando comando al servidor");
        return -1;
    }
    return 0;
}

int recibir_int_servidor() {
    int valor;
    if (recv(socket_servidor, &valor, sizeof(int), 0) < 0) {
        perror("Error recibiendo int del servidor");
        return -1;
    }
    return valor;
}

// ==================== CARGA DE CATEGORÍAS ====================

int cargar_categorias_desde_servidor() {
    if (enviar_comando_servidor("GET_CATEGORIAS") < 0) {
        return 0;
    }
    
    // Recibir número de categorías
    num_categorias = recibir_int_servidor();
    if (num_categorias <= 0) {
        return 0;
    }
    
    // Reservar memoria y recibir datos directamente
    categorias = malloc(num_categorias * sizeof(Categoria));
    if (!categorias) {
        g_warning("Error asignando memoria para categorías");
        return 0;
    }
    
    if (recv(socket_servidor, categorias, num_categorias * sizeof(Categoria), 0) < 0) {
        perror("Error recibiendo categorías del servidor");
        free(categorias);
        categorias = NULL;
        return 0;
    }
    
    g_print("Cargadas %zu categorías desde servidor\n", num_categorias);
    return num_categorias;
}

// ==================== CARGA DE PRODUCTOS ====================

int cargar_productos_desde_servidor() {
    if (enviar_comando_servidor("GET_PRODUCTOS") < 0) {
        return 0;
    }
    
    // Recibir número de productos
    num_productos = recibir_int_servidor();
    if (num_productos <= 0) {
        return 0;
    }
    
    // Reservar memoria y recibir datos directamente
    productos = malloc(num_productos * sizeof(Producto));
    if (!productos) {
        g_warning("Error asignando memoria para productos");
        return 0;
    }
    
    if (recv(socket_servidor, productos, num_productos * sizeof(Producto), 0) < 0) {
        perror("Error recibiendo productos del servidor");
        free(productos);
        productos = NULL;
        return 0;
    }
    
    g_print("Cargados %zu productos desde servidor\n", num_productos);
    return num_productos;
}

// ==================== COMPRA DE PRODUCTOS ====================

int comprar_producto_servidor(const char *product_id, int cantidad) {
    char comando[100];
    snprintf(comando, sizeof(comando), "COMPRAR_PRODUCTO:%s:%d", product_id, cantidad);
    
    if (enviar_comando_servidor(comando) < 0) {
        return -1;
    }
    
    int resultado = recibir_int_servidor();
    return resultado;
}

// ==================== FUNCIÓN PRINCIPAL DE CARGA ====================

void cargar_datos_desde_servidor() {
    // Cargar categorías
    if (cargar_categorias_desde_servidor() == 0) {
        g_warning("No se pudieron cargar categorías del servidor");
    }
    
    // Cargar productos
    if (cargar_productos_desde_servidor() == 0) {
        g_warning("No se pudieron cargar productos del servidor");
    }
}

// ==================== LIMPIAR MEMORIA ====================

void liberar_datos_servidor() {
    if (categorias) {
        free(categorias);
        categorias = NULL;
    }
    if (productos) {
        free(productos);
        productos = NULL;
    }
    num_categorias = 0;
    num_productos = 0;

    enviar_comando_servidor("EXIT");
    if (socket_servidor >= 0) {
        close(socket_servidor);
        socket_servidor = -1;
    }
}

int conectar_servidor(const char *ip, const char *puerto) {
    socket_servidor = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_servidor < 0) {
        perror("Error al crear socket");
        return -1;
    }
    
    // Configurar dirección del servidor
    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_port = htons(atoi(puerto));
    servidor_addr.sin_addr.s_addr = inet_addr(ip);
    
    // Conectar al servidor
    if (connect(socket_servidor, (struct sockaddr*)&servidor_addr, sizeof(servidor_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(socket_servidor);
        socket_servidor = -1;
        return -1;
    }
    
    return 0;
}

void aplicar_imagen(GObject *source_object, GAsyncResult *res, gpointer user_data) 
{
    SolicitudImagen *data = (SolicitudImagen *)user_data;
    GError *error = NULL;
    GBytes *bytes = soup_session_send_and_read_finish(SOUP_SESSION(source_object), res, &error); 

    if (bytes && !error) {
        // Guardar en caché
        char *cache_dir = g_build_filename(g_get_user_cache_dir(), "leches-cache", NULL);
        g_mkdir_with_parents(cache_dir, 0755);

        SoupMessageHeaders *response_headers = soup_message_get_response_headers(data->msg);
        const char *content_type = soup_message_headers_get_content_type(response_headers, NULL);
        
        const char *extension = "img";
        if (content_type) {
            if (g_str_has_prefix(content_type, "image/png")) extension = "png";
            else if (g_str_has_prefix(content_type, "image/jpeg")) extension = "jpg";
            else if (g_str_has_prefix(content_type, "image/gif")) extension = "gif";
            else if (g_str_has_prefix(content_type, "image/webp")) extension = "webp";
        }
        
        char *filename = g_strdup_printf("%s.%s", data->cache_key, extension);
        char *cache_path = g_build_filename(cache_dir, filename, NULL);
        g_file_set_contents(cache_path, g_bytes_get_data(bytes, NULL), g_bytes_get_size(bytes), NULL);
        
        // Crear pixbuf y reescalar
        GdkPixbuf *pixbuf = NULL;
        GInputStream *stream = g_memory_input_stream_new_from_bytes(bytes);
        if (stream) {
            pixbuf = gdk_pixbuf_new_from_stream(stream, NULL, NULL);
            g_object_unref(stream);
        }

        if (pixbuf && gdk_pixbuf_get_width(pixbuf) > 0 && gdk_pixbuf_get_height(pixbuf) > 0) {
            int original_width = gdk_pixbuf_get_width(pixbuf);
            int original_height = gdk_pixbuf_get_height(pixbuf);
            
            // Calcular dimensiones manteniendo aspect ratio
            double ratio = (double)original_width / original_height;
            int new_width, new_height;
            
            if (original_width > original_height) {
                new_width = data->max_width;
                new_height = data->max_width / ratio;
            } else {
                new_height = data->max_height;
                new_width = data->max_height * ratio;
            }
            
            // Asegurar que no exceda los límites
            if (new_width > data->max_width) {
                new_width = data->max_width;
                new_height = data->max_width / ratio;
            }
            if (new_height > data->max_height) {
                new_height = data->max_height;
                new_width = data->max_height * ratio;
            }
            
            GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
            GdkTexture *texture = gdk_texture_new_for_pixbuf(scaled_pixbuf);
            gtk_picture_set_paintable(data->picture, GDK_PAINTABLE(texture));
            
            g_object_unref(scaled_pixbuf);
            g_object_unref(pixbuf);
            g_object_unref(texture);
        }
        else {
            g_warning("Error creando pixbuf desde datos descargados");
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

void cargar_imagen(const char* url, GtkPicture *picture, int max_width, int max_height) {
    SolicitudImagen *data = g_new0(SolicitudImagen, 1);
    data->image_url = g_strdup(url);
    data->picture = picture;
    data->max_width = max_width;
    data->max_height = max_height;
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
    
    // Si encontramos en cache, cargar y reescalar
    if (found_cache_path) {
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(found_cache_path, NULL);
        if (pixbuf && gdk_pixbuf_get_width(pixbuf) > 0 && gdk_pixbuf_get_height(pixbuf) > 0) {
            int original_width = gdk_pixbuf_get_width(pixbuf);
            int original_height = gdk_pixbuf_get_height(pixbuf);
            
            // Calcular dimensiones manteniendo aspect ratio
            double ratio = (double)original_width / original_height;
            int new_width, new_height;
            
            if (original_width > original_height) {
                new_width = max_width;
                new_height = max_width / ratio;
            } else {
                new_height = max_height;
                new_width = max_height * ratio;
            }
            
            // Asegurar que no exceda los límites
            if (new_width > max_width) {
                new_width = max_width;
                new_height = max_width / ratio;
            }
            if (new_height > max_height) {
                new_height = max_height;
                new_width = max_height * ratio;
            }
            
            GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
            GdkTexture *texture = gdk_texture_new_for_pixbuf(scaled_pixbuf);
            gtk_picture_set_paintable(picture, GDK_PAINTABLE(texture));
            
            g_object_unref(scaled_pixbuf);
            g_object_unref(pixbuf);
            g_object_unref(texture);
        }
        else {
            g_warning("Error creando pixbuf desde caché para %s", url);
        }
        
        g_free(found_cache_path);
        g_free(data);
        return;
    }
    
    // Continuar con descarga asíncrona
    if (!image_session) {
        image_session = soup_session_new();
    }

    SoupMessage *msg = soup_message_new("GET", url);
    
    if (!msg) {
        g_free(data->image_url);
        g_free(data->cache_key);
        g_free(data);
        return;
    }
    
    data->msg = msg;
    
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