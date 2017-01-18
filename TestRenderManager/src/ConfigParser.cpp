// Author: Xavier Corbillon
// IMT Atlantique
#include "ConfigParser.hpp"

#include "MeshCube.hpp"
#include "MeshCubeEquiUV.hpp"
#include "ShaderTextureStatic.hpp"
#include "ShaderTextureVideo.hpp"
#include "LogWriter.hpp"

//Library includes
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

//standard library
#include <iostream>
#include <stdexcept>

using namespace IMT;


void ConfigParser::Init(void)
{
  boost::property_tree::ptree pt;
  boost::property_tree::ini_parser::read_ini(m_pathToConfigFile, pt);

  std::cout << "Start parsing " << m_pathToConfigFile << " configuration file\n";
  //Read the Config section
  auto textureConfig = pt.get<std::string>("Config.textureConfig");
  auto projectionConfig = pt.get<std::string>("Config.projectionConfig");
  auto logWriterConfig = pt.get<std::string>("Config.logWriterConfig");

  std::cout << "Parse the texture configuration: section "<< textureConfig <<"\n";

  auto textureType = pt.get<std::string>(textureConfig+".type");
  //Static picture
  if (textureType == "static")
  {
    auto pathToPicture = pt.get<std::string>(textureConfig+".pathToPicture");
    m_outputShaderTexture = std::make_shared<ShaderTextureStatic>(pathToPicture);
  }
  //Video
  else if (textureType == "video")
  {
    auto pathToVideo = pt.get<std::string>(textureConfig+".pathToVideo");
    size_t nbFrame = pt.get<size_t>(textureConfig+".nbFrame");
    size_t  bufferSize = pt.get<size_t>(textureConfig+".bufferSize");
    std::cout << "bufferSize " <<bufferSize << std::endl;
    m_outputShaderTexture = std::make_shared<ShaderTextureVideo>(pathToVideo, nbFrame, bufferSize);
  }
  else
  {
    throw(std::invalid_argument("Not supported texture type: "+textureType));
  }

  std::cout << "Parse the projection configuration: section "<< projectionConfig <<"\n";

  auto projectionType = pt.get<std::string>(projectionConfig+".type");
  if (projectionType == "CubeMap")
  {
    m_outputMesh = std::make_shared<MeshCube>(5.0f);
  }
  else if (projectionType == "Equirectangular")
  {
    m_outputMesh = std::make_shared<MeshCubeEquiUV>(5.0f);
  }
  else
  {
    throw(std::invalid_argument("Not supported projection type: "+projectionType));
  }

  std::cout << "Parse the log writer configuration: section "<< logWriterConfig <<"\n";

  auto logWriterOutputDirPath = pt.get<std::string>(logWriterConfig+".outputDirPath");
  auto logWriterOutputId = pt.get<std::string>(logWriterConfig+".outputId");
  m_outputLogWriter = std::make_shared<LogWriter>(logWriterOutputDirPath, logWriterOutputId);
}
