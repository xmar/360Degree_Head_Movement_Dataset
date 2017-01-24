// Author: Xavier Corbillon
// IMT Atlantique
// Parse an ini config file to generate a Mesh object and a Shader
#pragma once

//standard includes
#include <string>
#include <memory>

namespace IMT
{
//Forward declaration
class Mesh;
class ShaderTexture;
class LogWriter;
class PublisherLogMQ;

class ConfigParser
{
public:
  ConfigParser(std::string pathToConfigFile): m_pathToConfigFile(pathToConfigFile),
    m_outputMesh(nullptr), m_outputShaderTexture(nullptr), m_outputLogWriter(nullptr),
    m_outputPublisherLogMQ(nullptr) {}

  //Do the actual parsing. Should be call before GetMesh and GetShaderTexture
  void Init(void);

  auto GetMesh(void) const {return m_outputMesh;}
  auto GetShaderTexture(void) const {return m_outputShaderTexture;}
  auto GetLogWriter(void) const {return m_outputLogWriter;}
  auto GetPublisherLogMQ(void) const {return m_outputPublisherLogMQ;}
private:
  std::string m_pathToConfigFile;
  std::shared_ptr<Mesh> m_outputMesh;
  std::shared_ptr<ShaderTexture> m_outputShaderTexture;
  std::shared_ptr<LogWriter> m_outputLogWriter;
  std::shared_ptr<PublisherLogMQ> m_outputPublisherLogMQ;
};
}
