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
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

namespace IMT {

class LogWriter
{
public:
  LogWriter(std::string storageFolder, std::string logId): m_storageFolder(storageFolder),
        m_logId(logId), m_isRunning(false), m_testId(0), m_output(nullptr), m_lastTimestamp(0,0),
        m_startTimestamp(0,0), m_firstTimestamp(true),
        m_logQueue(), m_writerLogQueue(), m_mutex(), m_writingThread(),
        m_lastLog(Timestamp(0,0), Timestamp(0,0), Quaternion(0,0,0,0), 0)
        {}
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
  Log m_lastLog;

  std::queue<Log> m_logQueue;
  std::queue<Log> m_writerLogQueue;

  //multithread specific members
  std::mutex m_mutex;
  std::condition_variable m_cv;
  std::thread m_writingThread;

  //main function of the writing thread
  void Writer(void);
  //used by main thread to give logs to the writing thread
  void SwapBuffer(bool withLock = true);
};
}
