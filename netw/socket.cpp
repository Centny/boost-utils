//
//  socket.cpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#include "socket.hpp"
#include "../log/log.hpp"
namespace butils {
namespace netw {

// basic socket constructor
BasicSocket::BasicSocket(asio::io_service &ios, CmdHPtr cmd, ConHPtr con) : ios(ios), cmd(cmd), con(con) {
    mod = ModHPtr(new M1L2());
}

// readed handler
void BasicSocket::readed(const system::error_code &err) {
    if (!err) {
        V_LOG_E("BasicSocket readed error, will close->%s", err.message().c_str());
        con->OnClose(share(), err);
        close();
        return;
    }
    if (proc.get() == 0) {
        proc = CmdPtr(new Cmd(shared_from_this()));
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
BasicSocketPtr BasicSocket::share() { return shared_from_this(); }

// execute command
int BasicSocket::doCmd(CmdPtr m) { return cmd->OnCmd(m); }

// the command constructor
Cmd::Cmd(BasicSocketPtr socket) : socket(socket) {}

// return the command char data.
const char *Cmd::cdata() { return data.get(); }

// the basic acceptor sequence.
uint64_t BasicAcceptor::sequence = 0;

// the base acceptor constructor
BasicAcceptor::BasicAcceptor(asio::io_service &ios, CmdHPtr cmd, ConHPtr con) : ios(ios), cmd(cmd), con(con) {}

BasicAcceptor::~BasicAcceptor() {}
// the accepted handler
void BasicAcceptor::accepted(BasicSocketPtr s, const boost::system::error_code &err) {
    if (!err) {
        V_LOG_E("TcpServer accept error, will stopped->%s", err.message().c_str());
        close();
        return;
    }
    if (con->OnConn(s, err)) {
        s->Id = sequence++;
        V_LOG_I("TcpServer accepted from %s", s->address().c_str());
    } else {
        V_LOG_E("TcpServer accept refuse from %s", s->address().c_str());
        s->close();
    }
    accept();
}
BasicAcceptorPtr BasicAcceptor::share() { return shared_from_this(); }
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
