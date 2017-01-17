/**
 Author: Xavier Corbillon
 IMT Atlantique

 This class manage the storage in a file of the head position logs
*/
//internal includes
#include "LogWriter.hpp"

using namespace IMT;

using LockGuard = std::lock_guard<std::mutex>;
using UniqueLock = std::unique_lock<std::mutex>;

#define SWAP_LIMIT 10

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
      m_logQueue.push(log-m_startTimestamp);
      if (m_logQueue.size() > SWAP_LIMIT)
      {
        SwapBuffer();
      }
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
    m_writingThread = std::thread(&LogWriter::Writer, this);
  }
}

void LogWriter::Stop(void)
{
  if (m_isRunning)
  {
    {
      UniqueLock locker(m_mutex);
      std::cout << "Stop" << std::endl;
      m_isRunning = false;
      SwapBuffer(false);
      m_cv.wait(locker, [this](){return m_output == nullptr;});
    }
    ++m_testId;
    m_writingThread.join();
  }
}

void LogWriter::SwapBuffer(bool withLock)
{
  if (withLock)
  {
    UniqueLock locker(m_mutex);
    std::swap(m_logQueue, m_writerLogQueue);
    m_cv.notify_one();
  }
  else
  {
    std::swap(m_logQueue, m_writerLogQueue);
    m_cv.notify_one();
  }
}


void LogWriter::Writer(void)
{
  while(true)
  {
    UniqueLock locker(m_mutex);
    m_cv.wait(locker, [this](){return !m_writerLogQueue.empty() > 0 || !m_isRunning;});
    while(!m_writerLogQueue.empty())
    {
      *m_output << m_writerLogQueue.front() << "\n";
      m_writerLogQueue.pop();
    }
    if (!m_isRunning)
    { // if the loger is not running anymore we join the thread;
      *m_output << std::flush;
      m_output = nullptr;
      m_cv.notify_one();
      return;
    }
  }
}
