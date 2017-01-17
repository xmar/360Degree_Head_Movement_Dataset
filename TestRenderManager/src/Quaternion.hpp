/**
 Author: Xavier Corbillon
 IMT Atlantique

 This class manage a head position Log
*/
#pragma once

//library includes
#include <Eigen/Geometry>
#include <osvr/Util/EigenInterop.h>

namespace IMT {
  typedef Eigen::Quaterniond Quaternion;

  inline Quaternion ToQuaternion(const OSVR_Quaternion& quat) {return osvr::util::fromQuat(quat);}

  inline std::ostream& operator<< (std::ostream& stream, const Quaternion& quat)
  {
    stream << quat.w() << " " << quat.x() << " " << quat.y() << " " << quat.z();
  }
}
