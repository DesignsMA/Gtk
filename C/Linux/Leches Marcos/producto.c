#include "producto.h"

struct _Product
{
  GObject parent_instance;

  char *id;
  char *nombre;
  char *descripcion;
  float precio;
  int stock;
  char *categoria;
  char *subcategoria;
  char *imagen_url;
};

// Boilerplate GObject code
G_DEFINE_TYPE(Product, product, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_ID,
    PROP_NOMBRE,
    PROP_DESCRIPCION,
    PROP_PRECIO,
    PROP_STOCK,
    PROP_CATEGORIA,
    PROP_SUBCATEGORIA,
    PROP_IMAGEN_URL,
    N_PROPS
};

static GParamSpec *obj_properties[N_PROPS]; // arreglo de propiedades, atributos del objeto

static void
product_set_property(GObject *object, guint prop_id,
                     const GValue *value, GParamSpec *pspec)
{
    Product *self = PRODUCT(object);

    switch (prop_id) {
    case PROP_ID:
        self->id = g_value_dup_string(value);
        break;
    case PROP_NOMBRE:
        self->nombre = g_value_dup_string(value);
        break;
    case PROP_DESCRIPCION:
        self->descripcion = g_value_dup_string(value);
        break;
    case PROP_PRECIO:
        self->precio = g_value_get_float(value);
        break;
    case PROP_STOCK:
        self->stock = g_value_get_int(value);
        break;
    case PROP_CATEGORIA:
        self->categoria = g_value_dup_string(value);
        break;
    case PROP_SUBCATEGORIA:
        self->subcategoria = g_value_dup_string(value);
        break;
    case PROP_IMAGEN_URL:
        self->imagen_url = g_value_dup_string(value);
        break;
    }
}

static void
product_get_property(GObject *object, guint prop_id,
                     GValue *value, GParamSpec *pspec)
{
    Product *self = PRODUCT(object);

    switch (prop_id) {
    case PROP_ID:
        g_value_set_string(value, self->id);
        break;
    case PROP_NOMBRE:
        g_value_set_string(value, self->nombre);
        break;
    case PROP_DESCRIPCION:
        g_value_set_string(value, self->descripcion);
        break;
    case PROP_PRECIO:
        g_value_set_float(value, self->precio);
        break;
    case PROP_STOCK:
        g_value_set_int(value, self->stock);
        break;
    case PROP_CATEGORIA:
        g_value_set_string(value, self->categoria);
        break;
    case PROP_SUBCATEGORIA:
        g_value_set_string(value, self->subcategoria);
        break;
    case PROP_IMAGEN_URL:
        g_value_set_string(value, self->imagen_url);
        break;
    }
}

static void
product_finalize(GObject *object)
{
    Product *self = PRODUCT(object);

    g_free(self->nombre);
    g_free(self->id);
    g_free(self->descripcion);
    g_free(self->categoria);
    g_free(self->subcategoria);
    g_free(self->imagen_url);

    G_OBJECT_CLASS(product_parent_class)->finalize(object);
}

static void
product_class_init(ProductClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = product_set_property;
    object_class->get_property = product_get_property;
    object_class->finalize = product_finalize;

    obj_properties[PROP_ID] =
        g_param_spec_string("id", "ID", "UUID del producto",
                         NULL, G_PARAM_READWRITE);

    obj_properties[PROP_NOMBRE] =
        g_param_spec_string("nombre", "Nombre", "Nombre del producto",
                            NULL, G_PARAM_READWRITE);

    obj_properties[PROP_DESCRIPCION] =
        g_param_spec_string("descripcion", "Descripcion", "Descripcion",
                            NULL, G_PARAM_READWRITE);

    obj_properties[PROP_PRECIO] =
        g_param_spec_float("precio", "Precio", "Precio",
                           0, 100000, 0, G_PARAM_READWRITE);

    obj_properties[PROP_STOCK] =
        g_param_spec_int("stock", "Stock", "Stock",
                         0, G_MAXINT, 0, G_PARAM_READWRITE);

    obj_properties[PROP_CATEGORIA] =
        g_param_spec_string("categoria", "Categoria", "Categoria",
                            NULL, G_PARAM_READWRITE);

    obj_properties[PROP_SUBCATEGORIA] =
        g_param_spec_string("subcategoria", "Subcategoria", "Subcategoria",
                            NULL, G_PARAM_READWRITE);

    obj_properties[PROP_IMAGEN_URL] =
        g_param_spec_string("imagen-url", "Imagen URL", "Imagen",
                            NULL, G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPS, obj_properties);
}

static void product_init(Product *self) {}

Product *product_new(
    const char *id, const char *nombre, const char *descripcion,
    float precio, int stock, const char *categoria,
    const char *subcategoria, const char *imagen_url)
{
    return g_object_new(PRODUCT_TYPE,
                        "id", id,
                        "nombre", nombre,
                        "descripcion", descripcion,
                        "precio", precio,
                        "stock", stock,
                        "categoria", categoria,
                        "subcategoria", subcategoria,
                        "imagen-url", imagen_url,
                        NULL);
}