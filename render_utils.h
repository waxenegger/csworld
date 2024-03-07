#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <fstream>
#include <iostream>
#include <string>
#define GLM_FORCE_LEFT_HANDED
#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "types.h"

namespace RenderUtils {
  void create_shader(GLuint* shader, std::string vertex_shader_path, std::string fragment_shader_path);
  void set_up_standard_vao(GLuint vbo, GLuint vao);
} // namespace RenderUtils

#endif