// Author: Xavier Corbillon
// IMT Atlantique
//
// Description:
// Shader Texture implementation for a texture extract from a video

//internal includes
#include "ShaderTexture.hpp"
#include "VideoReader.hpp"

namespace IMT {

class ShaderTextureVideo: public ShaderTexture
{
public:
  ShaderTextureVideo(std::string pathToVideo): ShaderTexture(),
      m_pathToVideo(pathToVideo), m_videoReader(pathToVideo)
      {m_videoReader.Init(100000);}
  virtual ~ShaderTextureVideo(void) = default;
private:
  std::string m_pathToVideo;
  LibAv::VideoReader m_videoReader;

  virtual void UpdateTexture(std::chrono::system_clock::time_point deadline) override;
};

}
