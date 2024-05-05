#include "terrain_graphics.h"
#include <cstddef>
#include <type_traits>
#include "mesh_generator.h"
#include "region.h"
#include "render_utils.h"
#include "renderer.h"
#include "sky.h"
#include "stb_image.h"
#include "types.h"

template <typename T>
TerrainGraphics::MultiDrawHandle& TerrainGraphics::get_multi_draw_handle() {
  if constexpr (std::is_same_v<T, CubeVertex>)
    return cubes_draw_handle_;
  else if constexpr (std::is_same_v<T, Vertex>)
    return irregular_draw_handle_;
}

template <typename T>
void TerrainGraphics::set_up_vao() {
  if constexpr (std::is_same_v<T, CubeVertex>) {
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(CubeVertex), (void*)offsetof(CubeVertex, data_));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(CubeVertex), (void*)offsetof(CubeVertex, lighting_));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
  } else if constexpr (std::is_same_v<T, Vertex>) {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvs));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, layer));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, lighting));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
  }
}

template <typename T>
void TerrainGraphics::set_up() {
  auto& mdh = get_multi_draw_handle<T>();

  int defacto_vertices;
  if constexpr (std::is_same_v<T, CubeVertex>) {
    defacto_vertices = MeshGenerator::defacto_vertices_per_mesh;
    RenderUtils::create_shader(&mdh.shader, "shaders/terrain.vs", "shaders/terrain.fs");
  } else if constexpr (std::is_same_v<T, Vertex>) {
    defacto_vertices = MeshGenerator::defacto_vertices_per_irregular_mesh;
    RenderUtils::create_shader(&mdh.shader, "shaders/irregular.vs", "shaders/terrain.fs");
  }

  glGenBuffers(1, &mdh.vbo);
  glGenVertexArrays(1, &mdh.vao);

  glBindBuffer(GL_ARRAY_BUFFER, mdh.vbo);
  glBindVertexArray(mdh.vao);

  set_up_vao<T>();

  mdh.vbo_size = sizeof(T) * defacto_vertices * Region::max_sz;

  glBufferData(GL_ARRAY_BUFFER, mdh.vbo_size, nullptr, GL_STATIC_DRAW);

  for (int idx = 0; idx < mdh.commands.size(); ++idx) {
    auto& command = mdh.commands[idx];
    auto& metadata = mdh.commands_metadata[idx];
    command.count = 0;
    command.instance_count = 1;
    command.base_instance = 0;
    command.first = idx * defacto_vertices;
    metadata.buffer_size = sizeof(T) * defacto_vertices;
    metadata.occupied = false;
  }
  glGenBuffers(1, &mdh.ibo);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mdh.ibo);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawArraysIndirectCommand) * mdh.commands.size(), mdh.commands.data(), GL_STATIC_DRAW);
}

TerrainGraphics::TerrainGraphics() {
  set_up<CubeVertex>();
  glCreateBuffers(1, &cpx_ssbo_);
  glNamedBufferStorage(cpx_ssbo_, sizeof(glm::vec3) * cubes_draw_handle_.commands.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
  glCreateBuffers(1, &cpy_ssbo_);
  glNamedBufferStorage(cpy_ssbo_, sizeof(glm::vec3) * cubes_draw_handle_.commands.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
  glCreateBuffers(1, &cpz_ssbo_);
  glNamedBufferStorage(cpz_ssbo_, sizeof(glm::vec3) * cubes_draw_handle_.commands.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

  set_up<Vertex>();

  glGenTextures(1, &voxel_texture_array_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, voxel_texture_array_);
  GLint num_layers = static_cast<GLint>(VoxelTexture::num_voxel_textures);
  GLsizei width = 16;
  GLsizei height = 16;
  GLsizei channels;
  GLsizei num_mipmaps = 1;
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, num_mipmaps, GL_SRGB8_ALPHA8, width, height, num_layers);
  stbi_set_flip_vertically_on_load(1);
  std::vector<std::pair<std::string, VoxelTexture>> textures = {
    std::make_pair("dirt", VoxelTexture::dirt),
    std::make_pair("grass", VoxelTexture::grass),
    std::make_pair("grass_side", VoxelTexture::grass_side),
    std::make_pair("water", VoxelTexture::water),
    std::make_pair("sand", VoxelTexture::sand),
    std::make_pair("tree_trunk", VoxelTexture::tree_trunk),
    std::make_pair("leaves", VoxelTexture::leaves),
    std::make_pair("sandstone", VoxelTexture::sandstone),
    std::make_pair("stone", VoxelTexture::stone),
    std::make_pair("standing_grass", VoxelTexture::standing_grass),
  };
  for (auto [filename, texture] : textures) {
    std::string path = "images/" + filename + ".png";
    auto* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<int>(texture), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
  }
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // uniforms
  glUseProgram(cubes_draw_handle_.shader);
  GLint texture_loc = glGetUniformLocation(cubes_draw_handle_.shader, "textureArray");
  glUniform1i(texture_loc, 0);

  glUseProgram(irregular_draw_handle_.shader);
  texture_loc = glGetUniformLocation(irregular_draw_handle_.shader, "textureArray");
  glUniform1i(texture_loc, 0);
}

void TerrainGraphics::create(const Location& loc, const MeshGenerator& mesh_generator) {
  upload(loc, mesh_generator.get_mesh(loc), mesh_generator.get_origin());
  upload(loc, mesh_generator.get_irregular_mesh(loc), mesh_generator.get_origin());
}

template void TerrainGraphics::upload<CubeVertex>(const Location&, const std::vector<CubeVertex>&, const Location&);
template void TerrainGraphics::upload<Vertex>(const Location&, const std::vector<Vertex>&, const Location&);

template <typename T>
void TerrainGraphics::upload(const Location& loc, const std::vector<T>& mesh, const Location& offset) {
  MultiDrawHandle& mdh = get_multi_draw_handle<T>();

  std::size_t idx;
  if (mdh.loc_to_command_index.contains(loc)) {
    idx = mdh.loc_to_command_index[loc];
  } else {
    for (int i = 0; i < mdh.commands.size(); ++i) {
      auto& metadata = mdh.commands_metadata[i];
      if (!metadata.occupied) {
        idx = i;
        metadata.occupied = true;
        break;
      }
    }

    if constexpr (std::is_same_v<T, CubeVertex>) {
      float chunk_pos_x = (loc[0] - offset[0]) * Chunk::sz_x;
      float chunk_pos_y = (loc[1] - offset[1]) * Chunk::sz_y;
      float chunk_pos_z = (loc[2] - offset[2]) * Chunk::sz_z;

      int ssbo_float_offset = sizeof(float) * idx;
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, cpx_ssbo_);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, ssbo_float_offset, sizeof(float), &chunk_pos_x);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, cpy_ssbo_);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, ssbo_float_offset, sizeof(float), &chunk_pos_y);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, cpz_ssbo_);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, ssbo_float_offset, sizeof(float), &chunk_pos_z);
    }
  }

  auto& command = mdh.commands[idx];
  auto& metadata = mdh.commands_metadata[idx];

  GLuint mesh_size_bytes = sizeof(T) * mesh.size();
  if (mesh_size_bytes > metadata.buffer_size) {
    std::cout << "Buffer too small for mesh with size " << mesh.size() << std::endl;

    GLuint new_vbo;
    glGenBuffers(1, &new_vbo);

    glBindBuffer(GL_COPY_READ_BUFFER, mdh.vbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, new_vbo);

    // need to make sure added_size is a multiple of sizeof(T)
    int added_size = (mesh_size_bytes - metadata.buffer_size) + (metadata.buffer_size * .05);
    added_size /= sizeof(T);
    added_size *= sizeof(T);
    glBufferData(GL_COPY_WRITE_BUFFER, mdh.vbo_size + added_size, nullptr, GL_STATIC_DRAW);

    // Copy beginning of old buffer
    int pre_size = command.first * sizeof(T);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, pre_size);

    // Copy this mesh
    glBufferSubData(GL_COPY_WRITE_BUFFER, pre_size, mesh_size_bytes, mesh.data());

    if (idx < mdh.commands.size() - 1) {
      // Copy end of old buffer
      int post_size = pre_size + metadata.buffer_size + added_size;

      glCopyBufferSubData(
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        pre_size + metadata.buffer_size,
        post_size,
        mdh.vbo_size - metadata.buffer_size - pre_size);
    }

    mdh.vbo_size += added_size;
    metadata.buffer_size += added_size;

    // Shift the first index of every command following this one
    for (int i = idx + 1; i < mdh.commands.size(); ++i) {
      auto& command = mdh.commands[i];
      command.first += (added_size / sizeof(T));
    }
    command.count = mesh.size();

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mdh.ibo);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, idx * sizeof(DrawArraysIndirectCommand), sizeof(DrawArraysIndirectCommand) * (mdh.commands.size() - idx), mdh.commands.data() + idx);

    glDeleteBuffers(1, &mdh.vbo);
    mdh.vbo = new_vbo;

    glBindBuffer(GL_ARRAY_BUFFER, mdh.vbo);
    glBindVertexArray(mdh.vao);

    set_up_vao<T>();

  } else {
    glBindBuffer(GL_ARRAY_BUFFER, mdh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(T) * command.first, mesh_size_bytes, mesh.data());

    command.count = mesh.size();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mdh.ibo);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawArraysIndirectCommand) * idx, sizeof(DrawArraysIndirectCommand), &command);
  }

  mdh.loc_to_command_index[loc] = idx;
}

void TerrainGraphics::remove(const Location& loc, MultiDrawHandle& mdh) {
  auto idx = mdh.loc_to_command_index[loc];
  auto& command = mdh.commands[idx];
  auto& metadata = mdh.commands_metadata[idx];
  command.count = 0;
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mdh.ibo);
  glBufferSubData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawArraysIndirectCommand) * idx, sizeof(DrawArraysIndirectCommand), &command);
  mdh.loc_to_command_index.erase(loc);
  metadata.occupied = false;
}

void TerrainGraphics::destroy(const Location& loc) {
  remove(loc, cubes_draw_handle_);
  remove(loc, irregular_draw_handle_);
}

void TerrainGraphics::render(const Renderer& renderer) const {
  {
    auto& mdh = cubes_draw_handle_;

    glUseProgram(mdh.shader);
    auto transform_loc = glGetUniformLocation(mdh.shader, "uTransform");
    auto& projection = renderer.get_projection_matrix();
    auto& view = renderer.get_view_matrix();
    auto transform = projection * view;
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
    auto camera_world_position_loc = glGetUniformLocation(mdh.shader, "uCameraWorldPosition");
    auto& camera_world_position = renderer.get_camera_world_position();
    glUniform3fv(camera_world_position_loc, 1, glm::value_ptr(camera_world_position));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, voxel_texture_array_);
    GLuint voxel_textures_loc = glGetUniformLocation(mdh.shader, "textureArray");
    glUniform1i(voxel_textures_loc, 0);
    const Sky& sky = renderer.get_sky();
    GLuint texture = sky.get_texture();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glUniform1i(glGetUniformLocation(mdh.shader, "skybox"), 1);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cpx_ssbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, cpy_ssbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, cpz_ssbo_);
    glBindVertexArray(mdh.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mdh.ibo);
    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, mdh.commands.size(), 0);
  }
  {
    auto& mdh = irregular_draw_handle_;

    glUseProgram(mdh.shader);
    auto transform_loc = glGetUniformLocation(mdh.shader, "uTransform");
    auto& projection = renderer.get_projection_matrix();
    auto& view = renderer.get_view_matrix();
    auto transform = projection * view;
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
    auto camera_world_position_loc = glGetUniformLocation(mdh.shader, "uCameraWorldPosition");
    auto& camera_world_position = renderer.get_camera_world_position();
    glUniform3fv(camera_world_position_loc, 1, glm::value_ptr(camera_world_position));

    GLuint voxel_textures_loc = glGetUniformLocation(mdh.shader, "textureArray");
    glUniform1i(voxel_textures_loc, 0);
    const Sky& sky = renderer.get_sky();
    GLuint texture = sky.get_texture();
    glUniform1i(glGetUniformLocation(mdh.shader, "skybox"), 1);

    glBindVertexArray(mdh.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mdh.ibo);
    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, mdh.commands.size(), 0);
  }
}