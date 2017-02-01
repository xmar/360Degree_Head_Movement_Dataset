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
  ShaderTextureVideo(std::string pathToVideo, size_t nbFrame = -1, size_t bufferSize = 10, float startOffsetInSecond = 0): ShaderTexture(),
      m_pathToVideo(pathToVideo), m_videoReader(pathToVideo, bufferSize, startOffsetInSecond)
      {m_videoReader.Init(nbFrame);}
  virtual ~ShaderTextureVideo(void) = default;
  virtual void InitAudio(void) override {m_videoReader.InitAudio();}
  virtual void SetStartTime(std::chrono::system_clock::time_point startTime) {m_videoReader.SetStartTime(std::move(startTime));}
private:
  std::string m_pathToVideo;
  LibAv::VideoReader m_videoReader;

  virtual DisplayFrameInfo UpdateTexture(std::chrono::system_clock::time_point deadline) override;
};

}
