//
//  socket.hpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#ifndef socket_hpp
#define socket_hpp
#include "../log/log.hpp"
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>

namespace boost {
namespace utils {
namespace netw {
using namespace boost::asio::ip;

class Cmd;
class CmdH;
class ConH;
class ModH;
class BasicSocket;
typedef boost::shared_ptr<uint8_t *> DataPtr;
typedef boost::shared_ptr<Cmd> CmdPtr;
typedef boost::shared_ptr<ModH> ModPtr;
typedef boost::shared_ptr<BasicSocket> BasicSocketPtr;

// the mod handler for socket frame
class ModH {
public:
  // 0:complete one.
  // 100:nead more data.
  //-1:error.
  virtual int process(asio::streambuf &buf, CmdPtr cmd) = 0;
};

// the cmd handler for execute frame.
class CmdH {
public:
  virtual int OnCmd(CmdPtr c);
};

// the connect handler for socket event.
class ConH {
public:
  // event on socket connected.
  virtual bool OnConn(BasicSocketPtr s);
  // event on socket close.
  virtual void OnClose(BasicSocketPtr s);
};

// the socket frame data.
class Cmd {
public:
  BasicSocketPtr socket;
  DataPtr data;

public:
  Cmd(BasicSocketPtr socket);
};

// the basic socket to process readed
class BasicSocket : public boost::enable_shared_from_this<BasicSocket> {
public:
  CmdH &cmd;
  ConH &con;
  asio::io_service &ios;
  asio::streambuf buf;
  ModPtr mod;
  CmdPtr proc;

public:
  BasicSocket(asio::io_service &ios, CmdH &h, ConH &con);
  virtual void readed(const system::error_code &err);
  virtual int doCmd(CmdPtr m);
  virtual void read() = 0;
  virtual void close() = 0;
  virtual const char *string() = 0;
};

// the basic acceptor
class BasicAcceptor {
public:
  CmdH &cmd;
  ConH &con;
  asio::io_service &ios;

public:
  BasicAcceptor(asio::io_service &ios, CmdH &h, ConH &con);
  void accepted(BasicSocketPtr s, const boost::system::error_code &err);
  virtual void accept() = 0;
  virtual void close() = 0;
};

// the template socke for tcp/udp
template <typename T> class Socket : BasicSocket {
public:
  asio::basic_stream_socket<T> socket;

public:
  Socket(asio::io_service &ios, CmdH &cmd, ConH &con)
      : BasicSocket(ios, cmd, con), socket(ios) {}
  void start() { read(); }
  virtual void read() {
    asio::async_read(socket, buf, bind(&Socket::readed, shared_from_this(),
                                       asio::placeholders::error));
  }
  virtual void close() { socket.close(); }
  virtual const char *string() { return "remote"; }
};

// the template acceptor for tcp/udp
template <typename T> class Acceptor : BasicAcceptor {
public:
  asio::basic_socket_acceptor<T> acceptor;

public:
  Acceptor(asio::io_service &ios, const tcp::endpoint &endpoint, CmdH &cmd,
           ConH &con)
      : BasicAcceptor(ios, cmd, con), acceptor(ios, endpoint) {}

  void start() { accept()(); }
  virtual void accept() {
    Socket<T> s = new Socket<T>(ios, cmd, con);
    BasicSocketPtr bs(s);
    acceptor.async_accept(s->socket, boost::bind(&Acceptor::accepted, this, bs,
                                                 asio::placeholders::error));
  }
  virtual void close() { acceptor.close(); }
};

}
}
}
#endif /* socket_hpp */
