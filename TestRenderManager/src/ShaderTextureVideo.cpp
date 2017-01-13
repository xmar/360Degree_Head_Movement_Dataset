#include "ShaderTextureVideo.hpp"

using namespace IMT;

void ShaderTextureVideo::UpdateTexture(std::chrono::system_clock::time_point deadline)
{
  auto& textureId = GetTextureId();
  if (textureId == 0)
  {
    glGenTextures(1, &textureId);
  }
  glBindTexture(GL_TEXTURE_2D, textureId);
  m_videoReader.SetNextPictureToOpenGLTexture(0, deadline);
  glBindTexture(GL_TEXTURE_2D, 0);
}
