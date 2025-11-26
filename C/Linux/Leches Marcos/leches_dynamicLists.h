#ifndef LECHES_DYNAMICLISTS_H
#define LECHES_DYNAMICLISTS_H

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "producto.h"
#include <ctype.h>

typedef struct {
    const char *selected_category;
    GHashTable *selected_subcategories; // Hash table para múltiples subcategorías
    const char *search_query;  // texto de busqueda
    GtkCustomFilter *category_filter;
    GtkCustomFilter *subcategory_filter;
    GtkCustomFilter *search_filter;
} FilterState;

// Solo DECLARACIONES con extern
extern GListStore *store;
extern GtkNumericSorter *price_sorter;
extern GtkNumericSorter *stock_sorter;
extern FilterState filter_state;
extern GtkFilterListModel *category_filtered_model;
extern GtkFilterListModel *subcategory_filtered_model;
extern GtkSortListModel *sorted_model;
extern GtkSortListModel *stock_sorted_model; // para destacados
extern GtkSliceListModel *limited;
extern GtkNoSelection *selection;
extern GtkNoSelection *selectionstock;
extern GtkListView *list_view;
extern GtkGridView *grid_view;
extern GtkGridView *featured_view;
extern GtkListItemFactory *card_factory;
extern GtkListItemFactory *minicard_factory;

void cargar_productos(GObject *category_box, GCallback filtrar_categoria_cb);
gboolean subcat_filter_func(gpointer item, gpointer user_data);
gboolean category_filter_func(gpointer item, gpointer user_data);
gboolean search_filter_func(gpointer item, gpointer user_data);

void obtener_nombres_categoria_subcategoria(int categoria_id, int subcategoria_id, char *categoria_nombre, char *subcategoria_nombre);

void filter_state_init(FilterState *state);
void filter_state_clear(FilterState *state);
void filter_state_toggle_subcategory(FilterState *state, const char *subcategory);
gboolean filter_state_has_subcategory(FilterState *state, const char *subcategory);
guint filter_state_get_subcategory_count(FilterState *state);

#endif // LECHES_DYNAMICLISTS_H