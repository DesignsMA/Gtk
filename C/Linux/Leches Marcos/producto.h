#pragma once
#include <glib-object.h>

G_BEGIN_DECLS

#define PRODUCT_TYPE (product_get_type())

// Declare the final type
G_DECLARE_FINAL_TYPE(Product, product, PRODUCT, OBJECT, GObject)

// Define the PRODUCT macro for casting
#define PRODUCT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PRODUCT_TYPE, Product))

// Constructor
Product *product_new(
    const char *id,
    const char *nombre,
    const char *descripcion,
    float precio,
    int stock,
    const char *categoria,
    const char *subcategoria,
    const char *imagen_url
);

G_END_DECLS