// Author: Xavier Corbillon
// IMT Atlantique
//
// Description:
// Shader Texture implementation for a static texture (the same for each frame)

//internal includes
#include "ShaderTexture.hpp"

namespace IMT {

class ShaderTextureStatic: public ShaderTexture
{
public:
  ShaderTextureStatic(std::string pathToTexture): ShaderTexture(), m_pathToTexture(pathToTexture) {}
  virtual ~ShaderTextureStatic(void) = default;
private:
  std::string m_pathToTexture;

  virtual void UpdateTexture(void) override;
};

}
