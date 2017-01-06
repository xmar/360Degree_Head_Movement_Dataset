/**
  Author: Xavier Corbillon
  IMT-Atlantique

  This software intends to log the position of a Head-Mounted Device using the OSVR layer.
**/

//internal includes
#include "Quaternion.hpp"
#include "LogWriter.hpp"

// library includes
#include <osvr/ClientKit/ClientKit.h>

// Standard includes
#include <iostream>
#include <iomanip>
#include <memory>

using namespace IMT;

class OrientationLogerManager
{
public:
  static void SetManager(std::string logFolder, std::string logId, osvr::clientkit::Interface& interface)
  {
    if (!orientationLogerManageExist)
    {
        logWriter = LogWriter(logFolder, logId);
        interface.registerCallback(OrientationLogerManager::OrientationCallback, nullptr);
        orientationLogerManageExist = true;
    }
  }
  static void Start(void) {logWriter.Start();}
  static void Stop(void) {logWriter.Stop();}
private:
  OrientationLogerManager(void) {}

  static LogWriter logWriter;
  static bool orientationLogerManageExist;
  static void OrientationCallback(void* userdata,
              const OSVR_TimeValue* timestamp,
              const OSVR_OrientationReport *report)
  {
    auto q = ToQuaternion(report->rotation);
    Quaternion v(0, 1, 0, 0);
    Eigen::AngleAxisd angleAxisX(0.5*M_PI, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd angleAxisZ(0.5*M_PI, Eigen::Vector3d::UnitZ());
    Quaternion rotToOSVRX(angleAxisX);
    Quaternion rotToOSVRZ(angleAxisZ);
    Quaternion rotToOSVR = rotToOSVRZ*rotToOSVRX;
    Quaternion rot = rotToOSVR*q*rotToOSVR.conjugate();
    auto euler = rot.toRotationMatrix().eulerAngles(2, 1, 0);
    auto v_new = rot*v*rot.conjugate();
    v_new.z() *= -1;
    std::cout << "\033[2K\r" << timestamp->seconds <<"." << std::setw(6) << std::setfill('0') << std::left
      << timestamp->microseconds << " Direction: ("<< v_new.x() <<", " << v_new.y() <<", " << v_new.z() << "); Euler ("<< euler[0]*180.0/M_PI <<", " << euler[1]*180.0/M_PI <<", " << euler[2]*180.0/M_PI << ")" << std::flush;
    logWriter.AddLog(Log(*timestamp,rot));
  }
};
LogWriter OrientationLogerManager::logWriter("","");
bool OrientationLogerManager::orientationLogerManageExist = false;

void Clean(void)
{
  OrientationLogerManager::Stop();
}

int main(int argc, char** argv)
{
  std::cout << "Starting" << std::endl;

  auto ctx = std::make_shared<osvr::clientkit::ClientContext>("OSVRClientTracker");

  if (!ctx->checkStatus())
  {
    std::cerr << "OSVR client context did not start correctly. "<<
      "Please make sure the OSVR server is running." << std::endl;
      exit(1);
  }

  auto interface = ctx->getInterface("/me/head");

  std::cout << "OSVR clientKit connection established." << std::endl;

  OrientationLogerManager::SetManager("logs", "test", interface);

  std::cout << "Callback registered" << std::endl;


  atexit(Clean);
  OrientationLogerManager::Start();
  while(true) {ctx->update();}

  return 0;
}
