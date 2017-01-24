/**
Author: Xavier Corbillon
IMT Atlantique

The purpure of this class is to publish messages to other subscriber application
using zeroMQ
**/

#include <zmq.hpp>

#include <iostream>

namespace IMT
{

  constexpr const char* FPS_INFO = "FPS_INFO";
  constexpr const char* POSITION_INFO = "POSITION_INFO";
  constexpr const char* APP_STATUS = "APP_STATUS";

class PublisherLogMQ
{
public:
  PublisherLogMQ(void): m_zmqContext(1), m_zmqPublisher(m_zmqContext, ZMQ_PUB) {}
  ~PublisherLogMQ(void) = default;

  void Init(size_t port) {m_zmqPublisher.bind(("tcp://*:"+std::to_string(port)).c_str());}

  void SendMessage(std::string label, std::string message)
  {
    std::string m = label + ": " + message;
    zmq::message_t zmqMessage(m.size());
    memcpy (zmqMessage.data(), m.data(), m.size());
    m_zmqPublisher.send (zmqMessage);
  }
private:
  zmq::context_t m_zmqContext;
  zmq::socket_t m_zmqPublisher;
};

}
