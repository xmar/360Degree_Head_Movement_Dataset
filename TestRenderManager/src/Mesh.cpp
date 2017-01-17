//Author: Xavier Corbillon
//IMT Atlantique
#include "Mesh.hpp"
#include "ShaderTexture.hpp"

using namespace IMT;

Mesh::~Mesh(void)
{
  if(m_initialized)
  {
    glDeleteBuffers(1, &m_vertexBufferId);
    glDeleteBuffers(1, &m_uvBufferId);
    glDeleteVertexArrays(1, &m_vertexArrayId);
    m_initialized = false;
  }
}

DisplayFrameInfo Mesh::Draw(const GLdouble projection[], const GLdouble modelView[],
                std::shared_ptr<ShaderTexture> shader,
                std::chrono::system_clock::time_point deadline)
{
    Init();

    auto frameInfo = shader->useProgram(projection, modelView, std::move(deadline));

    glBindVertexArray(m_vertexArrayId);
    {
        glDrawArrays(GL_TRIANGLES, 0,
                     static_cast<GLsizei>(m_vertexBufferData.size()));
    }
    glBindVertexArray(0);
    return std::move(frameInfo);
}

void Mesh::Init(void)
{
  if (!m_initialized) {
      // Vertex buffer
      glGenBuffers(1, &m_vertexBufferId);
      glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(m_vertexBufferData[0]) * m_vertexBufferData.size(),
                   &m_vertexBufferData[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      // UV buffer
      glGenBuffers(1, &m_uvBufferId);
      glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferId);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(m_uvBufferData[0]) * m_uvBufferData.size(),
                   &m_uvBufferData[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      // Vertex array object
      glGenVertexArrays(1, &m_vertexArrayId);
      glBindVertexArray(m_vertexArrayId);
      {
          //Call specific implementation of the specialization class
          InitImpl();
      }
      glBindVertexArray(0);
      m_initialized = true;
  }
}
