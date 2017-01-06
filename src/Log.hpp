/**
 Author: Xavier Corbillon
 IMT Atlantique

 This class manage a head position Log
*/
#pragma once

//internal includes
#include "Quaternion.hpp"
#include "Timestamp.hpp"

namespace IMT {

class Log
{
public:
  Log(Timestamp t, Quaternion q): m_t(t), m_q(q) {};

  const Timestamp& GetTimestamp(void) const {return m_t;};
  const Quaternion& GetQuat(void) const {return m_q;};
  friend std::ostream& operator<< (std::ostream& stream, const Log& log)
  {
    stream << log.m_t << " " << log.m_q;
  }
  Log operator-(const Timestamp& t) const
  {
    Log out = *this;
    out.m_t -= t;
    return out;
  }
private:
  Timestamp m_t;
  Quaternion m_q;
};
}
