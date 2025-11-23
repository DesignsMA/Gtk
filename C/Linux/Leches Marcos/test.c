#include <gtk/gtk.h>
// crea una ventana, esto no es la app, es tan solo un widget especial
static void activate (GtkApplication* app, gpointer user_data)
{
    GtkWidget *window; // clase general widget

    window = gtk_application_window_new(app); // crear objeto y guardar apuntador
    // cast a clase window
    gtk_window_set_title(GTK_WINDOW(window), "Leches Marcos");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280,720);
    gtk_window_present (GTK_WINDOW (window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("github.com.designsma", G_APPLICATION_DEFAULT_FLAGS );
    // conectar la señal "activate" de la instacia app a la funcion activate (hacer cast a callback antes)
    // G_CALLBACK, NULL, no pasamos datos a la funcion
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    // se retorna estatus cuando TERMINA la applicacion
    g_object_unref(app); // borrando memoria
    return status;
}