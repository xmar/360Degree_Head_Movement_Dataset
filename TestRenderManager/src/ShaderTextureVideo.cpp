#include "ShaderTextureVideo.hpp"

using namespace IMT;

DisplayFrameInfo ShaderTextureVideo::UpdateTexture(std::chrono::system_clock::time_point deadline)
{
  auto& textureId = GetTextureId();
  if (textureId == 0)
  {
    glGenTextures(1, &textureId);
  }
  glBindTexture(GL_TEXTURE_2D, textureId);
  auto frameInfo = m_videoReader.SetNextPictureToOpenGLTexture(0, deadline);
  glBindTexture(GL_TEXTURE_2D, 0);
  return std::move(frameInfo);
}
