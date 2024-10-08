#ifndef SCENE_COMPONENT_H
#define SCENE_COMPONENT_H

#include <vector>
#include <glm/ext.hpp>
#include "material.h"
#include "flag_manager.h"

enum class PrimitiveType {
  Triangles,
  Lines,
};
enum class VertexAttributeType {
  Float,
  Int
};
struct VertexAttribute {
  VertexAttributeType type;
  std::uint8_t component_count;
};

enum class SceneComponentFlags {
  Dynamic = (1 << 0),
  Dirty = (1 << 1),
};

class SceneComponent : public FlagManager<SceneComponentFlags> {
public:
  SceneComponent();

  std::uint32_t get_id() const;
  void set_id(std::uint32_t id);

  const Material& get_material() const;
  void set_material(const Material& mat);

  const glm::mat4& get_model_transform() const;
  void set_model_transform(const glm::mat4& transform);

  void set_vertex_count(unsigned int count);
  unsigned int get_vertex_count() const;
  const std::vector<std::uint8_t>& get_vertices() const;
  const std::vector<unsigned int>& get_indices() const;
  std::vector<std::uint8_t>& get_vertices();
  std::vector<unsigned int>& get_indices();
  void set_vertices(std::vector<std::uint8_t>&& vertices);
  void memcpy_vertices(const void* src, std::size_t bytes);
  void set_indices(std::vector<unsigned int>&& indices);

  PrimitiveType get_primitive_type() const;
  void set_primitive_type(PrimitiveType type);

  const std::vector<VertexAttribute>& get_vertex_attributes() const;
  std::vector<VertexAttribute>& get_vertex_attributes();
  void set_vertex_attributes(std::vector<VertexAttribute>&& attributes);

private:
  std::uint32_t id_;
  Material material_;
  glm::mat4 model_transform_;
  unsigned int vertex_count_;
  std::vector<std::uint8_t> vertices_;
  std::vector<unsigned int> indices_;
  PrimitiveType primitive_type_;
  std::vector<VertexAttribute> vertex_attributes_;
  std::uint32_t flags_ = 0;
};

#endif