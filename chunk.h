#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include <vector>
#include "types.h"

class Chunk {
public:
  enum Flags : uint32_t {
    DELETED = 1 << 0,
  };

  Chunk(int x, int y, int z);
  Chunk(Location location);

  const Location& get_location() const;

  Voxel::VoxelType get_voxel(int x, int y, int z) const;
  Voxel::VoxelType get_voxel(int i) const;

  void set_voxel(int x, int y, int z, Voxel::VoxelType value);
  void set_voxel(int i, Voxel::VoxelType value);

  void set_flag(Flags flag);
  void unset_flag(Flags flag);
  bool check_flag(Flags flag);

  static Location pos_to_loc(const std::array<double, 3>& position);

  static constexpr int sz_uniform = 32;
  static constexpr int sz_x = sz_uniform;
  static constexpr int sz_y = sz_uniform;
  static constexpr int sz_z = sz_uniform;
  static constexpr int sz = sz_x * sz_y * sz_z;

private:
  std::vector<Voxel::VoxelType> voxels_;
  Location location_;

  uint32_t flags_;
};

#endif