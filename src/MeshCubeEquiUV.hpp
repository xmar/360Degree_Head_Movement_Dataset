// Author: Xavier Corbillon
// IMT Atlantique
//
// Description:
// Implementation of a MeshCubeEquiUV: a mesh that represent a cube
#pragma once

//Internal includes
#include "Mesh.hpp"

namespace IMT {

class MeshCubeEquiUV: public Mesh
{
public:
  MeshCubeEquiUV(GLfloat scale, size_t numTriangles = 6*2*15*15);

  virtual ~MeshCubeEquiUV() = default;

private:
  MeshCubeEquiUV(const MeshCubeEquiUV&) = delete;
  MeshCubeEquiUV& operator=(const MeshCubeEquiUV&) = delete;

  // Swizzle each triple of coordinates by the specified
  // index and then multiply by the specified scale.  This
  // lets us implement a poor-man's rotation matrix, where
  // we pick which element (0-2) and which polarity (-1 or
  // 1) to use.
  std::vector<GLfloat> VertexRotate(
      std::vector<GLfloat> const &inVec,
      std::array<size_t, 3> const &indices,
      std::array<GLfloat, 3> const &scales);

  std::vector<GLfloat> VertexToUVs( std::vector<GLfloat> const& inputVertexs);

  virtual void InitImpl(void) override;
};
}
