#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
extern GtkWidget *gl_area;

/* Signals */
static void activate (GtkApplication *app, gpointer user_data);
static void realize (GtkWidget *widget);
static void unrealize (GtkWidget *widget);
gboolean render (GtkGLArea *area, GdkGLContext *context);
/* Functions */

void displayGTK(void);

static gboolean on_play_button_clicked (GtkWidget *widget, gpointer data);
static gboolean on_load_button_clicked (GtkWidget *widget, GtkWindow *window);
static gboolean  on_pause_button_clicked (GtkWidget *widget, gpointer data);
static void on_close_button_clicked (GtkWidget *widget, gpointer data);
gpointer threademulate(gpointer data);
static gboolean on_key_press (GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean on_key_release (GtkWidget *widget, GdkEventKey *event, gpointer data);
struct emu{
    int state;
    char *rom_filename;
    int autostart;
}emulator;

#ifdef __cplusplus
}
#endif