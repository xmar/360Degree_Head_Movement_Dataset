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
  TimePoint m_timestamp;
  bool m_last;
} DisplayFrameInfo;

}
