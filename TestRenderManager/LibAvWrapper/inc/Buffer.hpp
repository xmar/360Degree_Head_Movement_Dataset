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

namespace IMT
{
template <class T, size_t m_maxQueueSize = 10>
class Buffer
{
public:
  Buffer(void): m_mutex(), m_cv(), m_queue(), m_nbSeenObjects(0), m_totalAllowedObjects(0) {};
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) noexcept = default;
  Buffer& operator=(Buffer&&) noexcept = default;

  //Add element t in the queue when queue size < m_maxQueueSize. Return false if will not add anything anymore
  bool Add(std::shared_ptr<T> t)
  {
    std::unique_lock<std::mutex> locker(m_mutex);
    if (IsAllDones__notProtected())
    {
      locker.unlock();
      return false;
    }
    m_cv.wait(locker, [this](){return m_queue.size() <= m_maxQueueSize;});
    m_queue.push(std::move(t));
    ++m_nbSeenObjects;
    locker.unlock();
    m_cv.notify_one();
    return true;
  }

  //Access first element from the queue
  std::shared_ptr<T> Get(void)
  {
    std::unique_lock<std::mutex> locker(m_mutex);
    bool allDone = IsAllDones__notProtected();
    if (allDone)
    {
      locker.unlock();
      return nullptr;
    }
    m_cv.wait(locker, [this](){return m_queue.size() > 0;});
    if (m_queue.size() > 0)
    {
      auto t = std::move(m_queue.front());
      m_queue.pop();
      locker.unlock();
      m_cv.notify_one();
      return std::move(t);
    }
    else if (allDone)
    {
      locker.unlock();
      return nullptr;
    }
  }

  //Set the total number of object that will transit through the queue
  void SetTotal(size_t total)
  {
    std::lock_guard<std::mutex> locker(m_mutex);
    m_totalAllowedObjects = total;
  }

  //Return true if no more object are allowed in the buffer (ie m_nbSeenObjects >= m_totalAllowedObjects)
  bool IsAllDones(void)
  {
    std::lock_guard<std::mutex> locker(m_mutex);
    return IsAllDones__notProtected();
  }
private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
  std::queue<std::shared_ptr<T>> m_queue;
  size_t m_nbSeenObjects;
  size_t m_totalAllowedObjects;

  bool IsAllDones__notProtected(void) const
  {
    return m_nbSeenObjects >= m_totalAllowedObjects;
  }
};
}
