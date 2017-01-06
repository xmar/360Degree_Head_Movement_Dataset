/**
 Author: Xavier Corbillon
 IMT Atlantique

 This class manage the storage in a file of the head position logs
*/
//internal includes
#include "LogWriter.hpp"

using namespace IMT;

void LogWriter::AddLog(const Log& log)
{
  if (m_isRunning)
  {
    if (m_firstTimestamp)
    {
      m_firstTimestamp = false;
      m_startTimestamp = log.GetTimestamp();
    }
    if (m_lastTimestamp < log.GetTimestamp())
    {
      *m_output << log-m_startTimestamp << "\n";
      m_lastTimestamp = log.GetTimestamp();
    }
  }
}

void LogWriter::Start(void)
{
  if (!m_isRunning)
  {
    m_isRunning = true;
    m_firstTimestamp = true;
    m_output = std::make_shared<std::ofstream>(m_storageFolder + "/" + m_logId + "_" + std::to_string(m_testId)+".txt");
  }
}

void LogWriter::Stop(void)
{
  if (m_isRunning)
  {
    m_isRunning = false;
    *m_output << std::flush;
    m_output = nullptr;
    ++m_testId;
  }
}
