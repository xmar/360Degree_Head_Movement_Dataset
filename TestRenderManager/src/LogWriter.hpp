/**
 Author: Xavier Corbillon
 IMT Atlantique

 This class manage the storage in a file of the head position logs
*/
#pragma once

//internal includes
#include "Log.hpp"

//standard library
#include <string>
#include <fstream>
#include <memory>
#include <iostream>


namespace IMT {

class LogWriter
{
public:
  LogWriter(std::string storageFolder, std::string logId): m_storageFolder(storageFolder),
        m_logId(logId), m_isRunning(false), m_testId(0), m_output(nullptr), m_lastTimestamp(0,0),
        m_startTimestamp(0,0), m_firstTimestamp(true) {}
  virtual ~LogWriter(void) {if (m_output != nullptr) {Stop();}};

  void AddLog(const Log& log);
  void Start(void);
  void Stop(void);
private:
  std::string m_storageFolder;
  std::string m_logId;
  bool m_isRunning;
  unsigned m_testId;
  std::shared_ptr<std::ofstream> m_output;
  Timestamp m_lastTimestamp;
  Timestamp m_startTimestamp;
  bool m_firstTimestamp;
};
}
