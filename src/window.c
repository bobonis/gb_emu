#include <stdlib.h>
#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include "gpu.h"
#include "window.h"
#include "rom.h"
#include "cpu.h"
#include "render.h"

#define STOP 0
#define RUNNING 1
#define PAUSE 2

/* glib-compile-resources --target=gresource.c --generate-source window.gresource.xml */
GtkWidget *gl_area = NULL;
GError *error = NULL;
unsigned int temp=0;

GMutex mutex_interface;
char *rom_filename = NULL;

struct emu emulator = {
    STOP       /* running */
};

static void activate (GtkApplication *app, gpointer user_data)
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *toolbar_play_button;
    GtkWidget *toolbar_load_button;
    GtkWidget *toolbar_pause_button;

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "window.glade", NULL);

    /* Create Window */
    
    window = GTK_WIDGET(gtk_builder_get_object (builder, "main_window") );
    box = GTK_WIDGET (gtk_builder_get_object (builder, "main_box") );
    g_signal_connect (window, "destroy", G_CALLBACK (on_close_button_clicked), NULL);
    toolbar_load_button = GTK_WIDGET( gtk_builder_get_object (builder, "toolbar_load_button") );
    toolbar_play_button = GTK_WIDGET( gtk_builder_get_object (builder, "toolbar_play_button") );
    toolbar_pause_button = GTK_WIDGET( gtk_builder_get_object (builder, "toolbar_pause_button") );
    //gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (toolbar_pause_button), TRUE);
    g_signal_connect (toolbar_load_button, "clicked", G_CALLBACK (on_load_button_clicked), window);
    g_signal_connect (toolbar_play_button, "clicked", G_CALLBACK (on_play_button_clicked), NULL);
    g_signal_connect (toolbar_pause_button, "clicked", G_CALLBACK (on_pause_button_clicked), NULL);
    /* Create GLAREA */
  
    gl_area = gtk_gl_area_new ();
    gtk_widget_set_size_request (gl_area, 480, 432);
    gtk_widget_set_hexpand (gl_area, TRUE);
    gtk_widget_set_vexpand (gl_area, TRUE);
    gtk_container_add (GTK_CONTAINER (box), gl_area);

    /* We need to initialize and free GL resources, so we use
     * the realize and unrealize signals on the widget
     */
    g_signal_connect (gl_area, "realize", G_CALLBACK (realize), NULL);
    g_signal_connect (gl_area, "unrealize", G_CALLBACK (unrealize), NULL);
    g_signal_connect (gl_area, "render", G_CALLBACK (render), NULL);



    /* Show everything */
    if (!gtk_widget_get_visible (window))
        gtk_widget_show_all (window);
    else
        gtk_widget_destroy (window);

    gtk_main ();
}


/* We need to set up our state when we realize the GtkGLArea widget */
static void realize (GtkWidget *widget)
{
    //const char *vertex_path, *fragment_path;
    GdkGLContext *context;

    gtk_gl_area_make_current (GTK_GL_AREA (widget));

    if (gtk_gl_area_get_error (GTK_GL_AREA (widget)) != NULL)
        return;

    context = gtk_gl_area_get_context (GTK_GL_AREA (widget));

    /*
     * Load and create all of our resources:
     */
    make_resources();

}

/* We should tear down the state when unrealizing */
static void unrealize (GtkWidget *widget)
{

    
    gtk_gl_area_make_current (GTK_GL_AREA (widget));

    if (gtk_gl_area_get_error (GTK_GL_AREA (widget)) != NULL)
        return;

    //glDeleteBuffers (1, &position_buffer);
    //glDeleteProgram (program);
}

gboolean render (GtkGLArea *area, GdkGLContext *context)
{

    gtk_gl_area_make_current(area);
        
    render_gl_area();

    return TRUE;
}



void displayGTK(void)
{
    GdkGLContext *context;
    context = gtk_gl_area_get_context (GTK_GL_AREA (gl_area));

    if (gtk_gl_area_get_error (GTK_GL_AREA (gl_area)) != NULL)
        return;
    gtk_gl_area_make_current (GTK_GL_AREA (gl_area));
    

    gtk_gl_area_queue_render (GTK_GL_AREA (gl_area));

}

gpointer threademulate(gpointer data)
{
  
    int count = 0;

    loadRom(rom_filename);
    reset();
    while(1) {

        switch (emulator.state){
            case STOP :
                return NULL;
                break;
            case RUNNING :
                execute();
                break;
            case PAUSE :
                sleep(0.1);
                while (gtk_events_pending())
                    gtk_main_iteration();
                break;
            default:
                break; 
        }
        
        
        
		//inputHandleEvents(event);

    // sometimes update the GUI:
    //while(gtk_events_pending())
    if (count > 1000){
        while (gtk_events_pending())
            gtk_main_iteration();
        count = 0;
    }
    count++;
  }

  return NULL;
}

static void on_close_button_clicked (GtkWidget *widget, gpointer data)
{
    emulator.state = STOP;
    gtk_main_quit();
}


static gboolean on_pause_button_clicked (GtkWidget *widget, gpointer data)
{
    switch (emulator.state){
        case STOP :
            break;
        
        case RUNNING :
            emulator.state = PAUSE;
            break;
    
        case PAUSE :
            emulator.state = RUNNING;
            break;
    
        default:
            break; 
        }

    return TRUE;
}

static gboolean on_play_button_clicked (GtkWidget *widget, gpointer data)
{
    g_print ("Start Emulation\n");
    gtk_widget_set_sensitive (widget, FALSE);
    emulator.state = RUNNING;
    //g_thread_new("thread",threademulate,data);
    threademulate(data);
    
    return TRUE;
}

static gboolean on_load_button_clicked (GtkWidget *widget, GtkWindow *parent_window)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    
    /* Create file filters */
    
    GtkFileFilter *filter_gb = gtk_file_filter_new ();
    GtkFileFilter *filter_all = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter_gb,"Gameboy roms");
    gtk_file_filter_set_name (filter_all,"All files");
    gtk_file_filter_add_pattern (filter_gb, "*.gb");
    gtk_file_filter_add_pattern (filter_all, "*");
    
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          parent_window,
                                          action,
                                          ("_Cancel"),
                                          GTK_RESPONSE_CANCEL,
                                          ("_Open"),
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
                                          
    gtk_window_set_modal (GTK_WINDOW (dialog),
                        FALSE);

    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_gb);
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_all);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if (res == GTK_RESPONSE_ACCEPT){

        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        rom_filename = gtk_file_chooser_get_filename (chooser);
    }

    gtk_widget_destroy (dialog);
    return TRUE;
}


int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}