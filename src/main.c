/** This file is just an example */

#include <glslwrap/glslwrap.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#include <signal.h>

static volatile int keepRunning = 1;

void intHandler(int dummy) { keepRunning = 0; }

int main(int argc, char *argv[]) {

  signal(SIGINT, intHandler);
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_FLOATING, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  GLFWwindow *window = glfwCreateWindow(640, 480, "hello", 0, 0);

  glfwMakeContextCurrent(window);

  glewInit();

  GLSLShaderProgram *program = glsl_compile_shader_program(
      "#version 440 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "uniform mat4 projection;\n"
      "uniform mat4 view;\n"
      "uniform mat4 model;\n"
      "void main()\n"
      "{\n"
      "gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, "
      "1.0);\n"
      "   //gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}\0",
      "#version 440 core\n"
      "out vec4 FragColor;\n"
      "uniform vec4 color;\n"
      "void main()\n"
      "{\n"
      "   FragColor = color;\n"
      "}\n\0");

  mat4 view = GLM_MAT4_IDENTITY_INIT;
  mat4 projection = GLM_MAT4_IDENTITY_INIT;
  mat4 model = GLM_MAT4_IDENTITY_INIT;

  glm_ortho(0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.0f, -100.0f,
            100.0f, projection);
  glsl_shader_program_set(
      program, glsl_init_shader_variable(SHADER_VARIABLE_MAT4, "projection",
                                         projection, 0));
  glm_translate(view, (vec3){0.0f, 0.0f, 0.0f});
  glsl_shader_program_set(program, glsl_init_shader_variable(
                                       SHADER_VARIABLE_MAT4, "view", view, 0));
  glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
  glsl_shader_program_set(
      program,
      glsl_init_shader_variable(SHADER_VARIABLE_MAT4, "model", model, 0));
  glsl_shader_program_set(
      program, glsl_init_shader_variable(SHADER_VARIABLE_VEC4, "color", 0,
                                         (float[4]){1, 1, 1, 1.0f}));

  GLSLDrawObject *draw_object = glsl_init_draw_object(program);

  float *vertices = (float[]){0.0f, 0.0f, 0.0f, 1920.0f, 1080.0f, 0.0f};

  GLSLVerticeBuffer *vertice_buffer =
      glsl_init_vertice_buffer(vertices, 3, 2, 6);
  draw_object->vertice_buffer = vertice_buffer;

  GLSLAttributeList *attr_list = glsl_init_attribute_list(
      (GLSLAttribute *[]){
          glsl_init_attribute(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0)},
      1);

  draw_object->attributes = attr_list;

  while (!glfwWindowShouldClose(window) && keepRunning) {
    glBindVertexArray(draw_object->VAO);
    glUseProgram(program->id);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glsl_buffer_object(draw_object);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glsl_draw_object_free(draw_object);

  glfwDestroyWindow(window);
  glfwTerminate();

  printf("Done\n");
  return 0;
}
