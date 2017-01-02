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
  mod = ModPtr(new M1L2());
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
  while (true) {
    if (code == 0) {
      doCmd(proc);
      proc = CmdPtr(new Cmd(shared_from_this()));
      continue;
    }
    if (code == 100) {
      continue;
    }
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
// the M1L2 process imple
int M1L2::process(asio::streambuf &buf, CmdPtr cmd) {
  size_t readed;
  if (length < 1) {
    if (buf.size() < 3) {
      return 100;
    }
    readed = buf.sgetn(cbuf, 3);
    if (readed < 3) {
      return -1;
    }
    if (cbuf[0] != mod) {
      return -1;
    }
    if (big) {
      length = endian::detail::load_big_endian<uint16_t, 16>(cbuf + 1);
    } else {
      length = endian::detail::load_little_endian<uint16_t, 16>(cbuf + 1);
    }
  }
  if (buf.size() < length) {
    return 100;
  }
  cmd->data = DataPtr(new char[length]);
  readed = buf.sgetn(cmd->data.get(), length);
  if (readed < length) {
    return -1;
  }
  length = 0;
  return 0;
}
}
}
}
