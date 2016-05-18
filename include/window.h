#include <gtk/gtk.h>
extern GtkWidget *gl_area;

/* Signals */
static void activate (GtkApplication *app, gpointer user_data);
static void realize (GtkWidget *widget);
static void unrealize (GtkWidget *widget);
gboolean render (GtkGLArea *area, GdkGLContext *context);
/* Functions */
static int make_resources(void);
static GLuint make_texture(void);
static GLuint update_texture(void);
static GLuint make_buffer(GLenum target,const void *buffer_data,GLsizei buffer_size);
static GLuint make_shader(GLenum type, const char *filename);
static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);
void displayGTK(void);
void *file_contents(const char *filename, GLint *length);
gboolean update_gui(gpointer data);
static gboolean on_play_button_clicked (GtkWidget *widget, gpointer data);
static gboolean on_load_button_clicked (GtkWidget *widget, GtkWindow *window);

gpointer threademulate(gpointer data);
