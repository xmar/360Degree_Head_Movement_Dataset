// Author: Xavier Corbillon
// IMT Atlantique
//
// Description:
// Implementation of a MeshCube: a mesh that represent a cube
#pragma once

//Internal includes
#include "Mesh.hpp"

namespace IMT {

class MeshCube: public Mesh
{
public:
  MeshCube(GLfloat scale, size_t numTriangles = 2);//6*2*15*15);

  virtual ~MeshCube() = default;

private:
  MeshCube(const MeshCube&) = delete;
  MeshCube& operator=(const MeshCube&) = delete;

  // Swizzle each triple of coordinates by the specified
  // index and then multiply by the specified scale.  This
  // lets us implement a poor-man's rotation matrix, where
  // we pick which element (0-2) and which polarity (-1 or
  // 1) to use.
  std::vector<GLfloat> VertexRotate(
      std::vector<GLfloat> const &inVec,
      std::array<size_t, 3> const &indices,
      std::array<GLfloat, 3> const &scales);

  // return UV map for each vertex of the quad of normalized index (i,j) for face f.
  std::vector<GLfloat> GetUVs(float i, float j, float numQuadsPerEdge);

  std::vector<GLfloat> TransposeUVs( std::vector<GLfloat> const& inputUVs,
      size_t faceId );

  virtual void InitImpl(void) override;
};
}
