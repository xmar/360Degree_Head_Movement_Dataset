/*
  Author: Xavier Corbillon
  IMT Atlantique

  structure that contains informations about the displayed frame
*/
#pragma once

//standard includes
#include <chrono>

namespace IMT {

typedef struct DisplayFrameInfo
{
public:
  using TimePoint = std::chrono::system_clock::time_point;
  size_t m_frameDisplayId;
  size_t m_nbDroppedFrame;
  TimePoint m_timestamp;
  TimePoint m_pts;
  bool m_last;
} DisplayFrameInfo;

}
