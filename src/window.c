#include <stdlib.h>
#include <gtk/gtk.h>
#include <epoxy/gl.h>

/* glib-compile-resources --target=gresource.c --generate-source window.gresource.xml */

GError *error = NULL;
unsigned int temp=0;


static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };
static short le_short(unsigned char *bytes)
{
    return bytes[0] | ((char)bytes[1] << 8);
}

void *read_tga(const char *filename, int *width, int *height)
{
    struct tga_header {
       char  id_length;
       char  color_map_type;
       char  data_type_code;
       unsigned char  color_map_origin[2];
       unsigned char  color_map_length[2];
       char  color_map_depth;
       unsigned char  x_origin[2];
       unsigned char  y_origin[2];
       unsigned char  width[2];
       unsigned char  height[2];
       char  bits_per_pixel;
       char  image_descriptor;
    } header;
    int i, color_map_size, pixels_size;
    FILE *f;
    size_t read;
    void *pixels;

    f = fopen(filename, "rb");

    if (!f) {
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return NULL;
    }

    read = fread(&header, 1, sizeof(header), f);

    if (read != sizeof(header)) {
        fprintf(stderr, "%s has incomplete tga header\n", filename);
        fclose(f);
        return NULL;
    }
    if (header.data_type_code != 2) {
        fprintf(stderr, "%s is not an uncompressed RGB tga file\n", filename);
        fclose(f);
        return NULL;
    }
    if (header.bits_per_pixel != 24) {
        fprintf(stderr, "%s is not a 24-bit uncompressed RGB tga file\n", filename);
        fclose(f);
        return NULL;
    }

    for (i = 0; i < header.id_length; ++i)
        if (getc(f) == EOF) {
            fprintf(stderr, "%s has incomplete id string\n", filename);
            fclose(f);
            return NULL;
        }

    color_map_size = le_short(header.color_map_length) * (header.color_map_depth/8);
    for (i = 0; i < color_map_size; ++i)
        if (getc(f) == EOF) {
            fprintf(stderr, "%s has incomplete color map\n", filename);
            fclose(f);
            return NULL;
        }

    *width = le_short(header.width); *height = le_short(header.height);
    pixels_size = *width * *height * (header.bits_per_pixel/8);
    pixels = malloc(pixels_size);

    read = fread(pixels, 1, pixels_size, f);
    fclose(f);

    if (read != pixels_size) {
        fprintf(stderr, "%s has incomplete image\n", filename);
        free(pixels);
        return NULL;
    }

    return pixels;
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
 * Functions for creating OpenGL objects:
 */
static GLuint make_buffer(
    GLenum target,
    const void *buffer_data,
    GLsizei buffer_size
) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

static GLuint make_texture(const char *filename)
{
    int width, height;
    void *pixels = read_tga(filename, &width, &height);
    GLuint texture;

    if (!pixels)
        return 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        width, height, 0,           /* width, height, border */
        GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
        pixels                      /* pixels */
    );
    free(pixels);
    return texture;
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
        //show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
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
        //show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

/*
 * Data used to seed our vertex array and element array buffers:
 */
static const GLfloat g_vertex_buffer_data[] = { 
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
};



/*
 * Load and create all of our resources:
 */
static int make_resources(void)
{
    
      GLenum err;
  /* Clear the viewport */
   err = glGetError();
    if (err)
        g_print(" start make resources %d\n",err); 
        
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

    g_resources.textures[0] = make_texture("hello1.tga");
    g_resources.textures[1] = make_texture("hello2.tga");

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
    if (err)
        g_print(" start make resources %d\n",err); 

    return 1;
}
























static gboolean
render (GtkGLArea    *area,
        GdkGLContext *context)
{
  
  gtk_gl_area_make_current(area);
  
  GLenum err;
  /* Clear the viewport */
   err = glGetError();
    if (err)
        g_print(" before clear %d\n",err); 
  glClearColor (1.0, 1.0, 1.0, 1.0);
  glClear (GL_COLOR_BUFFER_BIT);
  
  err = glGetError();
    if (err)
        g_print("clear %d\n",err);


  g_print("RENDER OK\n");
    
    glUseProgram(g_resources.program);

    err = glGetError();
    if (err)
        g_print("program error %d\n",err);

g_resources.fade_factor +=1;
    glUniform1f(g_resources.uniforms.fade_factor, g_resources.fade_factor);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
    glUniform1i(g_resources.uniforms.textures[0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
    glUniform1i(g_resources.uniforms.textures[1], 1);

    err = glGetError();
    if (err)
        g_print("after texture %d\n",err);
    
    GLuint vao;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);
    
        
    glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
        err = glGetError();
    if (err)
        g_print("after bind 1 buffer %d\n",err);
    glVertexAttribPointer(
        g_resources.attributes.position,  /* attribute */
        2,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(GLfloat)*2,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    
            err = glGetError();
    if (err)
        g_print("glVertexAttribPointer %d\n",err);
        
    glEnableVertexAttribArray(g_resources.attributes.position);
    err = glGetError();
    if (err)
        g_print("after enable vertex %d\n",err);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,  /* mode */
        4,                  /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    glDisableVertexAttribArray(g_resources.attributes.position);

    err = glGetError();
    if (err)
        g_print("before flush %d\n",err);
        
  /* Flush the contents of the pipeline */
  glFlush ();

  return TRUE;
}

/* We should tear down the state when unrealizing */
static void
unrealize (GtkWidget *widget)
{
  gtk_gl_area_make_current (GTK_GL_AREA (widget));

  if (gtk_gl_area_get_error (GTK_GL_AREA (widget)) != NULL)
    return;

  //glDeleteBuffers (1, &position_buffer);
  //glDeleteProgram (program);
}

/* Create and compile a shader */
static GLuint
create_shader (int         type,
               const char *src)
{
  GLuint shader;
  int status;

  shader = glCreateShader (type);
  glShaderSource (shader, 1, &src, NULL);
  glCompileShader (shader);

  glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
    {
      int log_len;
      char *buffer;

      glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_len);

      buffer = g_malloc (log_len + 1);
      glGetShaderInfoLog (shader, log_len, NULL, buffer);

      g_warning ("Compile failure in %s shader:\n%s",
                 type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                 buffer);

      g_free (buffer);

      glDeleteShader (shader);

      return 0;
    }

  return shader;
}






/* We need to set up our state when we realize the GtkGLArea widget */
static void
realize (GtkWidget *widget)
{
  const char *vertex_path, *fragment_path;
  GdkGLContext *context;

  gtk_gl_area_make_current (GTK_GL_AREA (widget));

  if (gtk_gl_area_get_error (GTK_GL_AREA (widget)) != NULL)
    return;

  context = gtk_gl_area_get_context (GTK_GL_AREA (widget));

  GLenum err;
  
  err = glGetError();
    if (err)
        g_print("realize %d\n",err);

  vertex_path = "/shaders/window.vs.glsl";
  fragment_path = "/shaders/window.fs.glsl";

/*
 * Load and create all of our resources:
 */
 
  make_resources();
    err = glGetError();
    if (err)
        g_print("realize 2 %d\n",err);
  g_print("REALIZE\n");
}


static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *gl_area;

  /* Create Window */

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
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