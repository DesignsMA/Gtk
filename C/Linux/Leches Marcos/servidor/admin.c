#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <uuid/uuid.h>
#include <ctype.h>

// Estructuras
#define MAX_NAME 100
#define MAX_DESC 200
#define MAX_URL 300
#define MAX_CAT_NAME 50
#define MAX_SUBCAT_NAME 50

typedef struct {
    char nombre[MAX_CAT_NAME];
    GArray *subcategorias;  // Array dinámico de strings
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

void limpiar_categorias_si_vacias(const char *categoria, const char *subcategoria);

// Variables globales con GLib
GArray *categorias = NULL;  // Array de Categoria
GArray *productos = NULL;   // Array de Producto

// ==================== FUNCIONES AUXILIARES ====================

void a_minusculas(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void generar_id(char *buffer) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, buffer);
}

void limpiarPantalla() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ==================== GESTIÓN DE CATEGORÍAS ====================

Categoria* buscar_categoria(const char *nombre) {
    for (int i = 0; i < categorias->len; i++) {
        Categoria *cat = &g_array_index(categorias, Categoria, i);
        if (strcmp(cat->nombre, nombre) == 0) {
            return cat;
        }
    }
    return NULL;
}

gboolean subcategoria_existe(Categoria *cat, const char *subcat) {
    for (int i = 0; i < cat->subcategorias->len; i++) {
        char *subcat_existente = g_array_index(cat->subcategorias, char*, i);
        if (strcmp(subcat_existente, subcat) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

void agregar_subcategoria(Categoria *cat, const char *subcat) {
    if (!subcategoria_existe(cat, subcat)) {
        char *nueva_subcat = g_strdup(subcat);  // Duplicar string
        g_array_append_val(cat->subcategorias, nueva_subcat);
    }
}

void procesar_categorias_automatico(const char *categoria, const char *subcategoria) {
    // Buscar categoría existente
    Categoria *cat_existente = buscar_categoria(categoria);
    
    if (cat_existente) {
        // Categoría existe, agregar subcategoría si no existe
        agregar_subcategoria(cat_existente, subcategoria);
    } else {
        // Crear nueva categoría
        Categoria nueva_cat;
        strcpy(nueva_cat.nombre, categoria);
        nueva_cat.subcategorias = g_array_new(FALSE, FALSE, sizeof(char*));
        
        // Agregar primera subcategoría
        agregar_subcategoria(&nueva_cat, subcategoria);
        
        // Agregar al array global
        g_array_append_val(categorias, nueva_cat);
    }
}

// ==================== GESTIÓN DE PRODUCTOS ====================

Producto* buscar_producto_por_id(const char *id) {
    for (int i = 0; i < productos->len; i++) {
        Producto *prod = &g_array_index(productos, Producto, i);
        if (strcmp(prod->id, id) == 0) {
            return prod;
        }
    }
    return NULL;
}

Producto* buscar_producto_por_nombre(const char *nombre) {
    for (int i = 0; i < productos->len; i++) {
        Producto *prod = &g_array_index(productos, Producto, i);
        if (strcmp(prod->nombre, nombre) == 0) {
            return prod;
        }
    }
    return NULL;
}

void agregar_producto(const char *nombre, const char *descripcion, float precio, 
                     int stock, const char *categoria, const char *subcategoria, 
                     const char *imagen_url) {
    
    // Verificar si ya existe producto con mismo nombre
    if (buscar_producto_por_nombre(nombre) != NULL) {
        printf("Ya existe un producto con ese nombre: %s\n", nombre);
        return;
    }
    
    // Crear nuevo producto
    Producto nuevo;
    generar_id(nuevo.id);
    
    // Copiar datos (convertir a minúsculas)
    strcpy(nuevo.nombre, nombre);
    a_minusculas(nuevo.nombre);
    
    strcpy(nuevo.descripcion, descripcion);
    a_minusculas(nuevo.descripcion);
    
    nuevo.precio = precio;
    nuevo.stock = stock;
    
    strcpy(nuevo.categoria, categoria);
    a_minusculas(nuevo.categoria);
    
    strcpy(nuevo.subcategoria, subcategoria);
    a_minusculas(nuevo.subcategoria);
    
    strcpy(nuevo.imagen_url, imagen_url);
    
    // Procesar categorías automáticamente
    procesar_categorias_automatico(nuevo.categoria, nuevo.subcategoria);
    
    // Agregar producto
    g_array_append_val(productos, nuevo);
    
    printf("Producto agregado: %s (ID: %s)\n", nuevo.nombre, nuevo.id);
}

void eliminar_producto(const char *id) {
    Producto *prod = buscar_producto_por_id(id);
    if (!prod) {
        printf("Producto no encontrado: %s\n", id);
        return;
    }
    
    // Guardar categoría/subcategoría para limpieza
    char cat_temp[MAX_CAT_NAME], subcat_temp[MAX_SUBCAT_NAME];
    strcpy(cat_temp, prod->categoria);
    strcpy(subcat_temp, prod->subcategoria);
    
    // Eliminar producto
    for (int i = 0; i < productos->len; i++) {
        Producto *p = &g_array_index(productos, Producto, i);
        if (strcmp(p->id, id) == 0) {
            g_array_remove_index(productos, i);
            printf("Producto eliminado: %s\n", p->nombre);
            break;
        }
    }
    
    // Limpiar categorías/subcategorías si es necesario
    limpiar_categorias_si_vacias(cat_temp, subcat_temp);
}

void modificar_producto(const char *id, const char *nuevo_nombre, const char *nueva_descripcion, 
                       float nuevo_precio, int nuevo_stock, const char *nueva_imagen_url) {
    
    Producto *prod = buscar_producto_por_id(id);
    if (!prod) {
        printf("Producto no encontrado: %s\n", id);
        return;
    }
    
    // Verificar nuevo nombre (si cambió)
    if (strcmp(nuevo_nombre, prod->nombre) != 0) {
        if (buscar_producto_por_nombre(nuevo_nombre) != NULL) {
            printf("Ya existe un producto con ese nombre: %s\n", nuevo_nombre);
            return;
        }
    }
    
    // Actualizar datos
    strcpy(prod->nombre, nuevo_nombre);
    a_minusculas(prod->nombre);
    
    strcpy(prod->descripcion, nueva_descripcion);
    a_minusculas(prod->descripcion);
    
    prod->precio = nuevo_precio;
    prod->stock = nuevo_stock;
    
    strcpy(prod->imagen_url, nueva_imagen_url);
    
    printf("Producto modificado: %s\n", prod->nombre);
}

// ==================== LIMPIEZA AUTOMÁTICA ====================

int contar_productos_por_categoria(const char *categoria) {
    int count = 0;
    for (int i = 0; i < productos->len; i++) {
        Producto *prod = &g_array_index(productos, Producto, i);
        if (strcmp(prod->categoria, categoria) == 0) {
            count++;
        }
    }
    return count;
}

int contar_productos_por_subcategoria(const char *categoria, const char *subcategoria) {
    int count = 0;
    for (int i = 0; i < productos->len; i++) {
        Producto *prod = &g_array_index(productos, Producto, i);
        if (strcmp(prod->categoria, categoria) == 0 && 
            strcmp(prod->subcategoria, subcategoria) == 0) {
            count++;
        }
    }
    return count;
}

void limpiar_categorias_si_vacias(const char *categoria, const char *subcategoria) {
    Categoria *cat = buscar_categoria(categoria);
    if (!cat) return;
    
    // Verificar subcategoría
    if (contar_productos_por_subcategoria(categoria, subcategoria) == 0) {
        // Eliminar subcategoría
        for (int i = 0; i < cat->subcategorias->len; i++) {
            char *subcat = g_array_index(cat->subcategorias, char*, i);
            if (strcmp(subcat, subcategoria) == 0) {
                g_free(subcat);  // Liberar memoria
                g_array_remove_index(cat->subcategorias, i);
                printf("Subcategoría eliminada: %s/%s\n", categoria, subcategoria);
                break;
            }
        }
    }
    
    // Verificar categoría completa
    if (contar_productos_por_categoria(categoria) == 0) {
        // Eliminar categoría
        for (int i = 0; i < categorias->len; i++) {
            Categoria *c = &g_array_index(categorias, Categoria, i);
            if (strcmp(c->nombre, categoria) == 0) {
                // Liberar todas las subcategorías
                for (int j = 0; j < c->subcategorias->len; j++) {
                    char *subcat = g_array_index(c->subcategorias, char*, j);
                    g_free(subcat);
                }
                g_array_free(c->subcategorias, TRUE);
                g_array_remove_index(categorias, i);
                printf("Categoría eliminada: %s\n", categoria);
                break;
            }
        }
    }
}

// ==================== PERSISTENCIA ====================

void guardar_datos() {
    // Guardar productos
    FILE *f_prod = fopen("productos.bin", "wb");
    if (f_prod) {
        int num_prod = productos->len;
        fwrite(&num_prod, sizeof(int), 1, f_prod);
        fwrite(productos->data, sizeof(Producto), num_prod, f_prod);
        fclose(f_prod);
    }
    
    // Guardar categorías (más complejo por los arrays dinámicos)
    FILE *f_cat = fopen("categorias.bin", "wb");
    if (f_cat) {
        int num_cat = categorias->len;
        fwrite(&num_cat, sizeof(int), 1, f_cat);
        
        for (int i = 0; i < num_cat; i++) {
            Categoria *cat = &g_array_index(categorias, Categoria, i);
            
            // Guardar categoría base
            fwrite(cat, sizeof(Categoria) - sizeof(GArray*), 1, f_cat);
            
            // Guardar número de subcategorías
            int num_sub = cat->subcategorias->len;
            fwrite(&num_sub, sizeof(int), 1, f_cat);
            
            // Guardar cada subcategoría
            for (int j = 0; j < num_sub; j++) {
                char *subcat = g_array_index(cat->subcategorias, char*, j);
                fwrite(subcat, MAX_SUBCAT_NAME, 1, f_cat);
            }
        }
        fclose(f_cat);
    }
    
    printf("Datos guardados\n");
}

void cargar_datos() {
    // Inicializar arrays GLib si no existen
    if (!productos) {
        productos = g_array_new(FALSE, FALSE, sizeof(Producto));
    }
    if (!categorias) {
        categorias = g_array_new(FALSE, FALSE, sizeof(Categoria));
    }
    
    // Cargar productos
    FILE *f_prod = fopen("productos.bin", "rb");
    if (f_prod) {
        int num_prod;
        fread(&num_prod, sizeof(int), 1, f_prod);
        g_array_set_size(productos, num_prod);
        fread(productos->data, sizeof(Producto), num_prod, f_prod);
        fclose(f_prod);
    }
    
    // Cargar categorías
    FILE *f_cat = fopen("categorias.bin", "rb");
    if (f_cat) {
        int num_cat;
        fread(&num_cat, sizeof(int), 1, f_cat);
        
        for (int i = 0; i < num_cat; i++) {
            Categoria cat;
            cat.subcategorias = g_array_new(FALSE, FALSE, sizeof(char*));
            
            // Cargar categoría base
            fread(&cat, sizeof(Categoria) - sizeof(GArray*), 1, f_cat);
            
            // Cargar subcategorías
            int num_sub;
            fread(&num_sub, sizeof(int), 1, f_cat);
            
            for (int j = 0; j < num_sub; j++) {
                char subcat[MAX_SUBCAT_NAME];
                fread(subcat, MAX_SUBCAT_NAME, 1, f_cat);
                
                char *subcat_ptr = g_strdup(subcat);
                g_array_append_val(cat.subcategorias, subcat_ptr);
            }
            
            g_array_append_val(categorias, cat);
        }
        fclose(f_cat);
    }
}

// ==================== INTERFAZ DE USUARIO ====================

void introducir_producto() {
    char nombre[MAX_NAME], descripcion[MAX_DESC];
    float precio;
    int stock;
    char categoria[MAX_CAT_NAME], subcategoria[MAX_SUBCAT_NAME], url[MAX_URL];
    
    printf("Nombre: ");
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0;
    
    printf("Descripción: ");
    fgets(descripcion, sizeof(descripcion), stdin);
    descripcion[strcspn(descripcion, "\n")] = 0;
    
    printf("Precio: ");
    scanf("%f", &precio);
    getchar();
    
    printf("Stock: ");
    scanf("%d", &stock);
    getchar();
    
    printf("Categoría: ");
    fgets(categoria, sizeof(categoria), stdin);
    categoria[strcspn(categoria, "\n")] = 0;
    
    printf("Subcategoría: ");
    fgets(subcategoria, sizeof(subcategoria), stdin);
    subcategoria[strcspn(subcategoria, "\n")] = 0;
    
    printf("URL imagen: ");
    fgets(url, sizeof(url), stdin);
    url[strcspn(url, "\n")] = 0;
    
    agregar_producto(nombre, descripcion, precio, stock, categoria, subcategoria, url);
}

void modificar_producto_ui() {
    char id[37], nombre[MAX_NAME], descripcion[MAX_DESC], url[MAX_URL];
    float precio;
    int stock;
    
    printf("ID del producto a modificar: ");
    if (fgets(id, sizeof(id), stdin) == NULL) return;
    id[strcspn(id, "\n")] = 0;
    
    // Verificar que el producto existe
    Producto *prod = buscar_producto_por_id(id);
    if (!prod) {
        printf("Producto no encontrado con ID: %s\n", id);
        return;
    }

    // Limpiar el buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    printf("Producto actual: %s\n", prod->nombre);
    
    printf("Nuevo nombre [Enter para mantener '%s']: ", prod->nombre);
    if (fgets(nombre, sizeof(nombre), stdin) == NULL) return;
    nombre[strcspn(nombre, "\n")] = 0;
    if (strlen(nombre) == 0) {
        strcpy(nombre, prod->nombre);  // Mantener el actual
    }
    
    printf("Nueva descripción [Enter para mantener '%s']: ", prod->descripcion);
    if (fgets(descripcion, sizeof(descripcion), stdin) == NULL) return;
    descripcion[strcspn(descripcion, "\n")] = 0;
    if (strlen(descripcion) == 0) {
        strcpy(descripcion, prod->descripcion);
    }
    
    printf("Nuevo precio [Enter para mantener %.2f]: ", prod->precio);
    char precio_str[20];
    if (fgets(precio_str, sizeof(precio_str), stdin) == NULL) return;
    precio_str[strcspn(precio_str, "\n")] = 0;
    if (strlen(precio_str) == 0) {
        precio = prod->precio;
    } else {
        precio = atof(precio_str);
    }
    
    printf("Nuevo stock [Enter para mantener %d]: ", prod->stock);
    char stock_str[20];
    if (fgets(stock_str, sizeof(stock_str), stdin) == NULL) return;
    stock_str[strcspn(stock_str, "\n")] = 0;
    if (strlen(stock_str) == 0) {
        stock = prod->stock;
    } else {
        stock = atoi(stock_str);
    }
    
    printf("Nueva URL imagen [Enter para mantener '%s']: ", prod->imagen_url);
    if (fgets(url, sizeof(url), stdin) == NULL) return;
    url[strcspn(url, "\n")] = 0;
    if (strlen(url) == 0) {
        strcpy(url, prod->imagen_url);
    }
    
    modificar_producto(id, nombre, descripcion, precio, stock, url);
}

void eliminar_producto_ui() {
    char id[37];
    
    printf("ID del producto a eliminar: ");
    fgets(id, sizeof(id), stdin);
    id[strcspn(id, "\n")] = 0;
    
    eliminar_producto(id);
}

void imprimir_productos() {
    printf("\n=== PRODUCTOS (%d) ===\n", productos->len);
    for (int i = 0; i < productos->len; i++) {
        Producto *prod = &g_array_index(productos, Producto, i);
        printf("ID: %s\n", prod->id);
        printf("Nombre: %s\n", prod->nombre);
        printf("Descripción: %s\n", prod->descripcion);
        printf("Precio: $%.2f | Stock: %d\n", prod->precio, prod->stock);
        printf("Categoría: %s | Subcategoría: %s\n", prod->categoria, prod->subcategoria);
        printf("Imagen: %s\n", prod->imagen_url);
        printf("---\n");
    }
}

void imprimir_categorias() {
    printf("\n=== CATEGORÍAS (%d) ===\n", categorias->len);
    for (int i = 0; i < categorias->len; i++) {
        Categoria *cat = &g_array_index(categorias, Categoria, i);
        printf("Categoría: %s\n", cat->nombre);
        printf("Subcategorías (%d): ", cat->subcategorias->len);
        for (int j = 0; j < cat->subcategorias->len; j++) {
            char *subcat = g_array_index(cat->subcategorias, char*, j);
            printf("%s", subcat);
            if (j < cat->subcategorias->len - 1) printf(", ");
        }
        printf("\n---\n");
    }
}

// ==================== MAIN ====================

int main() {
    // Inicializar GLib
    categorias = g_array_new(FALSE, FALSE, sizeof(Categoria));
    productos = g_array_new(FALSE, FALSE, sizeof(Producto));
    
    cargar_datos();
    
    char opcion[10];
    while (1) {
        limpiarPantalla();
        printf("=== CONSOLA DE ADMINISTRADOR | LECHES TOÑO ===\n\n");
        printf("1. Agregar Producto\n");
        printf("2. Modificar Producto\n");
        printf("3. Eliminar Producto\n");
        printf("4. Ver Productos\n");
        printf("5. Ver Categorías\n");
        printf("0. Salir\n");
        printf("Opción: ");
        
        // Usar fgets también para el menú principal
        if (fgets(opcion, sizeof(opcion), stdin) == NULL) {
            break;
        }
        
        // Limpiar newline
        opcion[strcspn(opcion, "\n")] = 0;
        
        if (strlen(opcion) == 0) continue;
        
        switch (opcion[0]) {
            case '1':
                introducir_producto();
                guardar_datos();
                break;
            case '2':
                imprimir_productos();
                modificar_producto_ui();
                guardar_datos();
                break;
            case '3':
                imprimir_productos();
                eliminar_producto_ui();
                guardar_datos();
                break;
            case '4':
                imprimir_productos();
                break;
            case '5':
                imprimir_categorias();
                break;
            case '0':
                guardar_datos();
                // Liberar memoria GLib
                for (int i = 0; i < categorias->len; i++) {
                    Categoria *cat = &g_array_index(categorias, Categoria, i);
                    for (int j = 0; j < cat->subcategorias->len; j++) {
                        char *subcat = g_array_index(cat->subcategorias, char*, j);
                        g_free(subcat);
                    }
                    g_array_free(cat->subcategorias, TRUE);
                }
                g_array_free(categorias, TRUE);
                g_array_free(productos, TRUE);
                printf("¡Hasta pronto!\n");
                return 0;
            default:
                printf("❌ Opción inválida: %s\n", opcion);
        }
        
        printf("\nPresiona Enter para continuar...");
        getchar();  // Esperar Enter
    }
    
    return 0;
}