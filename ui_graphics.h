#ifndef UI_GRAPHICS_H
#define UI_GRAPHICS_H
#include <array>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include "item.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <nuklear/nuklear.h>
#include "ui.h"

class Renderer;

class UIGraphics {
public:
  UIGraphics(GLFWwindow* window, const UI& ui);
  void render();
  std::optional<Item> get_hovering() const;

private:
  using UITexture = std::uint32_t;

  nk_context* ctx_;
  std::unordered_map<std::string, UITexture> ui_textures;
  std::unordered_map<UITexture, struct nk_image> icons_;
  std::unordered_map<Item, UITexture> item_to_texture_;

  static constexpr float widget_spacing = 4.f;
  static constexpr float button_padding = 4.f;
  float font_height_;

  const UI& ui_;

  std::optional<Item> hovering_;
};

#endif