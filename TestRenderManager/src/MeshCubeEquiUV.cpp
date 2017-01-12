//Author: Xavier Corbillon
//IMT Atlantique
#include "MeshCubeEquiUV.hpp"

//standard includes
#include <cmath>

using namespace IMT;

MeshCubeEquiUV::MeshCubeEquiUV(GLfloat scale, size_t numTriangles): Mesh()
{
  // Figure out how many quads we have per edge.  There
  // is a minimum of 1.
  size_t numQuads = numTriangles / 2;
  size_t numQuadsPerFace = numQuads / 6;
  size_t numQuadsPerEdge = static_cast<size_t> (
    std::sqrt(numQuadsPerFace));
  if (numQuadsPerEdge < 1) { numQuadsPerEdge = 1; }

  // Construct a white square with the specified number of
  // quads as the +Z face of the cube.  We'll copy this
  // and we'll
  // adjust the coordinates by rotation to match each face.
  std::vector<GLfloat> faceBufferData;
  std::vector<GLfloat> tmpUVBufferData;
  for (size_t i = 0; i < numQuadsPerEdge; i++) {
    for (size_t j = 0; j < numQuadsPerEdge; j++) {

      // Send the two triangles that make up this quad, where the
      // quad covers the appropriate fraction of the face from
      // -scale to scale in X and Y.
      GLfloat Z = scale;
      GLfloat minX = -scale + i * (2 * scale) / numQuadsPerEdge;
      GLfloat maxX = -scale + (i+1) * (2 * scale) / numQuadsPerEdge;
      GLfloat minY = -scale + j * (2 * scale) / numQuadsPerEdge;
      GLfloat maxY = -scale + (j + 1) * (2 * scale) / numQuadsPerEdge;
      { //regular square
        faceBufferData.push_back(minX);
        faceBufferData.push_back(maxY);
        faceBufferData.push_back(Z);

        faceBufferData.push_back(minX);
        faceBufferData.push_back(minY);
        faceBufferData.push_back(Z);

        faceBufferData.push_back(maxX);
        faceBufferData.push_back(minY);
        faceBufferData.push_back(Z);

        faceBufferData.push_back(maxX);
        faceBufferData.push_back(maxY);
        faceBufferData.push_back(Z);

        faceBufferData.push_back(minX);
        faceBufferData.push_back(maxY);
        faceBufferData.push_back(Z);

        faceBufferData.push_back(maxX);
        faceBufferData.push_back(minY);
        faceBufferData.push_back(Z);
      }
    }
  }
  // Make a copy of the vertices for each face, then modulate
  // the color by the face color and rotate the coordinates to
  // put them on the correct cube face.

  std::vector<GLfloat> tmpVertexBufferData;

  // +Z is blue and is in the same location as the original
  // faces.
  {
    // X = X, Y = Y, Z = Z
    std::array<GLfloat, 3> scales = { 1.0f, 1.0f, 1.0f };
    std::array<size_t, 3> indices = { 0, 1, 2 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    tmpVertexBufferData.insert(tmpVertexBufferData.end(),
      myFaceBufferData.begin(), myFaceBufferData.end());
  }

  // -Z is cyan and is in the opposite size from the
  // original face (mirror all 3).
  {
    // X = -X, Y = -Y, Z = -Z
    std::array<GLfloat, 3> scales = { -1.0f, -1.0f, -1.0f };
    std::array<size_t, 3> indices = { 0, 1, 2 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    tmpVertexBufferData.insert(tmpVertexBufferData.end(),
      myFaceBufferData.begin(), myFaceBufferData.end());
  }

  // +X is red and is rotated -90 degrees from the original
  // around Y.
  {
    // X = Z, Y = Y, Z = -X
    std::array<GLfloat, 3> scales = { 1.0f, 1.0f, -1.0f };
    std::array<size_t, 3> indices = { 2, 1, 0 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    tmpVertexBufferData.insert(tmpVertexBufferData.end(),
      myFaceBufferData.begin(), myFaceBufferData.end());
  }

  // -X is magenta and is rotated 90 degrees from the original
  // around Y.
  {
    // X = -Z, Y = Y, Z = X
    std::array<GLfloat, 3> scales = { -1.0f, 1.0f, 1.0f };
    std::array<size_t, 3> indices = { 2, 1, 0 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    tmpVertexBufferData.insert(tmpVertexBufferData.end(),
      myFaceBufferData.begin(), myFaceBufferData.end());
  }

  // +Y is green and is rotated -90 degrees from the original
  // around X.
  {
    // X = X, Y = Z, Z = -Y
    std::array<GLfloat, 3> scales = { 1.0f, 1.0f, -1.0f };
    std::array<size_t, 3> indices = { 0, 2, 1 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    tmpVertexBufferData.insert(tmpVertexBufferData.end(),
      myFaceBufferData.begin(), myFaceBufferData.end());
  }

  // -Y is yellow and is rotated 90 degrees from the original
  // around X.
  {

    // X = X, Y = -Z, Z = Y
    std::array<GLfloat, 3> scales = { 1.0f, -1.0f, 1.0f };
    std::array<size_t, 3> indices = { 0, 2, 1 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    tmpVertexBufferData.insert(tmpVertexBufferData.end(),
      myFaceBufferData.begin(), myFaceBufferData.end());
  }
  AppendVertexBufferData(tmpVertexBufferData);
  AppendUvBufferData(VertexToUVs(tmpVertexBufferData));
}


std::vector<GLfloat> MeshCubeEquiUV::VertexRotate(
    std::vector<GLfloat> const &inVec,
    std::array<size_t, 3> const &indices,
    std::array<GLfloat, 3> const &scales)
{
  std::vector<GLfloat> out;
  size_t elements = inVec.size() / 3;
  if (elements * 3 != inVec.size()) {
    // We don't have an even multiple of 3 elements, so bail.
    return out;
  }
  out.resize(inVec.size());
  for (size_t i = 0; i < elements; i++) {
    for (size_t p = 0; p < 3; p++) {
      out[3 * i + p] = inVec[3*i + indices[p]] * scales[p];
    }
  }
  return out;
}

constexpr float PI = 3.141592653589793238462643383279502884L;

std::vector<GLfloat> MeshCubeEquiUV::VertexToUVs( std::vector<GLfloat> const& inputVertexs)
{
  std::vector<GLfloat> out;
  //Generate Equirectangular UV map
  for(size_t i = 0; i < inputVertexs.size(); i += 3)
  {
    auto& x = inputVertexs[i];
    auto& y = inputVertexs[i+1];
    auto& z = inputVertexs[i+2];
    auto rho = std::sqrt(x*x+y*y+z*z);
    auto theta = std::atan2(z, x) + PI;
    auto phi = std::acos(y / rho);
    out.push_back(theta/(2.f*PI));
    out.push_back(phi/PI);
  }
  //Check if two vertix are at opposite side of the texture
  for(size_t i = 0; i < out.size(); i += 6)
  {
      if (std::abs(out[i] - out[i+2]) > 0.7)
      {
        if (out[i] < 0.3)
        {
          out[i] += 1.f;
        }
        else
        {
          out[i+2] += 1.f;
        }
      }
      if (std::abs(out[i] - out[i+4]) > 0.7)
      {
        if (out[i] < 0.3)
        {
          out[i] += 1.f;
        }
        else
        {
          out[i+4] += 1.f;
        }
      }
      if (std::abs(out[i+2] - out[i+4]) > 0.7)
      {
        if (out[i+2] < 0.3)
        {
          out[i+2] += 1.f;
        }
        else
        {
          out[i+4] += 1.f;
        }
      }
  }
  return out;
}

void MeshCubeEquiUV::InitImpl(void)
{
  // UV
  glBindBuffer(GL_ARRAY_BUFFER, GetUvBufferId());
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  // VBO
  glBindBuffer(GL_ARRAY_BUFFER, GetVertexBufferId());
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}
