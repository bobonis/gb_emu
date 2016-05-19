int make_resources(void);
static GLuint make_texture(void);
static GLuint update_texture(void);
static GLuint make_buffer(GLenum target,const void *buffer_data,GLsizei buffer_size);
static GLuint make_shader(GLenum type, const char *filename);
static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);
void *file_contents(const char *filename, GLint *length);
static inline void check_gl_error(char* error_str);
void render_gl_area(void);