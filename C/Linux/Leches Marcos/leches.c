#include <gtk/gtk.h>
#include <glib/gstdio.h> // standard glib in out
#include <json-glib/json-glib.h> //sudo apt install libjson-glib-dev
#include "leches_socket.h"
#include "leches_dynamicLists.h"
#include <fontconfig/fontconfig.h>

typedef struct {
    char id[37];           // UUID del producto
    char nombre[100];
    char descripcion[200];
    float precio;
    int cantidad;
    char categoria[50];
    char subcategoria[50];
    char imagen_url[300];
} CarritoItem;

// Variables globales
GHashTable *carrito = NULL; // Hash table para el carrito de compras, asocia ID de producto con CarritoItem

static void cargar_css();
// static indica que e suna funcion privada de este archivo
static void quit_cb (GtkWindow *window);
static void filtrar_categoria_cb (GtkWidget *widget, gpointer   user_data);
static void cargar_categorias();
static void inicio_cb (GtkWidget *widget, gpointer   user_data);
void ver_modo_lista_cb(GtkToggleButton *button, gpointer user_data);
void ver_modo_grid_cb(GtkToggleButton *button, gpointer user_data);
void ver_detalles_cb(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
static void buscar_producto_cb(GtkSearchEntry *entry, gpointer user_data);

// FUNCIONES DE CARRITO DE COMPRAS
void añadir_al_carrito_cb(GtkButton *button, gpointer user_data);
void cerrar_carrito_cb (GtkWidget *widget, gpointer   user_data);
void finalizar_compra_cb (GtkWidget *widget, gpointer   user_data);
void finalizar_carrito();
void vaciar_carrito_cb (GtkWidget *widget, gpointer   user_data);
void actualizar_ui_carrito();
void actualizar_badge_carrito();
void on_cantidad_cambiada(GtkSpinButton *spin_button, gpointer user_data);
void on_eliminar_item(GtkButton *button, gpointer user_data);
void actualizar_total_carrito();
void eliminar_item_carrito(const char *product_id);

void mostrar_error(const char *mensaje);

void cargar_carrito_archivo();
void guardar_carrito_archivo();
// ----------------------------------

void card_button_clicked(GtkButton *btn, gpointer data);
static void fabricar_card_productos();

static void on_subcategory_toggled(GtkCheckButton *button, gpointer user_data);
void update_subcategory_filters(GtkWidget *filter_box, const char *category_name);
void add_subcategory_checkboxes(GtkWidget *filter_box, Categoria *categoria);
void cargar_filtros();

void inicializar_variables_globales() {
    // Inicializar carrito si no está inicializado
    if (!carrito) {
        carrito = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    }

}


GObject *window; // Ventana principal
GObject *main_stack; // Stack de vistas principales
GObject *category_box; // Caja de categorías
GObject *header_button; // Botón de inicio en el header
GObject *header_search; // Caja de busqueda
GObject *header_icon; // Icono en el header
GObject *welcome_image; // Imagen de bienvenida
GObject *welcome_button; // Botón de bienvenida
GObject *products_stack;
GObject *button_list;
GObject *button_grid;

GtkRevealer *carrito_revealer; // Revealer del carrito
GtkWidget *carrito_products_list; // Lista de productos en el carrito
GtkLabel *cart_badge; // Insignia del carrito
GtkLabel *carrito_items_count; // Etiqueta de cantidad de items en el carrito
GtkLabel *carrito_total_amount; // Etiqueta de monto total del carrito
GtkRevealer *error_revealer; // Revealer de errores
GtkRevealer *checkout_revealer; // Revealer de checkout
GtkLabel *error_label; // Etiqueta de mensaje de error
GtkLabel *checkout_label; // Etiqueta de mensaje de checkout

GtkBuilder *builder; // Builder global

static void load_font_from_resource(const char *resource_path)
{
    // 1. Obtener los datos del recurso
    GBytes *bytes = g_resources_lookup_data(resource_path, 0, NULL);
    gsize size;
    const guint8 *data = g_bytes_get_data(bytes, &size);

    // 2. Crear archivo temporal (sin extensión)
    char *tmp = g_strdup("/tmp/fontXXXXXX");
    int fd = g_mkstemp(tmp);

    // 3. Escribir archivo
    write(fd, data, size);
    close(fd);

    // 4. Renombrarlo a .ttf (FontConfig quiere extensión correcta)
    char *tmp_ttf = g_strdup_printf("%s.ttf", tmp);
    rename(tmp, tmp_ttf);

    // 5. Registrar la fuente con Fontconfig
    FcConfig *config = FcConfigGetCurrent();
    FcConfigAppFontAddFile(config, (FcChar8 *)tmp_ttf);

    // Limpieza
    g_free(tmp);
    g_free(tmp_ttf);
    g_bytes_unref(bytes);
}

static void descargar_datos(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
    // HILO SECUNDARIO: descargar datos del servidor
}

/*#################################### Cargar programa ###################################################*/
static void
activate ( GtkApplication *app,
           gpointer user_data)
{
    // Cargar CSS primero
    cargar_css();

    /*Nueva instancia de GtkBuilder para cargar nuestro UI desde un gresource*/
    builder = gtk_builder_new_from_resource("/res/ui/start.ui");

    /*Conectar handlers de señales a loc widgets construidos*/
    window = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);

    main_stack = gtk_builder_get_object(builder, "main_stack"); // Stack de vistas principales

    category_box = gtk_builder_get_object(builder, "category_box"); // Caja de categorías

    header_button = gtk_builder_get_object(builder, "header_button"); // Botón de inicio en el header

    header_search = gtk_builder_get_object(builder, "header_search");
    g_signal_connect(header_search, "search-changed", G_CALLBACK(buscar_producto_cb), NULL);

    /*Cargando iconos e imagenes desde recursos*/
    header_icon = gtk_builder_get_object(builder, "header_icon"); // Icono en el header
    gtk_image_set_from_resource(GTK_IMAGE(header_icon), "/res/images/logo.svg"); // Establecer imagen desde recurso
    welcome_image = gtk_builder_get_object(builder, "welcome_image"); // Imagen de bienvenida
    gtk_picture_set_resource(GTK_PICTURE(welcome_image), "/res/images/bg.jpg"); // Establecer imagen desde recurso

    welcome_button = gtk_builder_get_object(builder, "welcome_button"); // Botón de bienvenida
    g_signal_connect(welcome_button, "clicked", G_CALLBACK(filtrar_categoria_cb), NULL); // Conectar señal de clic al botón

    // Boton inicio
    GtkWidget *button = gtk_button_new_with_label("TODO");
    g_signal_connect(button, "clicked",  G_CALLBACK(filtrar_categoria_cb), NULL); // Conectar señal de clic al botón
    gtk_widget_add_css_class(button, "category-button"); // Añadir clase CSS para estilos
    gtk_box_append(GTK_BOX(category_box), button); // Añadir botón a la caja de categorías
    gtk_widget_set_visible(button, TRUE); // mostrar boton

    g_signal_connect(header_button, "clicked", G_CALLBACK(inicio_cb), NULL); // Conectar señal de clic al botón de inicio

    // Seccion de productos
    products_stack = gtk_builder_get_object(builder, "products_stack");
    button_list = gtk_builder_get_object(builder, "button_list");
    button_grid = gtk_builder_get_object(builder, "button_grid"); 

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_list), TRUE); // por defecto activo

    // Carrito de compras
    carrito_revealer = GTK_REVEALER(gtk_builder_get_object(builder, "carrito_revealer"));
    carrito_products_list = GTK_WIDGET(gtk_builder_get_object(builder, "carrito_products_list")); 
    cart_badge = GTK_LABEL(gtk_builder_get_object(builder, "cart_badge"));
    carrito_items_count = GTK_LABEL(gtk_builder_get_object(builder, "carrito_items_count"));
    carrito_total_amount = GTK_LABEL(gtk_builder_get_object(builder, "carrito_total_amount"));

    error_revealer = GTK_REVEALER(gtk_builder_get_object(builder, "error_revealer"));
    error_label = GTK_LABEL(gtk_builder_get_object(builder, "error_label"));

    checkout_revealer = GTK_REVEALER(gtk_builder_get_object(builder, "checkout_revealer"));
    checkout_label = GTK_LABEL(gtk_builder_get_object(builder, "checkout_label"));

    gtk_widget_set_sensitive(GTK_WIDGET(error_revealer), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(checkout_revealer), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(carrito_revealer), FALSE);


    cargar_productos(G_OBJECT(category_box), G_CALLBACK(filtrar_categoria_cb)); // Cargar productos en la tienda

    fabricar_card_productos(builder); // Configurar fábrica de cards para productos

    cargar_filtros(); // Cargar filtros de categorías y subcategorías

    // Hashtable que usa el ID del producto como clave
    // hash string, valor struct CarritoItem*, compara con igualdad de strings

    cargar_carrito_archivo();
    actualizar_badge_carrito();

    gtk_widget_set_visible (GTK_WIDGET (window), TRUE);
}

/*#########################################################################################################*/

int
main (int   argc,
      char *argv[])
{
#ifdef GTK_SRCDIR
  g_chdir (GTK_SRCDIR);
#endif

    inicializar_variables_globales();

    // Pedir IP y puerto por consola
    char ip[16] = "127.0.0.1";  // valor por defecto
    char puerto[6] = "7777";    // valor por defecto
    
    printf("=== CLIENTE LECHES ===\n");
    printf("Ingrese IP del servidor [127.0.0.1]: ");
    if (fgets(ip, sizeof(ip), stdin) != NULL) {
        // Eliminar newline
        ip[strcspn(ip, "\n")] = '\0';
        // Si está vacío, usar valor por defecto
        if (strlen(ip) == 0) {
            strcpy(ip, "127.0.0.1");
        }
    }
    
    printf("Ingrese puerto del servidor: ");
    if (fgets(puerto, sizeof(puerto), stdin) != NULL) {
        // Eliminar newline
        puerto[strcspn(puerto, "\n")] = '\0';
        // Si está vacío, usar valor por defecto
        if (strlen(puerto) == 0) {
            strcpy(puerto, "7777");
        }
    }
    
    printf("Conectando al servidor: %s:%s\n", ip, puerto);
    if (conectar_servidor(ip, puerto) == 0) {
        cargar_datos_desde_servidor();
        printf("Conexión exitosa!\n");
    } else {
        g_warning("No se pudo conectar al servidor, usando modo sin servidor");
    }
    
    gtk_init();

    // Registrar el GResource
    GResource *res = g_resource_load("resources.gresource", NULL);
    if (!res) {
        g_error("No se pudo cargar resources.gresource");
    }
    g_resources_register(res);

    // Cargar fuentes desde el recurso
    load_font_from_resource("/res/fonts/Montserrat.ttf");
    load_font_from_resource("/res/fonts/BebasNeue.ttf");

    GtkApplication *app = gtk_application_new ("com.github.designsma.leches", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

    int status = g_application_run (G_APPLICATION (app), argc, argv);

    finalizar_carrito();

    liberar_datos_servidor();

    limpiar_sesion_imagenes();

    g_resources_unregister(res);
    g_object_unref(builder);
    g_object_unref (app);

    return status;
}

static void cargar_css() {
    GtkCssProvider *provider = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(provider, "start.css");

    // Pantalla principal
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );


    gtk_icon_theme_add_resource_path(
    gtk_icon_theme_get_for_display(gdk_display_get_default()),
    "/data/icons/scalable/actions"
    );

    g_object_unref(provider);
}

typedef struct {
    GtkPicture *image;
    GtkLabel *title;
    GtkLabel *subcategory;
    GtkLabel *description;
    GtkLabel *price;
    GtkButton *button;
} CardWidgets;

char* capitalize_for_categories(const char *str) {
    char *result = g_strdup(str);
    if (!result) return NULL;
    
    for (int i = 0; result[i] != '\0'; i++) {
        if (i == 0 || isspace(result[i-1])) {
            result[i] = toupper(result[i]);
        }
    }
    return result;
}

char* capitalize_first_char(const char *str) {
    if (str == NULL || *str == '\0') {
        return g_strdup("");
    }
    
    char *result = g_strdup(str);
    if (result == NULL) return NULL;
    
    // Solo convertir el primer carácter a mayúscula
    if (isalpha((unsigned char)result[0])) {
        result[0] = toupper((unsigned char)result[0]);
    }
    
    return result;
}

static void
setup_card(GtkListItemFactory *factory, GtkListItem *list_item)
{
    GtkBuilder *builder;
    
    builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/res/ui/card.ui", NULL);
    
    GtkWidget *card = GTK_WIDGET(gtk_builder_get_object(builder, "card"));
    
    // Obtener todos los widgets importantes una sola vez
    CardWidgets *widgets = g_new0(CardWidgets, 1);
    widgets->image = GTK_PICTURE(gtk_builder_get_object(builder, "card_image"));
    widgets->title = GTK_LABEL(gtk_builder_get_object(builder, "card_title"));
    widgets->subcategory = GTK_LABEL(gtk_builder_get_object(builder, "card_subcategory"));
    widgets->description = GTK_LABEL(gtk_builder_get_object(builder, "card_description"));
    widgets->price = GTK_LABEL(gtk_builder_get_object(builder, "card_price"));
    widgets->button = GTK_BUTTON(gtk_builder_get_object(builder, "card_button"));
    
    gtk_picture_set_resource(widgets->image, "/res/images/placeholder.png");

    gtk_widget_set_size_request(GTK_WIDGET(widgets->image), 250, 250);


    // Guardar los widgets en el list_item para usarlos en bind
    // usamos un data full porque hay que liberar la estructura al final de la vida del list_item
    g_object_set_data_full(G_OBJECT(list_item), "card-widgets", widgets, g_free);
    
    gtk_list_item_set_child(list_item, card);

    gtk_list_item_set_selectable(list_item, FALSE); // No seleccionable
    gtk_list_item_set_focusable(list_item, FALSE); // No focusable
    gtk_list_item_set_activatable(list_item, FALSE); // No activatable
    g_object_unref(builder);
}

static void
bind_card(GtkListItemFactory *factory, GtkListItem *list_item)
{
    CardWidgets *widgets = g_object_get_data(G_OBJECT(list_item), "card-widgets");
    if (!widgets) return;
    
    GObject *product = gtk_list_item_get_item(list_item);
    
    const char *nombre = NULL;
    const char *subcat = NULL;
    const char *descripcion = NULL;
    const char *img_url = NULL;
    float precio = 0.0f;
    
    g_object_get(product,
         "nombre", &nombre,
         "subcategoria", &subcat, 
         "descripcion", &descripcion,
         "imagen_url", &img_url,
         "precio", &precio,
         NULL);
    
    gtk_label_set_text(widgets->title, capitalize_for_categories(nombre));
    gtk_label_set_text(widgets->subcategory, capitalize_for_categories(subcat));
    gtk_label_set_text(widgets->description, capitalize_first_char(descripcion));
    
    // Formatear precio
    char price_str[32];
    g_snprintf(price_str, sizeof(price_str), "$%.2f", precio);
    gtk_label_set_text(widgets->price, price_str);
    
    // Cargar imagen
    if (img_url) cargar_imagen(img_url, widgets->image, 250, 250);
    
    // Guardar product en el botón para usarlo después
    g_object_set_data(G_OBJECT(widgets->button), "product", product);

    g_free((gpointer)nombre);
    g_free((gpointer)subcat);
    g_free((gpointer)descripcion);
    g_free((gpointer)img_url);
}

static void
setup_minicard(GtkListItemFactory *factory, GtkListItem *list_item)
{
    GtkBuilder *builder;
    
    builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/res/ui/minicard.ui", NULL);
    
    GtkWidget *card = GTK_WIDGET(gtk_builder_get_object(builder, "card"));
    
    // Obtener todos los widgets importantes una sola vez
    CardWidgets *widgets = g_new0(CardWidgets, 1);
    widgets->image = GTK_PICTURE(gtk_builder_get_object(builder, "card_image"));
    widgets->title = GTK_LABEL(gtk_builder_get_object(builder, "card_title"));
    widgets->subcategory = GTK_LABEL(gtk_builder_get_object(builder, "card_subcategory"));
    widgets->price = GTK_LABEL(gtk_builder_get_object(builder, "card_price"));
    widgets->button = GTK_BUTTON(gtk_builder_get_object(builder, "card_button"));
    
    gtk_picture_set_resource(widgets->image, "/res/images/placeholder.png");
    gtk_widget_set_size_request(GTK_WIDGET(widgets->image), 250, 250);

    // Guardar los widgets en el list_item para usarlos en bind
    // usamos un data full porque hay que liberar la estructura al final de la vida del list_item
    g_object_set_data_full(G_OBJECT(list_item), "card-widgets", widgets, g_free);
    
    gtk_list_item_set_child(list_item, card);
    gtk_list_item_set_selectable(list_item, FALSE); // No seleccionable
    gtk_list_item_set_focusable(list_item, FALSE); // No focusable
    gtk_list_item_set_activatable(list_item, FALSE); // No activatable
}

static void
bind_minicard(GtkListItemFactory *factory, GtkListItem *list_item)
{
    CardWidgets *widgets = g_object_get_data(G_OBJECT(list_item), "card-widgets");
    if (!widgets) return;
    
    GObject *product = gtk_list_item_get_item(list_item);
    
    const char *nombre = NULL;
    const char *subcat = NULL;
    const char *img_url = NULL;
    float precio = 0.0f;
    
    g_object_get(product,
         "nombre", &nombre,
         "subcategoria", &subcat, 
         "imagen_url", &img_url,
         "precio", &precio,
         NULL);
    
    gtk_label_set_text(widgets->title, capitalize_for_categories(nombre));
    gtk_label_set_text(widgets->subcategory, capitalize_for_categories(subcat));
    
    // Formatear precio
    char price_str[32];
    g_snprintf(price_str, sizeof(price_str), "$%.2f", precio);
    gtk_label_set_text(widgets->price, price_str);
    
    // Cargar imagen
    if (img_url) cargar_imagen(img_url, widgets->image, 250, 250);
    
    // Guardar product en el botón para usarlo después
    g_object_set_data(G_OBJECT(widgets->button), "product", product);

    g_free((gpointer)nombre);
    g_free((gpointer)subcat);
    g_free((gpointer)img_url);

}


static void fabricar_card_productos(GtkBuilder *builder) {
    // Fábrica de cards para la vista de lista y cuadrícula a partir del producto
    card_factory = gtk_signal_list_item_factory_new();

    // Configurar la fábrica para crear cards personalizadas
    g_signal_connect(card_factory, "setup", G_CALLBACK(setup_card), NULL); // configurar widget
    g_signal_connect(card_factory, "bind", G_CALLBACK(bind_card), NULL); // vincular widget con datos (items del modelo)
    
    minicard_factory = gtk_signal_list_item_factory_new();

    // Configurar la fábrica para crear cards personalizadas
    g_signal_connect(minicard_factory, "setup", G_CALLBACK(setup_minicard), NULL);
    g_signal_connect(minicard_factory, "bind", G_CALLBACK(bind_minicard), NULL);

    list_view = GTK_LIST_VIEW(gtk_builder_get_object(builder, "list_view"));
    gtk_list_view_set_factory(GTK_LIST_VIEW(list_view), card_factory);
    // no necesitamos seleccionar items en la lista
    gtk_list_view_set_model(GTK_LIST_VIEW(list_view), GTK_SELECTION_MODEL(selection));
    gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(list_view), FALSE); // activar con un solo clic

    grid_view = GTK_GRID_VIEW(gtk_builder_get_object(builder, "grid_view"));
    gtk_grid_view_set_factory(GTK_GRID_VIEW(grid_view), minicard_factory);
    // no necesitamos seleccionar items en la cuadrícula
    gtk_grid_view_set_model(GTK_GRID_VIEW(grid_view), GTK_SELECTION_MODEL(selection));
    gtk_grid_view_set_single_click_activate(GTK_GRID_VIEW(grid_view), FALSE); // activar con un solo clic

    // Vista de destacados, independiente del pipeline principal

    featured_view = GTK_GRID_VIEW(gtk_builder_get_object(builder, "featured_view"));
    gtk_grid_view_set_factory(GTK_GRID_VIEW(featured_view), minicard_factory);

    gtk_grid_view_set_model(GTK_GRID_VIEW(featured_view), GTK_SELECTION_MODEL(selectionstock));
    gtk_grid_view_set_single_click_activate(GTK_GRID_VIEW(featured_view), FALSE); // activar con un solo clic
}

static void
quit_cb (GtkWindow *window)
{
    gtk_window_close(window);
}

void cambiar_categoria_filtro(const char *nueva_categoria) {
    GtkWidget *filter_box = GTK_WIDGET(gtk_builder_get_object(
        builder, 
        "filter_box"));
    
    // Actualizar el estado del filtro
    if (nueva_categoria == NULL) {
        filter_state.selected_category = NULL;
    } else {
        filter_state.selected_category = nueva_categoria;
    }
    
    // Limpiar selecciones anteriores de subcategorías
    filter_state_clear(&filter_state);
    
    // Actualizar la UI de filtros
    update_subcategory_filters(filter_box, filter_state.selected_category);
    
    // Actualizar los filtros del modelo
    gtk_filter_changed(GTK_FILTER(filter_state.category_filter), GTK_FILTER_CHANGE_DIFFERENT);
    gtk_filter_changed(GTK_FILTER(filter_state.subcategory_filter), GTK_FILTER_CHANGE_DIFFERENT);
}


static void
filtrar_categoria_cb (GtkWidget *widget,
                  gpointer   user_data)
{
    const char* categoria =  g_object_get_data( G_OBJECT(widget), "categoria");
    gtk_stack_set_visible_child_name(GTK_STACK(main_stack), "products"); // Cambiar a la página de productos
    cambiar_categoria_filtro(categoria);
}

static void
inicio_cb (GtkWidget *widget,
            gpointer   user_data)
{
    gtk_stack_set_visible_child_name(GTK_STACK(main_stack), "welcome"); // Cambiar a la página de inicio
}

void ver_detalles_cb(GtkGestureClick *gesture,
                            int n_press,
                            double x,
                            double y,
                            gpointer user_data)
{
    GtkWidget *card = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

    // Obtenemos el widget real donde ocurrió el clic
    GtkWidget *target = gtk_widget_pick(card, x, y, GTK_PICK_DEFAULT);

    // Si se clickeó un botón, ignoramos el gesto del card
    if (GTK_IS_BUTTON(target)) {
        gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_DENIED);
        return;
    }

    g_print("Clic detectado!\n");
}

void card_button_clicked(GtkButton *btn, gpointer data) {
    g_print("Botón clickeado\n");
}

void ver_modo_lista_cb(GtkToggleButton *button, gpointer user_data)
{
    gboolean is_active = gtk_toggle_button_get_active(button);
    if (is_active)
    {   
        gtk_stack_set_visible_child_name(GTK_STACK(products_stack), "list"); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_grid), FALSE);
    }
}


void ver_modo_grid_cb(GtkToggleButton *button, gpointer user_data)
{
    gboolean is_active = gtk_toggle_button_get_active(button);
    if (is_active)
    {   
        gtk_stack_set_visible_child_name(GTK_STACK(products_stack), "grid"); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_list), FALSE);
    }
}
// Funciones para filtro
void cargar_filtros() {
    GtkWidget *filter_box = GTK_WIDGET(gtk_builder_get_object(builder, "filter_box"));
    
    // Simplemente cargamos los checkboxes de subcategorías según la categoría actual
    update_subcategory_filters(filter_box, filter_state.selected_category);
}

// Callback cuando se cambia el estado de un checkbox de subcategoría
static void on_subcategory_toggled(GtkCheckButton *button, gpointer user_data) {
    const char *subcategory = (const char *)user_data;
    FilterState *filter_state_p = &filter_state; // Tu variable global
    
    filter_state_toggle_subcategory(filter_state_p, subcategory);
    
    // Actualizar el filtro
    gtk_filter_changed(GTK_FILTER(filter_state_p->subcategory_filter), GTK_FILTER_CHANGE_DIFFERENT);

}

// Función para actualizar los checkboxes de subcategorías
void update_subcategory_filters(GtkWidget *filter_box, const char *category_name) {
    // Limpiar todos los widgets existentes del filter_box
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(filter_box)) != NULL) {
        gtk_box_remove(GTK_BOX(filter_box), child);
    }

    if (category_name == NULL) {
        // Si no hay categoría seleccionada, mostrar todas las subcategorías de todas las categorías
        for (int i = 0; i < num_categorias && categorias[i].nombre[0] != '\0'; i++) {
            add_subcategory_checkboxes(filter_box, &categorias[i]);
        }
    } else {
        // Encontrar la categoría específica y mostrar solo sus subcategorías
        gboolean categoria_encontrada = FALSE;
        for (int i = 0; i < num_categorias && categorias[i].nombre[0] != '\0'; i++) {
            if (g_strcmp0(categorias[i].nombre, category_name) == 0) {
                add_subcategory_checkboxes(filter_box, &categorias[i]);
                categoria_encontrada = TRUE;
                break;
            }
        }
        
        // Si no se encontró la categoría, mostrar todas
        if (!categoria_encontrada) {
            for (int i = 0; i < num_categorias && categorias[i].nombre[0] != '\0'; i++) {
                add_subcategory_checkboxes(filter_box, &categorias[i]);
            }
        }
    }
    
    // Forzar redibujado
    gtk_widget_queue_draw(filter_box);
}

// Función para agregar checkboxes de una categoría específica
void add_subcategory_checkboxes(GtkWidget *filter_box, Categoria *categoria) {
    // Agregar separador
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_add_css_class(separator, "filter-separator");
    gtk_box_append(GTK_BOX(filter_box), separator);
    
    // Agregar label de categoría
    GtkWidget *category_label = gtk_label_new( capitalize_for_categories(categoria->nombre));
    gtk_widget_set_halign(category_label, GTK_ALIGN_START);
    gtk_label_set_xalign(GTK_LABEL(category_label), 0.0);
    gtk_widget_add_css_class(category_label, "filter-category-label");
    gtk_box_append(GTK_BOX(filter_box), category_label);
    
    // Agregar checkboxes para cada subcategoría
    for (int j = 0; j < categoria->num_subcategorias; j++) {
        GtkWidget *check = gtk_check_button_new_with_label(capitalize_for_categories(categoria->subcategorias[j]));
        gtk_widget_set_halign(check, GTK_ALIGN_START);
        gtk_widget_add_css_class(check, "filter-checkbutton");
        
        g_signal_connect(check, "toggled", G_CALLBACK(on_subcategory_toggled), 
                        (gpointer)categoria->subcategorias[j]);
        gtk_box_append(GTK_BOX(filter_box), check);
    }
}

static void 
buscar_producto_cb(GtkSearchEntry *entry, gpointer user_data) {
    const char *query = gtk_editable_get_text(GTK_EDITABLE(entry)); 
    // Actualizar el query de búsqueda
    filter_state.search_query = query;
    
    // Actualizar el filtro
    if (filter_state.search_filter) {
        gtk_filter_changed(GTK_FILTER(filter_state.search_filter), 
                          GTK_FILTER_CHANGE_DIFFERENT);
    }
}

// Funciones de carrito de compras

// Servidor
// Función de verificación en servidor
gboolean verificar_stock_servidor(const char *product_id, int cantidad_deseada) {
    // Enviar comando al servidor
    char comando[256];
    snprintf(comando, sizeof(comando), "VERIFICAR_STOCK:%s:%d", product_id, cantidad_deseada);
    send(socket_servidor, comando, strlen(comando), 0);
    
    // Recibir respuesta
    int stock_disponible;
    if (recv(socket_servidor, &stock_disponible, sizeof(int), 0) > 0) {
        return stock_disponible >= cantidad_deseada;
    }
    
    return FALSE; // En caso de error, asumir sin stock
}

// Cliente

gboolean hide_error_timeout(gpointer user_data) {
    if (error_revealer) {
        gtk_revealer_set_reveal_child(error_revealer, FALSE);
    }
    return G_SOURCE_REMOVE;  // No repetir
}

gboolean hide_checkout_timeout(gpointer user_data) {
    if (checkout_revealer) {
        gtk_revealer_set_reveal_child(checkout_revealer, FALSE);
    }
    return G_SOURCE_REMOVE;  // No repetir
}

void mostrar_error(const char *mensaje) {
    gtk_label_set_text(error_label, mensaje);
    gtk_revealer_set_reveal_child(error_revealer, TRUE);
    
    // Ocultar el error después de 3 segundos
    g_timeout_add_seconds(3, hide_error_timeout, NULL);
}

void mostrar_checkout(const char *mensaje) {
    gtk_label_set_text(checkout_label, mensaje);
    gtk_revealer_set_reveal_child(checkout_revealer, TRUE);
    
    // Ocultar el error después de 7 segundos
    g_timeout_add_seconds(7, hide_checkout_timeout, NULL);
}


void añadir_al_carrito_cb(GtkButton *button, gpointer user_data) {
    GObject *product = G_OBJECT(g_object_get_data(G_OBJECT(button), "product"));
    
    // Obtener todos los datos del producto
    const char *id, *nombre, *desc, *cat, *subcat, *img;
    float precio;
    
    g_object_get(product,
        "id", &id, "nombre", &nombre, "descripcion", &desc,
        "precio", &precio, "categoria", &cat, 
        "subcategoria", &subcat, "imagen_url", &img,
        NULL);
    
    if (!id) return;

    if (!verificar_stock_servidor(id, 1)) {
        mostrar_error("Producto sin stock disponible");
        return;
    }
    
    // Buscar si ya existe
    CarritoItem *item = g_hash_table_lookup(carrito, id);
    
    if (item) {
        item->cantidad++;
    } else {
        // Crear nuevo item con copia de datos
        CarritoItem *nuevo_item = g_new0(CarritoItem, 1);
        
        g_strlcpy(nuevo_item->id, id, sizeof(nuevo_item->id));
        g_strlcpy(nuevo_item->nombre, nombre, sizeof(nuevo_item->nombre));
        g_strlcpy(nuevo_item->descripcion, desc, sizeof(nuevo_item->descripcion));
        g_strlcpy(nuevo_item->categoria, cat, sizeof(nuevo_item->categoria));
        g_strlcpy(nuevo_item->subcategoria, subcat, sizeof(nuevo_item->subcategoria));
        g_strlcpy(nuevo_item->imagen_url, img, sizeof(nuevo_item->imagen_url));
        nuevo_item->precio = precio;
        nuevo_item->cantidad = 1;
        
        g_hash_table_insert(carrito, g_strdup(id), nuevo_item);
    }
    
    actualizar_ui_carrito();
    actualizar_badge_carrito();
}

void configurar_item_carrito(GtkWidget *cart_item, GtkBuilder *item_builder, CarritoItem *item) {
    // Obtener los widgets del item
    GtkPicture *product_image = GTK_PICTURE(gtk_builder_get_object(item_builder, "product_image"));
    GtkLabel *product_title = GTK_LABEL(gtk_builder_get_object(item_builder, "product_title"));
    GtkSpinButton *quantity_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(item_builder, "quantity_spin"));
    GtkLabel *price_label = GTK_LABEL(gtk_builder_get_object(item_builder, "price_label"));
    GtkButton *delete_button = GTK_BUTTON(gtk_builder_get_object(item_builder, "delete_button"));
    
    // Configurar la imagen
    if (item->imagen_url && item->imagen_url[0] != '\0') {
        cargar_imagen(item->imagen_url, product_image, 85, 85);
    } else {
        gtk_picture_set_resource(product_image, "/res/images/placeholder.png");
    }
    
    // Configurar el título
    gtk_label_set_text(product_title, capitalize_for_categories(item->nombre));
    
    // Configurar el spin button
    GtkAdjustment *adj = gtk_spin_button_get_adjustment(quantity_spin);
    gtk_adjustment_set_value(adj, item->cantidad);
    
    // Conectar la señal de cambio de cantidad
    g_signal_connect_data(quantity_spin, "value-changed", 
                         G_CALLBACK(on_cantidad_cambiada), 
                         g_strdup(item->id), 
                         (GClosureNotify)g_free, 0);
    
    // Configurar el precio
    char precio_text[50];
    g_snprintf(precio_text, sizeof(precio_text), "x $%.2f", item->precio);
    gtk_label_set_text(price_label, precio_text);
    
    // Configurar el botón de eliminar
    g_signal_connect_data(delete_button, "clicked", 
                         G_CALLBACK(on_eliminar_item), 
                         g_strdup(item->id), 
                         (GClosureNotify)g_free, 0);
    
    // Guardar referencia al item en el widget principal
    g_object_set_data_full(G_OBJECT(cart_item), "carrito-item", 
                          g_memdup2(item, sizeof(CarritoItem)), 
                          g_free);
}

void on_cantidad_cambiada(GtkSpinButton *spin_button, gpointer user_data) {
    const char *product_id = (const char *)user_data;
    int nueva_cantidad = gtk_spin_button_get_value_as_int(spin_button);
    
    if (nueva_cantidad <= 0) {
        // Eliminar el item si la cantidad es 0 o menor
        eliminar_item_carrito(product_id);
        return;
    }
    
    // Verificar stock antes de actualizar
    if (!verificar_stock_servidor(product_id, nueva_cantidad)) {
        mostrar_error("Stock insuficiente");
        
        // Revertir al valor anterior
        CarritoItem *item = g_hash_table_lookup(carrito, product_id);
        if (item) {
            gtk_spin_button_set_value(spin_button, item->cantidad);
        }
        return;
    }
    
    // Actualizar la cantidad en el carrito
    CarritoItem *item = g_hash_table_lookup(carrito, product_id);
    if (item) {
        item->cantidad = nueva_cantidad;
    }
    
    actualizar_total_carrito();
    actualizar_badge_carrito();

}

void on_eliminar_item(GtkButton *button, gpointer user_data) {
    const char *product_id = (const char *)user_data;
    eliminar_item_carrito(product_id);
    actualizar_badge_carrito();
}

void eliminar_item_carrito(const char *product_id) {
    g_hash_table_remove(carrito, product_id);
    actualizar_ui_carrito();
}

// Función para calcular el total del carrito
float calcular_total_carrito() {
    float total = 0.0;
    
    GHashTableIter iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&iter, carrito);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CarritoItem *item = (CarritoItem *)value;
        total += item->precio * item->cantidad;
    }
    
    return total;
}

// Función auxiliar para obtener cantidad total de items
int obtener_cantidad_total_carrito() {
    int total = 0;
    GHashTableIter iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&iter, carrito);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CarritoItem *item = (CarritoItem *)value;
        total += item->cantidad;
    }
    
    return total;
}

void actualizar_badge_carrito() {
    int cantidad_total = obtener_cantidad_total_carrito();
    
    if (cantidad_total > 0) {
        char badge_text[10];
        g_snprintf(badge_text, sizeof(badge_text), "%d", cantidad_total);
        gtk_label_set_text(cart_badge, badge_text);
        gtk_widget_set_visible(GTK_WIDGET(cart_badge), TRUE);
    } else {
        gtk_widget_set_visible(GTK_WIDGET(cart_badge), FALSE);
    }
}

// Función actualizada para actualizar el total en la UI
void actualizar_total_carrito() {
    float total = calcular_total_carrito();
    
    // Actualizar la UI del total
    GtkLabel *total_label = GTK_LABEL(gtk_builder_get_object(builder, "carrito_total_amount"));
    if (total_label) {
        char total_text[50];
        g_snprintf(total_text, sizeof(total_text), "$%.2f", total);
        gtk_label_set_text(total_label, total_text);
    }

    GtkLabel *items_count_label = GTK_LABEL(gtk_builder_get_object(builder, "carrito_items_count"));
    if (items_count_label) {
        int cantidad_total = obtener_cantidad_total_carrito();
        char items_text[50];
        g_snprintf(items_text, sizeof(items_text), "%d productos", cantidad_total);
        gtk_label_set_text(items_count_label, items_text);
    }
}

// Función actualizada para finalizar compra
void finalizar_compra_cb(GtkWidget *widget, gpointer user_data) {
    g_print("Finalizando compra...\n");
    
    if (g_hash_table_size(carrito) == 0) {
        mostrar_error("El carrito está vacío");
        return;
    }
    
    // Calcular el total
    float total = calcular_total_carrito();
    int cantidad_total = obtener_cantidad_total_carrito();
    
    // Procesar cada compra con el servidor
    gboolean todas_compras_exitosas = TRUE;
    GString *compras_fallidas = g_string_new("");
    
    GHashTableIter iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&iter, carrito);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CarritoItem *item = (CarritoItem *)value;
        
        int resultado = comprar_producto_servidor(item->id, item->cantidad);
        if (resultado != 1) {
            todas_compras_exitosas = FALSE;
            g_string_append_printf(compras_fallidas, "• %s (solicitado: %d)\n", 
                                 item->nombre, item->cantidad);
        }
    }
    
    // Mostrar el resultado
    if (todas_compras_exitosas) {
        char mensaje[200];
        g_snprintf(mensaje, sizeof(mensaje), 
                  "¡Compra realizada con éxito!\n\n"
                  "Total: $%.2f\n"
                  "Productos: %d\n\n"
                  "Gracias por su compra",
                  total, cantidad_total);
        
        mostrar_checkout(mensaje);
        
        // Limpiar carrito solo si todo fue exitoso
        g_hash_table_remove_all(carrito);
    } else {
        char mensaje[500];
        g_snprintf(mensaje, sizeof(mensaje),
                  "Algunos productos no pudieron ser comprados:\n\n"
                  "%s\n"
                  "Total procesado: €%.2f\n\n"
                  "Los productos disponibles se han procesado.",
                  compras_fallidas->str, total);
        
        mostrar_error(mensaje);
    }
    
    // Limpiar
    if (compras_fallidas->str) {
        g_string_free(compras_fallidas, TRUE);
    }
    
    // Actualizar UI y guardar
    actualizar_ui_carrito();
    gtk_widget_set_visible(GTK_WIDGET(cart_badge), FALSE);
    guardar_carrito_archivo();
}


void actualizar_ui_carrito() {
    // Obtener el contenedor del carrito (debes tenerlo definido en tu UI principal)


    GtkWidget *carrito_container = GTK_WIDGET(gtk_builder_get_object(builder, "carrito_products_list"));
    if (!carrito_container) {
        g_warning("No se encontró el contenedor del carrito");
        return;
    }
    
    // Limpiar el contenedor actual
    GtkWidget *childs = gtk_widget_get_first_child(carrito_container);
    while (childs != NULL)
    {
        gtk_box_remove(GTK_BOX(carrito_container), childs);
        childs = gtk_widget_get_first_child(carrito_container);
    }
    
    // Recorrer todos los items del carrito
    GHashTableIter hash_iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&hash_iter, carrito);
    while (g_hash_table_iter_next(&hash_iter, &key, &value)) {
        CarritoItem *item = (CarritoItem *)value;
        
        // Cargar el template del item del carrito
        GError *error = NULL;
        GtkBuilder *item_builder = gtk_builder_new();
        
        if (!gtk_builder_add_from_resource(item_builder, "/res/ui/itemCart.ui", &error)) {
            g_warning("Error cargando itemCart.ui: %s", error->message);
            g_error_free(error);
            g_object_unref(item_builder);
            continue;
        }
        
        // Obtener el widget principal del item
        GtkWidget *cart_item = GTK_WIDGET(gtk_builder_get_object(item_builder, "cart_item"));
        if (!cart_item) {
            g_warning("No se encontró el widget cart_item");
            g_object_unref(item_builder);
            continue;
        }
        
        // Configurar los datos del item
        configurar_item_carrito(cart_item, item_builder, item);
        
        // Añadir al contenedor
        gtk_box_append(GTK_BOX(carrito_container), cart_item);
        
        // Mostrar el widget
        gtk_widget_set_visible(cart_item, TRUE);
        
        // Liberar el builder (los widgets permanecen)
        g_object_unref(item_builder);
    }
    
    // Actualizar el total del carrito
    actualizar_total_carrito();
}


void abrir_carrito_cb (GtkWidget *widget, gpointer   user_data) {
    gboolean visible = gtk_revealer_get_reveal_child(carrito_revealer);
    gtk_revealer_set_reveal_child(carrito_revealer, !visible);
    gtk_widget_set_sensitive(GTK_WIDGET(carrito_revealer), !visible);
}


void cerrar_carrito_cb (GtkWidget *widget, gpointer   user_data) {
    gtk_revealer_set_reveal_child(carrito_revealer, FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(carrito_revealer), FALSE);
}

void vaciar_carrito_cb (GtkWidget *widget, gpointer   user_data) {
    g_print("Vaciando carrito...\n");
    g_hash_table_remove_all(carrito);
    actualizar_ui_carrito();
    actualizar_badge_carrito();
    guardar_carrito_archivo();
}

void cargar_carrito_archivo() {
    const char *filename = "carrito.dat";
    GError *error = NULL;
    
    // Verificar si el archivo existe
    if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
        g_print("No existe archivo de carrito previo\n");
        return;
    }
    
    // Leer el archivo
    gchar *contents;
    gsize length;
    
    if (!g_file_get_contents(filename, &contents, &length, &error)) {
        g_warning("Error leyendo archivo de carrito: %s", error->message);
        g_error_free(error);
        return;
    }
    
    // Parsear el contenido como JSON
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, contents, length, &error)) {
        g_warning("Error parseando JSON del carrito: %s", error->message);
        g_error_free(error);
        g_free(contents);
        g_object_unref(parser);
        return;
    }
    
    JsonNode *root = json_parser_get_root(parser);
    JsonArray *array = json_node_get_array(root);
    
    if (!array) {
        g_warning("Formato inválido del archivo de carrito");
        g_free(contents);
        g_object_unref(parser);
        return;
    }
    
    // Limpiar el carrito actual antes de cargar
    g_hash_table_remove_all(carrito);
    
    // Cargar cada item del carrito
    guint length_array = json_array_get_length(array);
    for (guint i = 0; i < length_array; i++) {
        JsonObject *obj = json_array_get_object_element(array, i);
        
        CarritoItem *item = g_new0(CarritoItem, 1);
        
        // Leer propiedades del JSON
        const gchar *id = json_object_get_string_member(obj, "id");
        const gchar *nombre = json_object_get_string_member(obj, "nombre");
        const gchar *descripcion = json_object_get_string_member(obj, "descripcion");
        const gchar *categoria = json_object_get_string_member(obj, "categoria");
        const gchar *subcategoria = json_object_get_string_member(obj, "subcategoria");
        const gchar *imagen_url = json_object_get_string_member(obj, "imagen_url");
        gdouble precio = json_object_get_double_member(obj, "precio");
        gint64 cantidad = json_object_get_int_member(obj, "cantidad");
        
        // Copiar datos a la estructura
        if (id) g_strlcpy(item->id, id, sizeof(item->id));
        if (nombre) g_strlcpy(item->nombre, nombre, sizeof(item->nombre));
        if (descripcion) g_strlcpy(item->descripcion, descripcion, sizeof(item->descripcion));
        if (categoria) g_strlcpy(item->categoria, categoria, sizeof(item->categoria));
        if (subcategoria) g_strlcpy(item->subcategoria, subcategoria, sizeof(item->subcategoria));
        if (imagen_url) g_strlcpy(item->imagen_url, imagen_url, sizeof(item->imagen_url));
        
        item->precio = (float)precio;
        item->cantidad = (int)cantidad;
        
        // Verificar stock actual antes de añadir
        if (verificar_stock_servidor(item->id, item->cantidad)) {
            g_hash_table_insert(carrito, g_strdup(item->id), item);
        } else {
            g_warning("Producto %s sin stock suficiente, no se añade al carrito", item->id);
            g_free(item);
        }
    }
    
    g_free(contents);
    g_object_unref(parser);
    
    g_print("Carrito cargado desde archivo: %d items\n", g_hash_table_size(carrito));
    
    // Actualizar la UI
    actualizar_ui_carrito();
}

void guardar_carrito_archivo() {
    const char *filename = "carrito.dat";
    GError *error = NULL;
    
    // Crear array JSON para los items
    JsonArray *array = json_array_new();
    
    // Recorrer todos los items del carrito
    GHashTableIter iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&iter, carrito);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CarritoItem *item = (CarritoItem *)value;
        
        // Crear objeto JSON para el item
        JsonObject *obj = json_object_new();
        
        json_object_set_string_member(obj, "id", item->id);
        json_object_set_string_member(obj, "nombre", item->nombre);
        json_object_set_string_member(obj, "descripcion", item->descripcion);
        json_object_set_string_member(obj, "categoria", item->categoria);
        json_object_set_string_member(obj, "subcategoria", item->subcategoria);
        json_object_set_string_member(obj, "imagen_url", item->imagen_url);
        json_object_set_double_member(obj, "precio", item->precio);
        json_object_set_int_member(obj, "cantidad", item->cantidad);
        
        json_array_add_object_element(array, obj);
    }
    
    // Crear el nodo raíz
    JsonNode *root = json_node_new(JSON_NODE_ARRAY);
    json_node_set_array(root, array);
    
    // Generar el JSON string
    JsonGenerator *generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);
    
    gchar *json_str = json_generator_to_data(generator, NULL);
    
    // Guardar en archivo
    if (!g_file_set_contents(filename, json_str, -1, &error)) {
        g_warning("Error guardando carrito: %s", error->message);
        g_error_free(error);
    } else {
        g_print("Carrito guardado en archivo: %d items\n", g_hash_table_size(carrito));
    }
    
    // Limpiar
    if (json_str) {
        g_free(json_str);
    }
    if (generator) {
        g_object_unref(generator);
    }
    if (root) {
        json_node_free(root);
    }
}

void finalizar_carrito() {
    // Guardar carrito antes de salir
    guardar_carrito_archivo();
    
    // Limpiar memoria (si es necesario)
    if (carrito) {
        g_hash_table_destroy(carrito);
        carrito = NULL;
    }
}