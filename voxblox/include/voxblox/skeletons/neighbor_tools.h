#ifndef VOXBLOX_SKELETONS_NEIGHBOR_TOOLS_H_
#define VOXBLOX_SKELETONS_NEIGHBOR_TOOLS_H_

#include "voxblox/core/common.h"
#include "voxblox/core/layer.h"

namespace voxblox {

template <typename VoxelType>
class NeighborTools {
 public:
  NeighborTools() : voxels_per_side_(0) {}
  NeighborTools(const Layer<VoxelType>* layer) : layer_(layer) {
    CHECK_NOTNULL(layer);
    voxels_per_side_ = layer_->voxels_per_side();
    CHECK_NE(voxels_per_side_, 0);
  }

  void setLayer(const Layer<VoxelType>* layer) {
    CHECK_NOTNULL(layer);
    layer_ = layer;
    voxels_per_side_ = layer_->voxels_per_side();
    CHECK_NE(voxels_per_side_, 0u);
  }

  void getNeighborsAndDistances(
      const BlockIndex& block_index, const VoxelIndex& voxel_index,
      int connectivity, AlignedVector<VoxelKey>* neighbors,
      AlignedVector<float>* distances,
      AlignedVector<Eigen::Vector3i>* directions) const;

  void getNeighbor(const BlockIndex& block_index, const VoxelIndex& voxel_index,
                   const Eigen::Vector3i& direction,
                   BlockIndex* neighbor_block_index,
                   VoxelIndex* neighbor_voxel_index) const;

 private:
  const Layer<VoxelType>* layer_;

  size_t voxels_per_side_;
};

// Uses 26-connectivity and quasi-Euclidean distances.
// Directions is the direction that the neighbor voxel lives in. If you
// need the direction FROM the neighbor voxel TO the current voxel, take
// negative of the given direction.
template <typename VoxelType>
void NeighborTools<VoxelType>::getNeighborsAndDistances(
    const BlockIndex& block_index, const VoxelIndex& voxel_index,
    int connectivity, AlignedVector<VoxelKey>* neighbors,
    AlignedVector<float>* distances,
    AlignedVector<Eigen::Vector3i>* directions) const {
  CHECK_NOTNULL(layer_);
  CHECK_NOTNULL(neighbors);
  CHECK_NOTNULL(distances);
  CHECK_NOTNULL(directions);

  static const double kSqrt2 = std::sqrt(2);
  static const double kSqrt3 = std::sqrt(3);

  neighbors->reserve(connectivity);
  distances->reserve(connectivity);
  directions->reserve(connectivity);

  VoxelKey neighbor;
  Eigen::Vector3i direction;
  direction.setZero();
  // Distance 1 set.
  for (unsigned int i = 0; i < 3; ++i) {
    for (int j = -1; j <= 1; j += 2) {
      direction(i) = j;
      getNeighbor(block_index, voxel_index, direction, &neighbor.first,
                  &neighbor.second);
      neighbors->emplace_back(neighbor);
      distances->emplace_back(1.0);
      directions->emplace_back(direction);
    }
    direction(i) = 0;
  }
  if (connectivity > 6) {
    // Distance sqrt(2) set.
    for (unsigned int i = 0; i < 3; ++i) {
      unsigned int next_i = (i + 1) % 3;
      for (int j = -1; j <= 1; j += 2) {
        direction(i) = j;
        for (int k = -1; k <= 1; k += 2) {
          direction(next_i) = k;
          getNeighbor(block_index, voxel_index, direction, &neighbor.first,
                      &neighbor.second);
          neighbors->emplace_back(neighbor);
          distances->emplace_back(kSqrt2);
          directions->emplace_back(direction);
        }
        direction(i) = 0;
        direction(next_i) = 0;
      }
    }
  }

  if (connectivity > 18) {
    // Distance sqrt(3) set.
    for (int i = -1; i <= 1; i += 2) {
      direction(0) = i;
      for (int j = -1; j <= 1; j += 2) {
        direction(1) = j;
        for (int k = -1; k <= 1; k += 2) {
          direction(2) = k;
          getNeighbor(block_index, voxel_index, direction, &neighbor.first,
                      &neighbor.second);
          neighbors->emplace_back(neighbor);
          distances->emplace_back(kSqrt3);
          directions->emplace_back(direction);
        }
      }
    }
  }
}

template <typename VoxelType>
void NeighborTools<VoxelType>::getNeighbor(const BlockIndex& block_index,
                                    const VoxelIndex& voxel_index,
                                    const Eigen::Vector3i& direction,
                                    BlockIndex* neighbor_block_index,
                                    VoxelIndex* neighbor_voxel_index) const {
  CHECK_NOTNULL(layer_);
  DCHECK(neighbor_block_index != NULL);
  DCHECK(neighbor_voxel_index != NULL);

  *neighbor_block_index = block_index;
  *neighbor_voxel_index = voxel_index + direction;

  for (unsigned int i = 0; i < 3; ++i) {
    while ((*neighbor_voxel_index)(i) < 0) {
      (*neighbor_block_index)(i)--;
      (*neighbor_voxel_index)(i) += voxels_per_side_;
    }
    while ((*neighbor_voxel_index)(i) >=
           static_cast<IndexElement>(voxels_per_side_)) {
      (*neighbor_block_index)(i)++;
      (*neighbor_voxel_index)(i) -= voxels_per_side_;
    }
  }
}

}  // namespace voxblox

#endif  // VOXBLOX_SKELETONS_NEIGHBOR_TOOLS_H_
