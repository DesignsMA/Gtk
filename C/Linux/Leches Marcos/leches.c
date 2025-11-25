#include <gtk/gtk.h>
#include <glib/gstdio.h> // standard glib in out
#include "leches_socket.h"
#include "leches_dynamicLists.h"
#include <fontconfig/fontconfig.h>

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
void añadir_al_carrito_cb(GtkButton *button, gpointer user_data);
void card_button_clicked(GtkButton *btn, gpointer data);
static void fabricar_card_productos();

static void on_subcategory_toggled(GtkCheckButton *button, gpointer user_data);
void update_subcategory_filters(GtkWidget *filter_box, const char *category_name);
void add_subcategory_checkboxes(GtkWidget *filter_box, Categoria *categoria);
void cargar_filtros();



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

// Callback, se ejecuta en el hilo principal cuando termina la funcion del hilo
static void actualizar_modelo(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    // HILO PRINCIPAL: actualizar modelo existente
    /*DatosServidor *nuevos_datos = g_task_propagate_pointer(G_TASK(res), NULL);
    
    // Actualizar modelo SIN recargar UI completa
    g_list_store_remove_all(store);
    for (int i = 0; i < nuevos_datos->num_productos; i++) {
        Product *product = product_new(...);
        g_list_store_append(store, product);
        g_object_unref(product);
    }
        */
    
}

static void actualizar_productos() {
    g_print("Actualizando productos...\n");
    // Aquí iría la lógica para actualizar los productos desde el servidor
    // Por ahora, solo imprimimos un mensaje
    /*
    GTask *task = g_task_new(NULL, NULL, actualizar_modelo, NULL);
    g_task_run_in_thread(task, (GTaskThreadFunc)descargar_datos);
    */
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

    cargar_productos(G_OBJECT(category_box), G_CALLBACK(filtrar_categoria_cb)); // Cargar productos en la tienda

    fabricar_card_productos(builder); // Configurar fábrica de cards para productos
    
    cargar_filtros(); // Cargar filtros de categorías y subcategorías

    gtk_widget_set_visible (GTK_WIDGET (window), TRUE);

    // Cada 30 segundos actualizar productos
    g_timeout_add_seconds(30, (GSourceFunc)actualizar_productos, NULL);
}

/*#########################################################################################################*/

int
main (int   argc,
      char *argv[])
{
#ifdef GTK_SRCDIR
  g_chdir (GTK_SRCDIR);
#endif
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
    if (img_url) cargar_imagen(img_url, widgets->image);
    
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
    if (img_url) cargar_imagen(img_url, widgets->image);
    
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

void añadir_al_carrito_cb(GtkButton *button, gpointer user_data) {
    GObject *product = G_OBJECT(g_object_get_data(G_OBJECT(button), "product")); // Obtener el producto asociado al botón
    if (product) {
        const char *nombre = NULL;
        g_object_get(product, "nombre", &nombre, NULL);
        g_print("Añadiendo al carrito: %s\n", nombre);
        // Aquí puedes agregar la lógica para añadir el producto al carrito
        g_free((gpointer)nombre);
    }
    
}