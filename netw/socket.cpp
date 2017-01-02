//
//  socket.cpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#include "socket.hpp"
#include "../log/log.hpp"
namespace boost {
namespace utils {
namespace netw {

// basic socket constructor
BasicSocket::BasicSocket(asio::io_service &ios, CmdH &h, ConH &con)
    : ios(ios), cmd(cmd), con(con) {
  proc = CmdPtr(new Cmd(shared_from_this()));
}

// readed handler
void BasicSocket::readed(const system::error_code &err) {
  if (!err) {
    V_LOG_E("BasicSocket readed error, will close->%s", err.message().c_str());
    con.OnConn(shared_from_this());
    close();
    return;
  }
  int code = mod->process(buf, proc);
  switch (code) {
  case 0: // on command.
    doCmd(proc);
    proc = CmdPtr(new Cmd(shared_from_this()));
    break;
  case 100: // continue read
    break;
  case -1:
    close();
    break;
  default:
    close();
    return;
  }
  read();
}

// execute command
int BasicSocket::doCmd(CmdPtr m) { return cmd.OnCmd(m); }

// the command constructor
Cmd::Cmd(BasicSocketPtr socket) : socket(socket) {}

// the base acceptor constructor
BasicAcceptor::BasicAcceptor(asio::io_service &ios, CmdH &h, ConH &con)
    : ios(ios), cmd(cmd), con(con) {}
// the accepted handler
void BasicAcceptor::accepted(BasicSocketPtr s,
                             const boost::system::error_code &err) {
  if (!err) {
    V_LOG_E("TcpServer accept error, will stopped->%s", err.message().c_str());
    close();
    return;
  }
  if (con.OnConn(s)) {
    V_LOG_I("TcpServer accepted from %s", s->string());
  } else {
    V_LOG_E("TcpServer accept refuse from %s", s->string());
    s->close();
  }
  accept();
}
//
}
}
}
