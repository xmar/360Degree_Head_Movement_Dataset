/*
  Author: Xavier Corbillon
  IMT Atlantique

  Multi-thread protected Buffer class to store in a queue objects
  (not copyable but movable)
*/
#pragma once

//standard includes
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

#define DEBUG_BUFFER 0
#if DEBUG_BUFFER == 1
  #include <iostream>
  #define PRINT_DEBUG_BUFFER(x) {std::cout << "[Debug buffer]: " << x << "\n";}
#else
  #define PRINT_DEBUG_BUFFER(x) {}
#endif

namespace IMT
{
template <class T>
class Buffer
{
public:
  Buffer(size_t bufferSize): m_mutex(), m_cv(), m_queue(), m_nbSeenObjects(0), m_totalAllowedObjects(0), m_stopped(false), m_maxQueueSize(bufferSize) {};
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) noexcept = default;
  Buffer& operator=(Buffer&&) noexcept = default;
  ~Buffer(void) {std::lock_guard<std::mutex> locker (m_mutex);}

  //Add element t in the queue when queue size < m_maxQueueSize. Return false if will not add anything anymore
  bool Add(std::shared_ptr<T> t)
  {
    std::unique_lock<std::mutex> locker(m_mutex);
    if (IsAllDones__notProtected() || m_stopped)
    {
      PRINT_DEBUG_BUFFER("Cannot add more frame to the buffer")
      locker.unlock();
      m_cv.notify_all();
      return false;
    }
    PRINT_DEBUG_BUFFER("Wait to add a frame in the buffer")
    m_cv.wait(locker, [this](){return m_queue.size() <= m_maxQueueSize || m_stopped;});
    if (m_stopped)
    {
      PRINT_DEBUG_BUFFER("Cannot add more frame to the buffer: buffer stopped")
      locker.unlock();
      return false;
    }
    m_queue.push(std::move(t));
    ++m_nbSeenObjects;
    PRINT_DEBUG_BUFFER("Add the frame to the buffer: buffer size "<< m_queue.size() << "; number of object seen: " <<m_nbSeenObjects)
    locker.unlock();
    m_cv.notify_all();
    return true;
  }

  //Access first element from the queue
  void Pop(void)
  {
    std::unique_lock<std::mutex> locker(m_mutex);
    if (m_stopped)
    {
      PRINT_DEBUG_BUFFER("No more frame to pop")
      locker.unlock();
      return;
    }
    if (!IsAllDones__notProtected())
    {
      PRINT_DEBUG_BUFFER("Wait to pop a frame")
      m_cv.wait(locker, [this](){return m_queue.size() > 0 || m_stopped || IsAllDones__notProtected();});
    }
    if (m_queue.size() > 0 && !m_stopped)
    {
      m_queue.pop();
      PRINT_DEBUG_BUFFER("Poped a frame: buffer size " << m_queue.size() << "; number of object seen: " <<m_nbSeenObjects)
      locker.unlock();
      m_cv.notify_all();
    }
    else if (IsAllDones__notProtected() || m_stopped)
    {
      PRINT_DEBUG_BUFFER("No more frame to pop")
      locker.unlock();
    }
  }

  //Access first element from the queue
  std::shared_ptr<T> Get(void)
  {
    std::unique_lock<std::mutex> locker(m_mutex);
    if (m_stopped)
    {
      PRINT_DEBUG_BUFFER("No more frame to get")
      locker.unlock();
      return nullptr;
    }
    else
    {
      if (!IsAllDones__notProtected())
      {
        PRINT_DEBUG_BUFFER("Wait to get a frame")
        m_cv.wait(locker, [this](){return m_queue.size() > 0 || m_stopped || IsAllDones__notProtected();});
      }
      if (m_queue.size() > 0 && !m_stopped)
      {
        auto t = m_queue.front();
        PRINT_DEBUG_BUFFER("Got a frame: buffer size " << m_queue.size() << "; number of object seen: " <<m_nbSeenObjects)
        locker.unlock();
        m_cv.notify_all();
        return t;
      }
      else if (IsAllDones__notProtected() || m_stopped)
      {
        PRINT_DEBUG_BUFFER("No more frame to get")
        locker.unlock();
        return nullptr;
      }
    }
  }

  //Set the total number of object that will transit through the queue
  void SetTotal(size_t total)
  {
    std::lock_guard<std::mutex> locker(m_mutex);
    PRINT_DEBUG_BUFFER("Set total frame to see: "<<total);
    m_totalAllowedObjects = total;
    m_cv.notify_all();
  }

  //Return true if no more object are allowed in the buffer (ie m_nbSeenObjects >= m_totalAllowedObjects)
  //and if queue is empty
  bool IsAllDones(void)
  {
    std::lock_guard<std::mutex> locker(m_mutex);
    return IsAllDones__notProtected() && m_queue.empty();
  }

  //wake up all waiting thread and stop this buffer
  void Stop(void)
  {
    std::lock_guard<std::mutex> locker(m_mutex);
    PRINT_DEBUG_BUFFER("Stop the buffer")
    m_stopped = true;
    m_cv.notify_all();
  }
private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
  std::queue<std::shared_ptr<T>> m_queue;
  size_t m_nbSeenObjects;
  size_t m_totalAllowedObjects;
  bool m_stopped;
  const size_t m_maxQueueSize;

  bool IsAllDones__notProtected(void) const
  {
    return m_nbSeenObjects >= m_totalAllowedObjects;
  }
};
}
