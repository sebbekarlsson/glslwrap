#ifndef GLSLWRAP_H
#define GLSLWRAP_H
#include <cglm/call.h> /* for library call (this also includes cglm.h) */
#include <cglm/cglm.h> /* for inline */
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <hashmap/map.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned int id;
  unsigned int vertex_shader;
  unsigned int fragment_shader;
  map_T *variables;
} GLSLShaderProgram;

void glsl_shader_program_free(GLSLShaderProgram *program);

typedef enum { SHADER_VARIABLE_MAT4, SHADER_VARIABLE_VEC4 } EShaderVariableType;

typedef struct {
  EShaderVariableType type;
  char *name;
  mat4 mat4_value;
  float vec4_value[4];
} GLSLShaderVariable;

void glsl_shader_variable_free(GLSLShaderVariable *var);

typedef struct {
  float *values;
  uint32_t length;
  unsigned int VBO;
  uint32_t columns;
  uint32_t rows;
} GLSLVerticeBuffer;

void glsl_vertice_buffer_free(GLSLVerticeBuffer *buffer);

GLSLVerticeBuffer *glsl_init_vertice_buffer(float values[], uint32_t columns,
                                            uint32_t rows, uint32_t length);

typedef struct {
  unsigned int *values;
  uint32_t length;
  unsigned int EBO;
  uint32_t columns;
  uint32_t rows;
} GLSLElementBuffer;

void glsl_element_buffer_free(GLSLElementBuffer *buffer);

GLSLElementBuffer *glsl_init_element_buffer(unsigned int values[],
                                            uint32_t columns, uint32_t rows,
                                            uint32_t length);

typedef struct {
  unsigned int index;
  uint32_t size;
  GLenum type;
  GLboolean normalized;
  GLsizei stride;
  void *ptr;
} GLSLAttribute;

void glsl_attribute_free(GLSLAttribute *attr);

GLSLAttribute *glsl_init_attribute(unsigned int index, uint32_t size,
                                   GLenum type, GLboolean normalized,
                                   GLsizei stride, void *ptr);

typedef struct {
  GLSLAttribute **values;
  uint32_t length;
} GLSLAttributeList;

void glsl_attribute_list_free(GLSLAttributeList *attribute_list);

GLSLAttributeList *glsl_init_attribute_list(GLSLAttribute *attributes[],
                                            uint32_t length);

typedef struct {
  GLSLShaderProgram *program;
  GLSLVerticeBuffer *vertice_buffer;
  GLSLElementBuffer *element_buffer;
  GLSLAttributeList *attributes;
  unsigned int VAO;
} GLSLDrawObject;

void glsl_draw_object_free(GLSLDrawObject *draw_object);

GLSLDrawObject *glsl_init_draw_object(GLSLShaderProgram *program);

GLSLDrawObject *glsl_buffer_object(GLSLDrawObject *draw_object);

GLSLShaderVariable *glsl_init_shader_variable(EShaderVariableType type,
                                              const char *name, mat4 mat4_value,
                                              float vec4_value[]);

void glsl_shader_program_set(GLSLShaderProgram *program,
                             GLSLShaderVariable *variable);

void glsl_shader_program_update(GLSLShaderProgram *program);

GLSLShaderProgram *
glsl_compile_shader_program(const char *vertex_shader_text,
                            const char *fragment_shader_text);

#define GLSL_ATTRIBUTE(index, size, type, normalized, stride)                  \
  (GLSLAttribute) { index, size, type, normalized, stride, 0 }

#endif
