/**
 Author: Xavier Corbillon
 IMT Atlantique

 This class store a timestamp
*/
#pragma once

//library includes
#include <osvr/Util/TimeValueC.h>

// Standard includes
#include <iostream>
#include <iomanip>

namespace IMT {
class Timestamp
{
public:
  Timestamp(long seconds, long microseconds): m_seconds(seconds), m_microseconds(microseconds) {}
  Timestamp(const OSVR_TimeValue& time): m_seconds(time.seconds), m_microseconds(time.microseconds) {}
  ~Timestamp(void) = default;

  const long& GetSec(void) const {return m_seconds;}
  const long& GetMicrosec(void) const {return m_microseconds;}
  bool operator<(const Timestamp& t) const {return GetSec() < t.GetSec() || (GetSec() == t.GetSec()  && GetMicrosec() < t.GetMicrosec());}
  bool operator<=(const Timestamp& t) const {return GetSec() < t.GetSec() || (GetSec() == t.GetSec()  && GetMicrosec() <= t.GetMicrosec());}
  bool operator>(const Timestamp& t) const {return t < *this;}
  bool operator>=(const Timestamp& t) const {return t <= *this;}
  bool operator==(const Timestamp& t) const {return GetSec() == t.GetSec() && GetMicrosec() == t.GetMicrosec();}
  Timestamp& operator-=(const Timestamp& t)
  {
    m_microseconds -= t.m_microseconds;
    if (m_microseconds < 0)
    {
      m_microseconds += 1000000;
      m_seconds -= 1;
    }
    m_seconds -= t.m_seconds;
    return *this;
  }
  Timestamp operator-(const Timestamp& t) const
  { Timestamp out(*this); return std::move(out-=t); }
  friend std::ostream& operator<< (std::ostream& stream, const Timestamp& timestamp)
  {
    stream << timestamp.GetSec() <<"." << std::setw(6) << std::setfill('0')
      << std::right << timestamp.GetMicrosec();
  }
private:
  long m_seconds;
  long m_microseconds;
};
}
