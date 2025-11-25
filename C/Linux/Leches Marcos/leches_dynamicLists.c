#include "leches_dynamicLists.h"
// ejemplo

// ARREGLO DE CATEGORÍAS (5 categorías)
Categoria categorias[5] = {
    // CATEGORÍA 1: leches
    {
        .nombre = "leches",
        .subcategorias = {
            "entera",
            "deslactosada"
        },
        .num_subcategorias = 2
    },
    
    // CATEGORÍA 2: quesos
    {
        .nombre = "quesos", 
        .subcategorias = {
            "fresco",
            "mozzarella"
        },
        .num_subcategorias = 2
    },
    
    // CATEGORÍA 3: yogures
    {
        .nombre = "yogures",
        .subcategorias = {
            "natural", 
            "griego"
        },
        .num_subcategorias = 2
    },
    
    // CATEGORÍA 4: cremas
    {
        .nombre = "cremas",
        .subcategorias = {
            "para batir"
        },
        .num_subcategorias = 1
    },

    // CATEGORÍA 5: test
    {
        .nombre = "test",
        .subcategorias = {
            "test"
        },
        .num_subcategorias = 1
    }
};

// ARREGLO DE PRODUCTOS (8 productos)
Producto productos[8] = {

    // PRODUCTOS DE CATEGORÍA "leches"
    {
        .id = "10",
        .nombre = "Lala Entera",
        .descripcion = "Leche entera fresca de vaca",
        .precio = 25.50,
        .stock = 100,
        .categoria = "leches",
        .subcategoria = "entera",
        .imagen_url = "https://imgur.com/bBuqTi9.png"
    },
    {
        .id = "11",
        .nombre = "Leche Deslactosada", 
        .descripcion = "Leche sin lactosa",
        .precio = 28.00,
        .stock = 80,
        .categoria = "leches",
        .subcategoria = "deslactosada",
        .imagen_url = ""
    },
    
    // PRODUCTOS DE CATEGORÍA "quesos"
    {
        .id = "12",
        .nombre = "Queso Fresco",
        .descripcion = "Queso fresco natural",
        .precio = 45.00,
        .stock = 50,
        .categoria = "quesos",
        .subcategoria = "fresco", 
        .imagen_url = ""
    },
    {
        .id = "13",
        .nombre = "Queso Mozzarella",
        .descripcion = "Queso mozzarella para pizza",
        .precio = 60.00,
        .stock = 30,
        .categoria = "quesos",
        .subcategoria = "mozzarella",
        .imagen_url = ""
    },
    
    // PRODUCTOS DE CATEGORÍA "yogures"
    {
        .id = "15",
        .nombre = "Yogur Natural",
        .descripcion = "Yogur natural sin azúcar",
        .precio = 18.75,
        .stock = 80,
        .categoria = "yogures",
        .subcategoria = "natural",
        .imagen_url = ""
    },
    {
        .id = "16",
        .nombre = "Yogur Griego",
        .descripcion = "Yogur estilo griego cremoso",
        .precio = 22.50,
        .stock = 60,
        .categoria = "yogures",
        .subcategoria = "griego",
        .imagen_url = ""
    },
    
    // PRODUCTOS DE CATEGORÍA "cremas"
    {
        .id = "17",
        .nombre = "Crema para Batir",
        .descripcion = "Crema para batir 35% grasa",
        .precio = 35.00,
        .stock = 40,
        .categoria = "cremas",
        .subcategoria = "para batir",
        .imagen_url = ""
    },

    {
        .id = "18",
        .nombre = "Test",
        .descripcion = "Prueba de funcionamiento",
        .precio = 777.00,
        .stock = 70,
        .categoria = "test",
        .subcategoria = "test",
        .imagen_url = "https://imgur.com/bc9Zj0Y.png"
    }
};

GListStore *store = NULL;
GtkNumericSorter *price_sorter = NULL;
GtkNumericSorter *stock_sorter = NULL;
FilterState filter_state = {0};
GtkFilterListModel *category_filtered_model = NULL;
GtkFilterListModel *subcategory_filtered_model = NULL;
GtkSortListModel *sorted_model = NULL;
GtkSortListModel *stock_sorted_model = NULL;
GtkSliceListModel *limited = NULL;
GtkNoSelection *selection = NULL;
GtkNoSelection *selectionstock = NULL;
GtkListView *list_view = NULL;
GtkGridView *grid_view = NULL;
GtkGridView *featured_view = NULL;
GtkListItemFactory *card_factory = NULL;
GtkListItemFactory *minicard_factory = NULL;

size_t num_categorias = sizeof(categorias)/sizeof(categorias[0]);
size_t num_productos = sizeof(productos)/sizeof(productos[0]);

void cargar_productos(GObject *category_box, GCallback filtrar_categoria_cb) {
    // Esta función puede ser utilizada para cargar productos desde una base de datos o archivo
    // Actualmente, los productos están definidos estáticamente en el arreglo 'productos'
    filter_state_init(&filter_state); // Inicializar el estado del filtro

    store = g_list_store_new(PRODUCT_TYPE); // Nuevo modelo de productos

    for (size_t i = 0; i < num_categorias; i++) // Cargar categorías en la interfaz
    {
        char *upper = g_utf8_strup(categorias[i].nombre, -1);
        GtkWidget *button = gtk_button_new_with_label(upper); // Crear botón con etiqueta de categoría

        g_object_set_data(G_OBJECT(button), "categoria", (gpointer)categorias[i].nombre); // Asociar categoría al botón

        g_signal_connect(button, "clicked", filtrar_categoria_cb, NULL); // Conectar señal de clic al botón

        gtk_widget_add_css_class(button, "category-button"); // Añadir clase CSS para estilos

        gtk_box_append(GTK_BOX(category_box), button); // Añadir botón a la caja de categorías

        gtk_widget_set_visible(button, TRUE); // mostrar boton
    }
    

    for (int i = 0; i < num_productos; i++) {
        Product *product = product_new(
            productos[i].id,
            productos[i].nombre,
            productos[i].descripcion,
            productos[i].precio,
            productos[i].stock,
            productos[i].categoria,
            productos[i].subcategoria,
            productos[i].imagen_url
        );
        g_list_store_append(store, G_OBJECT(product)); // agregar producto al modelo
        g_object_unref(product);
    }

    // Nuevo ordenador por precio, por defecto ascendente
    price_sorter = gtk_numeric_sorter_new(
        // Indicamos la propiedad "precio" del objeto Producto como expresión a ordenar
        gtk_property_expression_new(PRODUCT_TYPE, NULL, "precio")
    );

    // Productos destacados (es decir los que no se vendieron bien y queremos que tengan más visibilidad)
    stock_sorter = gtk_numeric_sorter_new(
        gtk_property_expression_new(PRODUCT_TYPE, NULL, "stock")
    );
    
    // No tiene intervención en el pipeline, solo es para los destacados
    stock_sorted_model = gtk_sort_list_model_new(
        G_LIST_MODEL(store), // modelo base
        GTK_SORTER(stock_sorter) // ordenador por precio
    );

    gtk_sort_list_model_set_incremental( // Habilitar ordenamiento incremental
        stock_sorted_model,
        TRUE
    );

    gtk_numeric_sorter_set_sort_order(stock_sorter, GTK_SORT_DESCENDING); // Ordenar de mayor a menor stock

    limited = gtk_slice_list_model_new(
        G_LIST_MODEL(stock_sorted_model),  // modelo de entrada
        0,                      // offset = desde el primer item
        4                       // size = máximo 4 items
    );
    
    selectionstock = gtk_no_selection_new(
        G_LIST_MODEL(limited)
    );

    filter_state.category_filter = 
        gtk_custom_filter_new(
            category_filter_func, // función de filtrado por categoría
            &filter_state,  // datos de usuario (estado del filtro)
            NULL);

    filter_state.subcategory_filter = gtk_custom_filter_new(
        subcat_filter_func, // función de filtrado por subcategoría
        &filter_state,  // datos de usuario (estado del filtro)
        NULL);

    // Modelo filtrado por categoría, filtra los elementos de store según el filtro de categoría
    category_filtered_model = gtk_filter_list_model_new(
        G_LIST_MODEL(store), // modelo base, contenedro de productos
        GTK_FILTER(filter_state.category_filter) // filtro de categoría
    );

    gtk_filter_list_model_set_incremental( // Habilitar filtrado incremental
        category_filtered_model,
        TRUE
    );

    // Modelo filtrado por subcategoría, filtra los elementos del modelo filtrado por categoría según el filtro de subcategoría
    subcategory_filtered_model = gtk_filter_list_model_new(
        G_LIST_MODEL(category_filtered_model), // modelo base, modelo filtrado por categoría
        GTK_FILTER(filter_state.subcategory_filter) // filtro de subcategoría
    );

    gtk_filter_list_model_set_incremental( // Habilitar filtrado incremental
        subcategory_filtered_model,
        TRUE
    );

    // Modelo ordenado, ordena los elementos del modelo filtrado por subcategoría según el ordenador de precio
    sorted_model = gtk_sort_list_model_new(
        G_LIST_MODEL(subcategory_filtered_model), // modelo base, modelo filtrado por subcategoría
        GTK_SORTER(price_sorter) // ordenador por precio
    );

    gtk_sort_list_model_set_incremental( // Habilitar ordenamiento incremental
        sorted_model,
        TRUE
    );

    filter_state.search_filter = gtk_custom_filter_new(
        search_filter_func,
        &filter_state,
        NULL
    );

    GtkFilterListModel *search_filtered_model = gtk_filter_list_model_new(
        G_LIST_MODEL(sorted_model),  // Al final del pipeline
        GTK_FILTER(filter_state.search_filter)
    );
    
    gtk_filter_list_model_set_incremental(search_filtered_model, TRUE);

    selection = gtk_no_selection_new(G_LIST_MODEL(search_filtered_model)); // Modelo de selección sin selección, envuelve el modelo ordenado
        
}

// Filtrado por subcategoría
// Filtrado por subcategoría (ahora múltiple)
gboolean subcat_filter_func(gpointer item, gpointer user_data) {
    FilterState *state = (FilterState *)user_data;
    Product *product = (Product *)item;

    // Si no hay subcategorías seleccionadas, permitir todos los productos
    if (filter_state_get_subcategory_count(state) == 0) {
        return TRUE;
    }

    const char *product_subcat;
    g_object_get(G_OBJECT(product), "subcategoria", &product_subcat, NULL);
    
    // Verificar si la subcategoría del producto está en las seleccionadas
    gboolean ok = filter_state_has_subcategory(state, product_subcat);
    
    g_free((gpointer)product_subcat);
    return ok;
}

// Filtrado por categoría
gboolean category_filter_func(gpointer item, gpointer user_data) {
    FilterState *state = (FilterState *)user_data;
    Product *product = (Product *)item; 
    // Si no hay categoría seleccionada, permitir todos los productos
    if (state->selected_category == NULL || g_strcmp0(state->selected_category, "") == 0) {
        return TRUE;
    }
    const char *cat;
    g_object_get(G_OBJECT(product), "categoria", &cat, NULL);
    // Filtrar por categoría
    gboolean ok = (g_strcmp0(cat, state->selected_category) == 0
    );
    g_free((gpointer)cat);
    return ok;
}

// Función auxiliar para obtener los nombres
void obtener_nombres_categoria_subcategoria(int categoria_id, int subcategoria_id, 
                                           char *categoria_nombre, char *subcategoria_nombre) {
    if (categoria_id >= 0 && categoria_id < MAX_CATEGORIAS && 
        categorias[categoria_id].nombre[0] != '\0') {
        
        strcpy(categoria_nombre, categorias[categoria_id].nombre);
        
        if (subcategoria_id >= 0 && subcategoria_id < categorias[categoria_id].num_subcategorias) {
            strcpy(subcategoria_nombre, categorias[categoria_id].subcategorias[subcategoria_id]);
        } else {
            strcpy(subcategoria_nombre, "Sin subcategoría");
        }
    } else {
        strcpy(categoria_nombre, "Sin categoría");
        strcpy(subcategoria_nombre, "Sin subcategoría");
    }
}

gboolean fuzzy_match(const char *text, const char *pattern) {
    if (pattern == NULL || pattern[0] == '\0') {
        return TRUE; // Si no hay patrón, mostrar todo
    }
    
    // Convertir ambos a minúsculas para búsqueda case-insensitive
    char *text_lower = g_ascii_strdown(text, -1);
    char *pattern_lower = g_ascii_strdown(pattern, -1);
    
    const char *text_ptr = text_lower;
    const char *pattern_ptr = pattern_lower;
    
    // Algoritmo: cada carácter del patrón debe aparecer en orden en el texto
    while (*pattern_ptr != '\0') {
        // Buscar el carácter actual del patrón en el texto
        text_ptr = strchr(text_ptr, *pattern_ptr);
        if (text_ptr == NULL) {
            g_free(text_lower);
            g_free(pattern_lower);
            return FALSE; // No se encontró el carácter
        }
        text_ptr++; // Avanzar al siguiente carácter
        pattern_ptr++;
    }
    
    g_free(text_lower);
    g_free(pattern_lower);
    return TRUE; // Todos los caracteres del patrón fueron encontrados en orden
}

gboolean search_filter_func(gpointer item, gpointer user_data) {
    FilterState *state = (FilterState *)user_data;
    Product *product = (Product *)item;
    
    // Si no hay texto de búsqueda, mostrar todos los productos
    if (state->search_query == NULL || state->search_query[0] == '\0') {
        return TRUE;
    }
    
    // Obtener propiedades del producto para buscar
    const char *nombre = NULL;
    const char *descripcion = NULL;
    const char *categoria = NULL;
    const char *subcategoria = NULL;
    
    g_object_get(G_OBJECT(product),
                 "nombre", &nombre,
                 "descripcion", &descripcion,
                 "categoria", &categoria,
                 "subcategoria", &subcategoria,
                 NULL);
    
    // Buscar en nombre, descripción, categoría y subcategoría
    gboolean match = FALSE;
    
    if (nombre && fuzzy_match(nombre, state->search_query)) {
        match = TRUE;
    } else if (descripcion && fuzzy_match(descripcion, state->search_query)) {
        match = TRUE;
    } else if (categoria && fuzzy_match(categoria, state->search_query)) {
        match = TRUE;
    } else if (subcategoria && fuzzy_match(subcategoria, state->search_query)) {
        match = TRUE;
    }
    
    // Liberar strings
    g_free((gpointer)nombre);
    g_free((gpointer)descripcion);
    g_free((gpointer)categoria);
    g_free((gpointer)subcategoria);
    
    return match;
}

// FILTRADO

// Función para inicializar el FilterState
void filter_state_init(FilterState *state) {
    state->selected_category = NULL;
    state->selected_subcategories = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    state->category_filter = NULL;
    state->subcategory_filter = NULL;
    filter_state.search_query = NULL;
}

// Función para limpiar el FilterState
void filter_state_clear(FilterState *state) {
    if (state->selected_subcategories) {
        g_hash_table_remove_all(state->selected_subcategories);
    }
}

// Función para agregar/remover subcategoría
void filter_state_toggle_subcategory(FilterState *state, const char *subcategory) {
    if (g_hash_table_contains(state->selected_subcategories, subcategory)) {
        g_hash_table_remove(state->selected_subcategories, subcategory);
    } else {
        g_hash_table_add(state->selected_subcategories, g_strdup(subcategory));
    }
}

// Función para verificar si una subcategoría está seleccionada
gboolean filter_state_has_subcategory(FilterState *state, const char *subcategory) {
    return g_hash_table_contains(state->selected_subcategories, subcategory);
}

// Función para obtener el número de subcategorías seleccionadas
guint filter_state_get_subcategory_count(FilterState *state) {
    return g_hash_table_size(state->selected_subcategories);
}