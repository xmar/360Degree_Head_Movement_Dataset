/** @file
    @brief Example program that uses the OSVR direct-to-display interface
           and the OpenGL Core profile to render a scene that has lots
           of polygons.  This can be used to do speed tests on various
           cards, or regression tests on new versions.

    @date 2015

    @author
    Russ Taylor <russ@sensics.com>
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include <osvr/RenderKit/RenderManager.h>
#include <osvr/ClientKit/Context.h>

// Library/third-party includes
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "boost/program_options.hpp"
#include <boost/config.hpp>

#include <GL/glew.h>

// Standard includes
#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <chrono>
#include <stdlib.h> // For exit()

// This must come after we include <GL/gl.h> so its pointer types are defined.
#include <osvr/RenderKit/GraphicsLibraryOpenGL.h>
#include <osvr/RenderKit/RenderKitGraphicsTransforms.h>

//Internal Includes
#include "Mesh.hpp"
#include "ShaderTexture.hpp"
#include "ConfigParser.hpp"
#include "LogWriter.hpp"
#include "Quaternion.hpp"
#include "PublisherLogMQ.hpp"

using namespace IMT;

//static global variable
static std::shared_ptr<ShaderTexture> sampleShader(nullptr);
static std::shared_ptr<Mesh> roomMesh(nullptr);
static std::shared_ptr<LogWriter> logWriter(nullptr);
static std::shared_ptr<PublisherLogMQ> publisherLogMQ(nullptr);
static bool firstFrame = true;
constexpr std::chrono::system_clock::time_point zero(std::chrono::nanoseconds(0));
static std::chrono::system_clock::time_point global_startDisplayTime(zero);
static size_t lastDisplayedFrame(0);
static size_t lastNbDroppedFrame(0);
static bool started(false);


// Set to true when it is time for the application to quit.
// Handlers below that set it to true when the user causes
// any of a variety of events so that we shut down the system
// cleanly.  This only works on Windows.
static bool quit = false;

#ifdef _WIN32
// Note: On Windows, this runs in a different thread from
// the main application.
static BOOL CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
    // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
    // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        quit = true;
        return TRUE;
    default:
        return FALSE;
    }
}
#endif

// This callback sets a boolean value whose pointer is passed in to
// the state of the button that was pressed.  This lets the callback
// be used to handle any button press that just needs to update state.
void myButtonCallback(void* userdata, const OSVR_TimeValue* /*timestamp*/,
                      const OSVR_ButtonReport* report) {
    bool* result = static_cast<bool*>(userdata);
    *result = (report->state != 0);
}

bool SetupRendering(osvr::renderkit::GraphicsLibrary library) {
    // Make sure our pointers are filled in correctly.
    if (library.OpenGL == nullptr) {
        std::cerr << "SetupRendering: No OpenGL GraphicsLibrary, this should "
                     "not happen"
                  << std::endl;
        return false;
    }

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    // Turn on depth testing, so we get correct ordering.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    return true;
}

// Callback to set up a given display, which may have one or more eyes in it
void SetupDisplay(
    void* userData //< Passed into SetDisplayCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    ) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "SetupDisplay: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr
            << "SetupDisplay: No OpenGL RenderBuffer, this should not happen"
            << std::endl;
        return;
    }

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    // Clear the screen to black and clear depth
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.8f, 0, 0.1f, 1.0f);
}

// Callback to set up for rendering into a given eye (viewpoint and projection).
void SetupEye(
    void* userData //< Passed into SetViewProjectionCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    , osvr::renderkit::OSVR_ViewportDescription
        viewport //< Viewport set by RenderManager
    , osvr::renderkit::OSVR_ProjectionMatrix
        projection //< Projection matrix set by RenderManager
    , size_t whichEye //< Which eye are we setting up for?
    ) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "SetupEye: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr << "SetupEye: No OpenGL RenderBuffer, this should not happen"
                  << std::endl;
        return;
    }

    // Set the viewport
    glViewport(static_cast<GLint>(viewport.left),
      static_cast<GLint>(viewport.lower),
      static_cast<GLint>(viewport.width),
      static_cast<GLint>(viewport.height));
}

// Callbacks to draw things in world space.
void DrawWorld(
    void* userData //< Passed into AddRenderCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    , osvr::renderkit::OSVR_ViewportDescription
        viewport //< Viewport we're rendering into
    , OSVR_PoseState pose //< OSVR ModelView matrix set by RenderManager
    , osvr::renderkit::OSVR_ProjectionMatrix
        projection //< Projection matrix set by RenderManager
    , OSVR_TimeValue deadline //< When the frame should be sent to the screen
    )
{
  if(started)
  {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "DrawWorld: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr << "DrawWorld: No OpenGL RenderBuffer, this should not happen"
                  << std::endl;
        return;
    }

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    GLdouble projectionGL[16];
    osvr::renderkit::OSVR_Projection_to_OpenGL(projectionGL, projection);

    GLdouble viewGL[16];
    osvr::renderkit::OSVR_PoseState_to_OpenGL(viewGL, pose);

    std::chrono::system_clock::time_point deadlineTP(
                                        std::chrono::seconds{deadline.seconds} +
                                        std::chrono::microseconds{deadline.microseconds} );

    auto now = std::chrono::system_clock::now();
    if (global_startDisplayTime == zero)
    {
      global_startDisplayTime = now;// + std::chrono::milliseconds(5000);
      sampleShader->SetStartTime(global_startDisplayTime);
      firstFrame = false;
      logWriter->Start();
    }
    deadlineTP = std::chrono::system_clock::time_point(now - global_startDisplayTime);

    /// Draw a cube with a 5-meter radius as the room we are floating in.
    auto frameInfo = roomMesh->Draw(projectionGL, viewGL, sampleShader, std::move(deadlineTP));

    auto q = ToQuaternion(pose.rotation);
    Quaternion v(0, 1, 0, 0);
    Eigen::AngleAxisd angleAxisX(0.5*M_PI, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd angleAxisZ(0.5*M_PI, Eigen::Vector3d::UnitZ());
    Quaternion rotToOSVRX(angleAxisX);
    Quaternion rotToOSVRZ(angleAxisZ);
    Quaternion rotToOSVR = rotToOSVRZ*rotToOSVRX;
    Quaternion rot = rotToOSVR*q*rotToOSVR.conjugate();


    // if(frameInfo.m_timestamp > zero && firstFrame)
    // {
    //   firstFrame = false;
    //   logWriter->Start();
    // }
    lastDisplayedFrame = frameInfo.m_frameDisplayId;
    lastNbDroppedFrame += frameInfo.m_nbDroppedFrame;
    Log log(frameInfo.m_timestamp, frameInfo.m_pts, rot, frameInfo.m_frameDisplayId);
    std::stringstream ss;
    ss << log.GetQuaternion();
    if (frameInfo.m_frameDisplayId != size_t(-1))
    {
      publisherLogMQ->SendMessage(POSITION_INFO, ss.str());
      logWriter->AddLog(std::move(log));
    }

    if (frameInfo.m_last)
    {
      logWriter->Stop();
      quit = true;
    }
  }
}

void Usage(std::string name) {
  std::cerr << "Usage: " << name << " -c pathToConfigFile" << std::endl;
  std::cerr << "       pathToConfigFile: path to the ini configuration file" << std::endl;

  exit(-1);
}

int main(int argc, char* argv[]) {
    // Parse the command line
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
       ("help,h", "Produce this help message")
       ("config,c", po::value<std::string>(), "Path to the configuration file")
       ;

   po::variables_map vm;
   try
   {
      po::store(po::parse_command_line(argc, argv, desc),
            vm);

      //--help
      if ( vm.count("help") || !vm.count("config"))
      {
         Usage(argv[0]);
      }

      po::notify(vm);

      //Get the path to the configuration file
      std::string pathToIni = vm["config"].as<std::string>();

      //Create the configuration file parser
      ConfigParser configParser(pathToIni);
      configParser.Init();

      //Get the mesh and the shader generated from the configuration parser
      roomMesh = configParser.GetMesh();
      sampleShader = configParser.GetShaderTexture();
      logWriter = configParser.GetLogWriter();
      publisherLogMQ = configParser.GetPublisherLogMQ();

      // Get an OSVR client context to use to access the devices
      // that we need.
      osvr::clientkit::ClientContext context(
          "com.osvr.renderManager.openGLExample");

      // Open OpenGL and set up the context for rendering to
      // an HMD.  Do this using the OSVR RenderManager interface,
      // which maps to the nVidia or other vendor direct mode
      // to reduce the latency.
      std::shared_ptr<osvr::renderkit::RenderManager> render(
          osvr::renderkit::createRenderManager(context.get(), "OpenGL")
      );

      if ((render == nullptr) || (!render->doingOkay())) {
          std::cerr << "Could not create RenderManager" << std::endl;
          return 1;
      }

      // Set callback to handle setting up rendering in an eye
      render->SetViewProjectionCallback(SetupEye);

      // Set callback to handle setting up rendering in a display
      render->SetDisplayCallback(SetupDisplay);

      // Register callback to render things in world space.
      render->AddRenderCallback("/", DrawWorld);

  // Set up a handler to cause us to exit cleanly.
  #ifdef _WIN32
      SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
  #endif

      // Open the display and make sure this worked.
      osvr::renderkit::RenderManager::OpenResults ret = render->OpenDisplay();
      if (ret.status == osvr::renderkit::RenderManager::OpenStatus::FAILURE) {
          std::cerr << "Could not open display" << std::endl;
          return 2;
      }
      if (ret.library.OpenGL == nullptr) {
          std::cerr << "Attempted to run an OpenGL program with a config file "
                    << "that specified a different rendering library."
                    << std::endl;
          return 3;
      }

      // Set up the rendering state we need.
      if (!SetupRendering(ret.library)) {
          return 3;
      }

      glewExperimental = true;
      if (glewInit() != GLEW_OK) {
          std::cerr << "Failed ot initialize GLEW\n" << std::endl;
          return -1;
      }
      // Clear any GL error that Glew caused.  Apparently on Non-Windows
      // platforms, this can cause a spurious  error 1280.
      glGetError();

      sampleShader->InitAudio();

      //Sleep 5 seconds to give time for the decoder to start properly
      std::this_thread::sleep_for(std::chrono::seconds(1));
      global_startDisplayTime = zero;
      started = true;

      std::cout << "Start player the video\n";
      publisherLogMQ->SendMessage(APP_STATUS, "RUNNING");

      // Frame timing
      size_t countFrames = 0;
      size_t startDisplayedFrame = lastDisplayedFrame;
      auto startTime = std::chrono::system_clock::now();

      // Continue rendering until it is time to quit.
      while (!quit) {
          // Update the context so we get our callbacks called and
          // update tracker state.
          context.update();

          if (!render->Render()) {
              std::cerr
                  << "Render() returned false, maybe because it was asked to quit"
                  << std::endl;
              quit = true;
          }

          // Print timing info
          auto nowTime = std::chrono::system_clock::now();
          auto duration = nowTime-startTime;
          ++countFrames;
          constexpr std::chrono::seconds twoSeconds(2);
          if (duration >= twoSeconds)
          {
            std::string message = "Rendering at "
                + std::to_string(countFrames / std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1>>>(duration).count())
                + " fps | Video displayed at "
                + std::to_string((lastDisplayedFrame-startDisplayedFrame - lastNbDroppedFrame)/std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1>>>(duration).count())
                + " fps | nb droppped frame: "
                + std::to_string(lastNbDroppedFrame)
                ;
            std::cout << "\033[2K\r" << message << std::flush;
            publisherLogMQ->SendMessage(FPS_INFO, message);
            startTime = nowTime;
            startDisplayedFrame = lastDisplayedFrame;
            lastNbDroppedFrame = 0;
            countFrames = 0;
          }
      }

      publisherLogMQ->SendMessage(APP_STATUS, "DONE");

    }
    catch(const po::error& e)
    {
       std::cerr << "ERROR: " << e.what() << std::endl << std::endl
          << desc << std::endl;
       publisherLogMQ->SendMessage(APP_STATUS, "ERROR");
       return 1;
    }
    catch(std::exception& e)
    {
       std::cerr << "Uncatched exception: " << e.what() << std::endl
          << desc << std::endl;
       publisherLogMQ->SendMessage(APP_STATUS, "ERROR");
       return 1;

    }
    catch(...)
    {
       std::cerr << "Uncatched exception" << std::endl
         << desc << std::endl;
       publisherLogMQ->SendMessage(APP_STATUS, "ERROR");
       return 1;

    }

    return 0;
}
