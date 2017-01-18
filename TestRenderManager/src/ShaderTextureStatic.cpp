// Author: Xavier Corbillon
// IMT Atlantique

#include "ShaderTextureStatic.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


using namespace IMT;

DisplayFrameInfo ShaderTextureStatic::UpdateTexture(std::chrono::system_clock::time_point deadline)
{
  auto& textureId = GetTextureId();
  if(textureId == 0)
  {
    std::cout << "Load texture " << std::endl;
    int w;
    int h;
    int comp;
    unsigned char* image = stbi_load(m_pathToTexture.c_str(), &w, &h, &comp, 0);
    if(image == nullptr)
    {
      throw(std::string("Failed to load texture"));
    }
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    std::cout << "Loaded with " << comp << " comp" << std::endl;

    if(comp == 3)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    else if(comp == 4)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    std::cout << "Texture loaded" << std::endl;
  }
  return {0, 0, deadline, deadline, false};
}
