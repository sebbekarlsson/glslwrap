#include <glslwrap/glslwrap.h>
#include <glslwrap/macros.h>
#include <stdlib.h>

GLSLShaderProgram *
glsl_compile_shader_program(const char *vertex_shader_text,
                            const char *fragment_shader_text) {

  int success;
  char infoLog[512];
  unsigned int vertex_shader;
  unsigned int fragment_shader;
  unsigned int program;
  GLSLShaderProgram *glsl_program = NEW(GLSLShaderProgram);
  glsl_program->variables = NEW_MAP();

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, (const GLchar *const *)&vertex_shader_text,
                 NULL);
  glCompileShader(vertex_shader);

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
    printf("Could not compile vertex_shader %s\n", infoLog);
    return 0;
  }

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1,
                 (const GLchar *const *)&fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
    printf("Could not compile vertex_shader %s\n", infoLog);
    return 0;
  }

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    printf("Error compiling shader program: %s\n", infoLog);
    return 0;
  }

  glsl_program->id = program;
  glsl_program->fragment_shader = fragment_shader;
  glsl_program->vertex_shader = vertex_shader;

  return glsl_program;
}

void glsl_shader_program_update(GLSLShaderProgram *program) {
  unsigned int variables_len = 0;
  char **keys = 0;
  map_get_keys(program->variables, &keys, &variables_len);

  for (uint32_t i = 0; i < variables_len; i++) {
    char *key = keys[i];
    if (!key)
      continue;

    GLSLShaderVariable *var = map_get_value(program->variables, key);

    switch (var->type) {
    case SHADER_VARIABLE_MAT4: {

      glUniformMatrix4fv(glGetUniformLocation(program->id, var->name), 1,
                         GL_FALSE, *var->mat4_value);
    }; break;
    case SHADER_VARIABLE_VEC4: {
      glUniform4fv(glGetUniformLocation(program->id, var->name), 1,
                   (float[4]){var->vec4_value[0], var->vec4_value[1],
                              var->vec4_value[2], var->vec4_value[3]});
    }; break;
    default: { /*noop*/
    }; break;
    }
  }
}

void glsl_shader_program_set(GLSLShaderProgram *program,
                             GLSLShaderVariable *variable) {
  map_set(program->variables, variable->name, variable);
}

GLSLShaderVariable *glsl_init_shader_variable(EShaderVariableType type,
                                              const char *name, mat4 mat4_value,
                                              float vec4_value[]) {
  GLSLShaderVariable *var = NEW(GLSLShaderVariable);
  if (vec4_value) {
    memcpy(&var->vec4_value[0], &vec4_value[0], 4 * sizeof(float));
  }
  var->type = type;
  var->name = strdup(name);
  if (mat4_value) {
    glm_mat4_copy(mat4_value, var->mat4_value);
  }
  return var;
}

GLSLDrawObject *glsl_buffer_object(GLSLDrawObject *draw_object) {
  if (!draw_object)
    return 0;
  if (!draw_object->program)
    return 0;

  if (!draw_object->VAO) {
    glGenVertexArrays(1, &draw_object->VAO);
  }

  uint32_t array_offset = 0;
  uint32_t element_offset = 0;
  GLSLVerticeBuffer *vertice_buffer = draw_object->vertice_buffer;
  GLSLElementBuffer *element_buffer = draw_object->element_buffer;
  GLSLAttributeList *attribute_list = draw_object->attributes;
  unsigned int vertices_len = 0;
  unsigned int indices_len = 0;

  glBindVertexArray(draw_object->VAO);

  glUseProgram(draw_object->program->id);

  if (vertice_buffer) {
    vertices_len = vertice_buffer->columns * vertice_buffer->rows;
    if (vertices_len) {
      glBindBuffer(GL_ARRAY_BUFFER, vertice_buffer->VBO);
      glBufferData(GL_ARRAY_BUFFER, vertices_len * sizeof(float), 0,
                   GL_STATIC_DRAW); // allocate
      glBufferSubData(GL_ARRAY_BUFFER, array_offset,
                      vertices_len * sizeof(float), &vertice_buffer->values[0]);
    }
  }

  if (element_buffer) {
    indices_len = element_buffer->columns * element_buffer->rows;
    if (indices_len) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer->EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices_len * sizeof(float), 0,
                   GL_STATIC_DRAW); // allocate
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, element_offset,
                      indices_len * sizeof(unsigned int),
                      &element_buffer->values[0]);
    }
  }

  if (attribute_list) {
    uint32_t cursor = 0;
    unsigned int attributes_len = attribute_list->length;
    for (uint32_t i = 0; i < attributes_len; i++) {
      GLSLAttribute *attr = attribute_list->values[i];
      glVertexAttribPointer(attr->index, attr->size, attr->type, GL_FALSE,
                            vertice_buffer->columns * sizeof(float),
                            (void *)0 +
                                (array_offset + (cursor * sizeof(float))));
      glEnableVertexAttribArray(attr->index);
      cursor += attr->size;
    }
  }

  glsl_shader_program_update(draw_object->program);

  if (vertices_len > 0 || indices_len > 0) {
    glBindVertexArray(draw_object->VAO);
    glDrawArrays(GL_LINES, 0, 2);
  }

  return draw_object;
}

GLSLDrawObject *glsl_init_draw_object(GLSLShaderProgram *program) {
  GLSLDrawObject *obj = NEW(GLSLDrawObject);
  obj->program = program;
  glGenVertexArrays(1, &obj->VAO);
  return obj;
}

GLSLVerticeBuffer *glsl_init_vertice_buffer(float values[], uint32_t columns,
                                            uint32_t rows, uint32_t length) {
  GLSLVerticeBuffer *buffer = NEW(GLSLVerticeBuffer);
  buffer->columns = columns;
  buffer->rows = rows;
  buffer->length = length;
  glGenBuffers(1, &buffer->VBO);

  buffer->values = (float *)calloc(length, sizeof(float));

  for (int i = 0; i < length; i++) {
    buffer->values[i] = values[i];
  }

  return buffer;
}
GLSLElementBuffer *glsl_init_element_buffer(unsigned int values[],
                                            uint32_t columns, uint32_t rows,
                                            uint32_t length) {
  GLSLElementBuffer *buffer = NEW(GLSLElementBuffer);
  buffer->columns = columns;
  buffer->rows = rows;
  buffer->length = length;

  buffer->values = (unsigned int *)calloc(length, sizeof(unsigned int));
  memcpy(buffer->values, values, length * sizeof(unsigned int));

  return buffer;
}

GLSLAttribute *glsl_init_attribute(unsigned int index, uint32_t size,
                                   GLenum type, GLboolean normalized,
                                   GLsizei stride, void *ptr) {
  GLSLAttribute *attr = NEW(GLSLAttribute);
  attr->index = index;
  attr->size = size;
  attr->type = type;
  attr->normalized = normalized;
  attr->stride = stride;
  attr->ptr = ptr;
  return attr;
}

GLSLAttributeList *glsl_init_attribute_list(GLSLAttribute *attributes[],
                                            uint32_t length) {
  GLSLAttributeList *attrlist = NEW(GLSLAttributeList);
  attrlist->length = length;
  if (length) {
    attrlist->values =
        (GLSLAttribute **)calloc(length, sizeof(GLSLAttribute *));

    for (uint32_t i = 0; i < length; i++) {
      attrlist->values[i] = attributes[i];
    }
  }

  return attrlist;
}

void glsl_shader_program_free(GLSLShaderProgram *program) {
  if (!program)
    return;

  if (program->variables) {
    char **keys = 0;
    unsigned int variables_len = 0;
    map_get_keys(program->variables, &keys, &variables_len);

    for (int i = 0; i < (int)variables_len; i++) {
      char *key = keys[i];
      if (!key)
        continue;
      GLSLShaderVariable *var =
          (GLSLShaderVariable *)map_get_value(program->variables, key);
      if (!var)
        continue;
      glsl_shader_variable_free(var);
    }
    map_free(program->variables);
  }

  if (program->fragment_shader) {
    glDeleteShader(program->fragment_shader);
  }

  if (program->vertex_shader) {
    glDeleteShader(program->vertex_shader);
  }

  if (program->id) {
    glDeleteProgram(program->id);
  }

  free(program);
}

void glsl_shader_variable_free(GLSLShaderVariable *var) {
  if (!var)
    return;
  if (var->name) {
    free(var->name);
  }

  free(var);
}

void glsl_vertice_buffer_free(GLSLVerticeBuffer *buffer) {
  if (!buffer)
    return;
  if (buffer->VBO) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &buffer->VBO);
  }
  if (buffer->values) {
    free(buffer->values);
  }

  free(buffer);
}

void glsl_element_buffer_free(GLSLElementBuffer *buffer) {
  if (!buffer)
    return;
  if (buffer->EBO) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &buffer->EBO);
  }
  if (buffer->values) {
    free(buffer->values);
  }

  free(buffer);
}

void glsl_attribute_free(GLSLAttribute *attr) {
  if (!attr)
    return;
  free(attr);
}

void glsl_attribute_list_free(GLSLAttributeList *attribute_list) {
  if (!attribute_list)
    return;
  if (attribute_list->values) {
    for (uint32_t i = 0; i < attribute_list->length; i++) {
      glsl_attribute_free(attribute_list->values[i]);
    }
  }

  free(attribute_list->values);
  free(attribute_list);
}

void glsl_draw_object_free(GLSLDrawObject *draw_object) {
  if (!draw_object)
    return;

  if (draw_object->program) {
    glsl_shader_program_free(draw_object->program);
  }

  if (draw_object->element_buffer) {
    glsl_element_buffer_free(draw_object->element_buffer);
  }

  if (draw_object->vertice_buffer) {
    glsl_vertice_buffer_free(draw_object->vertice_buffer);
  }

  if (draw_object->attributes) {
    glsl_attribute_list_free(draw_object->attributes);
  }

  if (draw_object->VAO) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &draw_object->VAO);
  }

  free(draw_object);
}
