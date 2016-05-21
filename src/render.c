#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <stdlib.h>
#include "render.h"
#include "gpu.h"

#define SCREEN_HEIGHT 144
#define SCREEN_WIDTH 160


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








void render_gl_area (void)
{
    GLuint vao;

    check_gl_error("Start rendering");
    
    /* Clear the viewport */ 
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
 

    g_resources.textures[0] = update_texture();
    g_resources.textures[1] = update_texture();
 
    check_gl_error("Textures updated");
        
    glUseProgram(g_resources.program);

    g_resources.fade_factor +=1;
    glUniform1f(g_resources.uniforms.fade_factor, g_resources.fade_factor);
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
    glUniform1i(g_resources.uniforms.textures[0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
    glUniform1i(g_resources.uniforms.textures[1], 1);

    check_gl_error("Textures activated");
      
    glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
    
    glVertexAttribPointer(
        g_resources.attributes.position,  /* attribute */
        2,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(GLfloat)*2,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    
    check_gl_error("Vertex updated");
        
    glEnableVertexAttribArray(g_resources.attributes.position);
   
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);

    glDrawElements(
        GL_TRIANGLE_STRIP,  /* mode */
        4,                  /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    check_gl_error("Elements updated");
    
    glDisableVertexAttribArray(g_resources.attributes.position);
    
    check_gl_error("Flush buffers");
    
    /* Flush the contents of the pipeline */
    glFlush ();
    
    check_gl_error("Finish rendering");
}

/*
 * Load and create all OpenGL resources
 */
int make_resources(void)
{

    check_gl_error("Init OpenGL");
    
    /* Initialize buffers */
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

    /* Initialize textures */
    g_resources.textures[0] = make_texture();
    g_resources.textures[1] = make_texture();

    if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0){
        g_print("TEXTURE ERROR\n");
        return 0;
    }

    /* Initialize shaders */
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

    /* Initialize OpenGL program */
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
        
    check_gl_error("Finish OpenGL init");

    return 1;
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

static GLuint make_texture(void)
{
    int y,x;
    GLuint texture;
	
    // Clear screen
	for(y = 0; y < SCREEN_HEIGHT; ++y)		
		for(x = 0; x < SCREEN_WIDTH; ++x)
			framebuffer[y][x][0] = framebuffer[y][x][1] = framebuffer[y][x][2] = 100;


    //GLfloat aniso = 0.0;
    glGenTextures(1, &texture);
    
    glBindTexture(GL_TEXTURE_2D, texture);

    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);

    glTexImage2D(
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        SCREEN_WIDTH, SCREEN_HEIGHT, 0,/* width, height, border */
        GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
        (GLvoid*)framebuffer        /* pixels */
    );
    
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

static inline void check_gl_error(char* error_str)
{
    GLenum error_code;
    error_code = glGetError();
    
    if (error_code) 
        g_print("[ERROR][GL %d] %s\n",error_code,error_str);
}