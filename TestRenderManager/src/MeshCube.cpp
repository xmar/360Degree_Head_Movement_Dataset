//Author: Xavier Corbillon
//IMT Atlantique
#include "MeshCube.hpp"

//standard includes
#include <cmath>

using namespace IMT;

MeshCube::MeshCube(GLfloat scale, size_t numTriangles): Mesh()
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

      auto tmptmpUVBufferData = GetUVs(float(i), float(j), float(numQuadsPerEdge));
      tmpUVBufferData.insert(tmpUVBufferData.end(),
        tmptmpUVBufferData.begin(), tmptmpUVBufferData.end());
    }
  }
  // Make a copy of the vertices for each face, then modulate
  // the color by the face color and rotate the coordinates to
  // put them on the correct cube face.

  // +Z is blue and is in the same location as the original
  // faces.
  {
    // X = X, Y = Y, Z = Z
    std::array<GLfloat, 3> scales = { 1.0f, 1.0f, 1.0f };
    std::array<size_t, 3> indices = { 0, 1, 2 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    //UV cubeMap
    auto myUVBufferData =  TransposeUVs(tmpUVBufferData, 0);

    // Catenate the vertices onto the end of the
    // vertex buffer.
    AppendVertexBufferData(myFaceBufferData);

    // Catenate the UVs onto the end of the
    // UV buffer.
    AppendUvBufferData(myUVBufferData);
  }

  // -Z is cyan and is in the opposite size from the
  // original face (mirror all 3).
  {
    // X = -X, Y = -Y, Z = -Z
    std::array<GLfloat, 3> scales = { -1.0f, -1.0f, -1.0f };
    std::array<size_t, 3> indices = { 0, 1, 2 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    //UV cubeMap
    auto myUVBufferData = TransposeUVs(tmpUVBufferData, 5);

    // Catenate the vertices onto the end of the
    // vertex buffer.
    AppendVertexBufferData(myFaceBufferData);

    // Catenate the UVs onto the end of the
    // UV buffer.
    AppendUvBufferData(myUVBufferData);
  }

  // +X is red and is rotated -90 degrees from the original
  // around Y.
  {
    // X = Z, Y = Y, Z = -X
    std::array<GLfloat, 3> scales = { 1.0f, 1.0f, -1.0f };
    std::array<size_t, 3> indices = { 2, 1, 0 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    //UV cubeMap
    auto myUVBufferData = TransposeUVs(tmpUVBufferData, 2);

    // Catenate the vertices onto the end of the
    // vertex buffer.
    AppendVertexBufferData(myFaceBufferData);

    // Catenate the UVs onto the end of the
    // UV buffer.
    AppendUvBufferData(myUVBufferData);
  }

  // -X is magenta and is rotated 90 degrees from the original
  // around Y.
  {
    // X = -Z, Y = Y, Z = X
    std::array<GLfloat, 3> scales = { -1.0f, 1.0f, 1.0f };
    std::array<size_t, 3> indices = { 2, 1, 0 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    //UV cubeMap
    auto myUVBufferData = TransposeUVs(tmpUVBufferData, 1);

    // Catenate the vertices onto the end of the
    // vertex buffer.
    AppendVertexBufferData(myFaceBufferData);

    // Catenate the UVs onto the end of the
    // UV buffer.
    AppendUvBufferData(myUVBufferData);
  }

  // +Y is green and is rotated -90 degrees from the original
  // around X.
  {
    // X = X, Y = Z, Z = -Y
    std::array<GLfloat, 3> scales = { 1.0f, 1.0f, -1.0f };
    std::array<size_t, 3> indices = { 0, 2, 1 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    //UV cubeMap
    auto myUVBufferData = TransposeUVs(tmpUVBufferData, 3);

    // Catenate the vertices onto the end of the
    // vertex buffer.
    AppendVertexBufferData(myFaceBufferData);

    // Catenate the UVs onto the end of the
    // UV buffer.
    AppendUvBufferData(myUVBufferData);
  }

  // -Y is yellow and is rotated 90 degrees from the original
  // around X.
  {

    // X = X, Y = -Z, Z = Y
    std::array<GLfloat, 3> scales = { 1.0f, -1.0f, 1.0f };
    std::array<size_t, 3> indices = { 0, 2, 1 };
    std::vector<GLfloat> myFaceBufferData =
      VertexRotate(faceBufferData, indices, scales);

    //UV cubeMap
    auto myUVBufferData = TransposeUVs(tmpUVBufferData, 4);

    // Catenate the vertices onto the end of the
    // vertex buffer.
    AppendVertexBufferData(myFaceBufferData);

    // Catenate the UVs onto the end of the
    // UV buffer.
    AppendUvBufferData(myUVBufferData);
  }
}


std::vector<GLfloat> MeshCube::VertexRotate(
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

// return UV map for each vertex of the quad of normalized index (i,j) for face f.
std::vector<GLfloat> MeshCube::GetUVs(float i, float j, float numQuadsPerEdge) {
  std::vector<GLfloat> out;
  out.push_back(i/(4.f*numQuadsPerEdge));
  out.push_back((j+1.f)/(3.f*numQuadsPerEdge));

  out.push_back(i/(4.f*numQuadsPerEdge));
  out.push_back(j/(3.f*numQuadsPerEdge));

  out.push_back((i+1.f)/(4.f*numQuadsPerEdge));
  out.push_back(j/(3.f*numQuadsPerEdge));

  out.push_back((i+1.f)/(4.f*numQuadsPerEdge));
  out.push_back((j+1.f)/(3.f*numQuadsPerEdge));

  out.push_back(i/(4.f*numQuadsPerEdge));
  out.push_back((j+1.f)/(3.f*numQuadsPerEdge));

  out.push_back((i+1.f)/(4.f*numQuadsPerEdge));
  out.push_back(j/(3.f*numQuadsPerEdge));
  return out;
}

std::vector<GLfloat> MeshCube::TransposeUVs( std::vector<GLfloat> const& inputUVs,
    size_t faceId )
{
  std::vector<GLfloat> out;
  if (faceId >= 6)
  {
    return out;
  }
  out = inputUVs;
  if (faceId == 0)
  {
    for (size_t i = 0; i < out.size(); i += 2)
    {
      out[i] =  1.f/2.f - inputUVs[i];
      out[i+1] =  2.f/3.f-inputUVs[i+1];
    }
  }
  else if (faceId == 1)
  {
    for (size_t i = 0; i < out.size(); i += 2)
    {
      out[i] = 3.f/4.f - inputUVs[i];
      out[i+1] = 2.f/3.f-inputUVs[i+1];
    }
  }
  else if (faceId == 2)
  {
    for (size_t i = 0; i < out.size(); i += 2)
    {
      out[i] = 1.f/4.f - inputUVs[i];
      out[i+1] =  2.f/3.f-inputUVs[i+1];
    }
  }
  else if (faceId == 3)
  {
    for (size_t i = 0; i < out.size(); i += 2)
    {
      out[i] = 1.f/2.f - inputUVs[i];
      out[i+1] = 1.f/3.f - inputUVs[i+1];
    }
  }
  else if (faceId == 4)
  {
    for (size_t i = 0; i < out.size(); i += 2)
    {
      out[i] = 1.f/2.f - inputUVs[i];
      out[i+1] = 1.f - inputUVs[i+1];
    }
  }
  else if (faceId == 5)
  {
    for (size_t i = 0; i < out.size(); i += 2)
    {
      out[i] = 1.f - inputUVs[i];
      out[i+1] = inputUVs[i+1] + 1.f/3.f;
    }
  }
  return out;
}

void MeshCube::InitImpl(void)
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
