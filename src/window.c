#include <stdlib.h>
#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include "gpu.h"
#include "window.h"
#include "rom.h"
#include "cpu.h"


#define SCREEN_HEIGHT 144
#define SCREEN_WIDTH 160
/* glib-compile-resources --target=gresource.c --generate-source window.gresource.xml */
GtkWidget *gl_area = NULL;
GError *error = NULL;
unsigned int temp=0;
gpointer data1 = NULL;
GMutex mutex_interface;

/*
 * Global data used by our render callback:
 */
static struct {
    GLuint vertex_buffer, element_buffer;
    GLuint textures[2];
    GLuint vertex_shader, fragment_shader, program;
    
    struct {
        GLint fade_factor;
        GLint textures[2];
    } uniforms;

    struct {
        GLint position;
    } attributes;

    GLfloat fade_factor;
} g_resources;

/*
 * Data used to seed our vertex array and element array buffers:
 */
static const GLfloat g_vertex_buffer_data[] = { 
    -1.0f, -1.0f,   //1
     1.0f, -1.0f,   //2
    -1.0f,  1.0f,   //3
     1.0f,  1.0f    //4
};

static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };




static void activate (GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *box;

    /* Create Window */

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 160, 144);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroy), window);

    /* Create Box */
 
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
    gtk_box_set_spacing (GTK_BOX (box), 6);
    gtk_container_add (GTK_CONTAINER (window), box);

    /* Create GLAREA */
  
    gl_area = gtk_gl_area_new ();
    gtk_widget_set_hexpand (gl_area, TRUE);
    gtk_widget_set_vexpand (gl_area, TRUE);
    gtk_container_add (GTK_CONTAINER (box), gl_area);

    /* Create Button */
  
    GtkWidget *button;
    button = gtk_button_new_with_label ("Start");
    g_signal_connect (button, "clicked", G_CALLBACK (start_emu), NULL);
    gtk_container_add (GTK_CONTAINER (box), button);
  
  
    /* We need to initialize and free GL resources, so we use
     * the realize and unrealize signals on the widget
     */
    g_signal_connect (gl_area, "realize", G_CALLBACK (realize), NULL);
    g_signal_connect (gl_area, "unrealize", G_CALLBACK (unrealize), NULL);

    /* The main "draw" call for GtkGLArea */
    g_signal_connect (gl_area, "render", G_CALLBACK (render), NULL);

    if (!gtk_widget_get_visible (window))
        gtk_widget_show_all (window);
    else
        gtk_widget_destroy (window);

    gtk_widget_show_all (window);
}


/* We need to set up our state when we realize the GtkGLArea widget */
static void realize (GtkWidget *widget)
{
    //const char *vertex_path, *fragment_path;
    GdkGLContext *context;
    GLenum err;

    gtk_gl_area_make_current (GTK_GL_AREA (widget));

    if (gtk_gl_area_get_error (GTK_GL_AREA (widget)) != NULL)
        return;

    context = gtk_gl_area_get_context (GTK_GL_AREA (widget));

    err = glGetError();
    if (err) g_print("realize %d\n",err);

    //vertex_path = "/shaders/window.vs.glsl";
    //fragment_path = "/shaders/window.fs.glsl";

    /*
     * Load and create all of our resources:
     */
    make_resources();
    
    err = glGetError();
    if (err) g_print("realize 2 %d\n",err);
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

/*
 * Load and create all of our resources:
 */
static int make_resources(void)
{
    GLenum err;
    
    err = glGetError();
    if (err) g_print(" start make resources %d\n",err); 

    g_resources.vertex_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        g_vertex_buffer_data,
        sizeof(g_vertex_buffer_data)
    );
    g_resources.element_buffer = make_buffer(
        GL_ELEMENT_ARRAY_BUFFER,
        g_element_buffer_data,
        sizeof(g_element_buffer_data)
    );

    g_resources.textures[0] = make_texture();
    g_resources.textures[1] = make_texture();

    if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0){
        g_print("TEXTURE ERROR\n");
        return 0;
    }

    g_resources.vertex_shader = make_shader(
        GL_VERTEX_SHADER,
        "window.vs.glsl"
    );
    if (g_resources.vertex_shader == 0)
        return 0;

    g_resources.fragment_shader = make_shader(
        GL_FRAGMENT_SHADER,
        "window.fs.glsl"
    );
    if (g_resources.fragment_shader == 0)
        return 0;

    g_resources.program = make_program(g_resources.vertex_shader, g_resources.fragment_shader);
    if (g_resources.program == 0)
        return 0;

    g_resources.uniforms.fade_factor
        = glGetUniformLocation(g_resources.program, "fade_factor");
    g_resources.uniforms.textures[0]
        = glGetUniformLocation(g_resources.program, "textures[0]");
    g_resources.uniforms.textures[1]
        = glGetUniformLocation(g_resources.program, "textures[1]");

    g_resources.attributes.position
        = glGetAttribLocation(g_resources.program, "position");
        
    err = glGetError();
    if (err) g_print(" start make resources %d\n",err); 

    return 1;
}

static GLuint make_texture(void)
{
    int y,x;
    GLuint texture;
    GLenum err;
	
    // Clear screen
	for(y = 0; y < SCREEN_HEIGHT; ++y)		
		for(x = 0; x < SCREEN_WIDTH; ++x)
			framebuffer[y][x][0] = framebuffer[y][x][1] = framebuffer[y][x][2] = 100;


    //GLfloat aniso = 0.0;
    glGenTextures(1, &texture);
    err = glGetError();
    if (err) g_print(" glGenTextures %d\n",err); 
    
    glBindTexture(GL_TEXTURE_2D, texture);
    err = glGetError();
    if (err) g_print(" glBindTexture %d\n",err);
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);
    
    err = glGetError();
    if (err) g_print(" glTexParameteri %d\n",err);     
    
    glTexImage2D(
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        SCREEN_WIDTH, SCREEN_HEIGHT, 0,/* width, height, border */
        GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
        (GLvoid*)framebuffer        /* pixels */
    );
    
    err = glGetError();
    if (err) g_print(" glTexImage2D %d\n",err); 
    
    return texture;
}

static GLuint update_texture(void)
{

    GLuint texture = g_resources.textures[0];
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        SCREEN_WIDTH, SCREEN_HEIGHT, 0,/* width, height, border */
        GL_RGB, GL_UNSIGNED_BYTE,   /* external format, type */
        (GLvoid*)framebuffer       /* pixels */
    );
    
    return texture;
}


/*
 * Functions for creating OpenGL objects:
 */
static GLuint make_buffer(GLenum target,const void *buffer_data,GLsizei buffer_size)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

static GLuint make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source)
        return 0;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

gboolean render (GtkGLArea *area, GdkGLContext *context)
{
  
    gtk_gl_area_make_current(area);

    GLenum err;
    GLuint vao;

    err = glGetError();
    if (err) g_print(" before clear %d\n",err); 
    
    /* Clear the viewport */ 
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  
    err = glGetError();
    if (err) g_print("clear %d\n",err);

    g_resources.textures[0] = update_texture();
    g_resources.textures[1] = update_texture();
    
    glUseProgram(g_resources.program);

    err = glGetError();
    if (err) g_print("program error %d\n",err);

    g_resources.fade_factor +=1;
    glUniform1f(g_resources.uniforms.fade_factor, g_resources.fade_factor);
    
    err = glGetError();
    if (err) g_print("glUniform1f %d\n",err);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
    glUniform1i(g_resources.uniforms.textures[0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
    glUniform1i(g_resources.uniforms.textures[1], 1);

    err = glGetError();
    if (err) g_print("after texture %d\n",err);
      
    glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
    
    err = glGetError();
    if (err) g_print("after bind 1 buffer %d\n",err);
    
    glVertexAttribPointer(
        g_resources.attributes.position,  /* attribute */
        2,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(GLfloat)*2,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    
    err = glGetError();
    if (err) g_print("glVertexAttribPointer %d\n",err);
        
    glEnableVertexAttribArray(g_resources.attributes.position);
    
    err = glGetError();
    if (err) g_print("after enable vertex %d\n",err);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,  /* mode */
        4,                  /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    glDisableVertexAttribArray(g_resources.attributes.position);

    err = glGetError();
    if (err) g_print("before flush %d\n",err);
        
    /* Flush the contents of the pipeline */
    glFlush ();

    return TRUE;
}

void *file_contents(const char *filename, GLint *length)
{
    FILE *f = fopen(filename, "r");
    void *buffer;

    if (!f) {
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = fread(buffer, 1, *length, f);
    fclose(f);
    ((char*)buffer)[*length] = '\0';

    return buffer;
}

void displayGTK(void)
{
    GdkGLContext *context;
    context = gtk_gl_area_get_context (GTK_GL_AREA (gl_area));

    if (gtk_gl_area_get_error (GTK_GL_AREA (gl_area)) != NULL)
        return;
    gtk_gl_area_make_current (GTK_GL_AREA (gl_area));
    

    //render (GTK_GL_AREA (gl_area), context);
    gtk_gl_area_queue_render (GTK_GL_AREA (gl_area));
    //gdk_threads_add_idle(update_gui,data1);
}

gboolean update_gui(gpointer data)
{
    g_mutex_lock(&mutex_interface);
      //gtk_gl_area_queue_render (GTK_GL_AREA (gl_area));
      //gtk_widget_queue_draw (gl_area);

  // update the GUI here:
  //gtk_button_set_label(button,"label");
  // And read the GUI also here, before the mutex to be unlocked:
  //gchar * text = gtk_entry_get_text(GTK_ENTRY(entry));
    g_mutex_unlock(&mutex_interface);

    return FALSE;
}

gpointer threademulate(gpointer data)
{
  
    int count = 0;

    loadRom("../../roms/Tetris.gb");
    data1 = data;
    while(1) {

        execute();
        
		//inputHandleEvents(event);

    // sometimes update the GUI:
    //while(gtk_events_pending())
    if (count > 1000){
        //gdk_threads_add_idle(update_gui,data);
        count = 0;
    }
    // or:
    //g_idle_add(update_gui,data);

    count++;
  }

  return NULL;
}



static void start_emu (GtkWidget *widget, gpointer data)
{
    g_print ("Start Emulation\n");
    g_thread_new("thread",threademulate,data);
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