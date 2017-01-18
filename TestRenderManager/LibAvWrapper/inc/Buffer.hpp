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
#include <atomic>

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
    while(true)
    {
      if (!StillAcceptAdd())
      {
        //No more work accepted
        if (m_queue_producer.empty())
        {
          //no more work accepted and no object left in the m_queue_producer
          m_workerDone = true;
        }
        return false;
      }
      if (m_queue_producer.size() <= m_maxQueueSize)
      {
        m_queue_producer.push(std::move(t));
        return true;
      }
      else
      {
        m_cv.wait(locker, [this](){return m_queue_producer.size() <= m_maxQueueSize || !StillAcceptAdd();});
      }
    }
  }

  //Access first element from the queue
  void Pop(void)
  {
    while(!IsAllDones())
    {
      if (!m_queue.empty())
      {
        m_queue.pop();
        PRINT_DEBUG_BUFFER("Poped a frame");
        return;
      }
      else
      {
        PRINT_DEBUG_BUFFER("Pop: swap queues")
        SwapQueues();
      }
    }
    PRINT_DEBUG_BUFFER("No more frame to pop")
  }

  //Access first element from the queue
  std::shared_ptr<T> Get(void)
  {
    while(!IsAllDones())
    {
      if (!m_queue.empty())
      {
        PRINT_DEBUG_BUFFER("Get a frame");
        return m_queue.front();;
      }
      else
      {
        PRINT_DEBUG_BUFFER("Get: swap queues")
        SwapQueues();
        if(m_queue.empty() && !IsAllDones())
        {
          PRINT_DEBUG_BUFFER("Get: nothing to get yet")
          return nullptr;
        }
      }
    }
    PRINT_DEBUG_BUFFER("No frame to get")
    return nullptr;
  }

  //Set the total number of object that will transit through the queue [thread safe]
  void SetTotal(size_t total)
  {
    std::lock_guard<std::mutex> locker(m_mutex);
    PRINT_DEBUG_BUFFER("Set total frame to see: "<<total);
    m_totalAllowedObjects = total;
    m_cv.notify_all();
  }

  //Return true if the buffer will never output any object anymore [from the getter thread]
  bool IsAllDones(void)
  {
    //std::lock_guard<std::mutex> locker(m_mutex);
    return m_workerDone && m_queue.empty();
  }

  //wake up all waiting thread and stop this buffer [thread safe]
  void Stop(void)
  {
    //std::lock_guard<std::mutex> locker(m_mutex);
    PRINT_DEBUG_BUFFER("Stop the buffer")
    m_stopped = true;
    m_cv.notify_all();
  }
private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
  //not thread safe queue used by the getter thread
  std::queue<std::shared_ptr<T>> m_queue;
  //thread safe queue used by the producer
  std::queue<std::shared_ptr<T>> m_queue_producer;
  size_t m_nbSeenObjects;
  size_t m_totalAllowedObjects;
  //m_stopped is true if the buffer has been stopped
  std::atomic_bool m_stopped;
  //m_workerDone is true if the m_queue_producer buffer is empty and the buffer is not
  //allowed to add more object
  std::atomic_bool m_workerDone;
  const size_t m_maxQueueSize;

  //swap the content from the getter and producer queues [from the getter thread]
  void SwapQueues(void)
  {
    if (m_queue.empty())
    {
      {
        std::lock_guard<std::mutex> locker(m_mutex);
        std::swap(m_queue, m_queue_producer);
        if(m_queue_producer.empty() && !StillAcceptAdd())
        {
          m_workerDone = true;
        }
        PRINT_DEBUG_BUFFER("Swap: new buffer size = " << m_queue.size() << std::endl)
       }
      m_cv.notify_all();
    }
  }

  //Return true if the Add function is still allowed to add object to its queue [called from producer thread]
  bool StillAcceptAdd(void)
  {
    return m_nbSeenObjects < m_totalAllowedObjects && !m_stopped;
  }
};
}
