//
//  socket.hpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#ifndef socket_hpp
#define socket_hpp
#include <stdio.h>
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include "../log/log.hpp"

namespace butils {
namespace netw {
using namespace boost::asio::ip;
using namespace boost;

class Cmd;
class CmdH;
class ConH;
class ModH;
class BasicSocket;
class BasicAcceptor;
typedef boost::shared_ptr<char> DataPtr;
typedef boost::shared_ptr<Cmd> CmdPtr;
typedef boost::shared_ptr<CmdH> CmdHPtr;
typedef boost::shared_ptr<ConH> ConHPtr;
typedef boost::shared_ptr<ModH> ModHPtr;
typedef boost::shared_ptr<BasicSocket> BasicSocketPtr;
typedef boost::shared_ptr<BasicAcceptor> BasicAcceptorPtr;

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
    virtual int OnCmd(CmdPtr c) = 0;
};

// the connect handler for socket event.
class ConH {
   public:
    // event on socket connected.
    virtual bool OnConn(BasicSocketPtr s, const boost::system::error_code &ec) = 0;
    // event on socket close.
    virtual void OnClose(BasicSocketPtr s, const boost::system::error_code &ec) = 0;
};

// the socket frame data.
class Cmd {
   public:
    BasicSocketPtr socket;
    DataPtr data;

   public:
    Cmd(BasicSocketPtr socket);
    const char *cdata();
};

// the basic socket to process readed
class BasicSocket : public boost::enable_shared_from_this<BasicSocket> {
   public:
    uint64_t Id;
    CmdHPtr cmd;
    ConHPtr con;
    asio::io_service &ios;
    asio::streambuf buf;
    ModHPtr mod;
    CmdPtr proc;

   public:
    BasicSocket(asio::io_service &ios, CmdHPtr cmd, ConHPtr con);
    virtual void readed(const system::error_code &err);
    virtual int doCmd(CmdPtr m);
    virtual void read() = 0;
    virtual void close() = 0;
    virtual std::string address() = 0;
    virtual size_t write(const char *data, size_t len, boost::system::error_code &ec) = 0;
    virtual size_t write(asio::streambuf &buf, boost::system::error_code &ec) = 0;
    virtual BasicSocketPtr share();
};

// the basic acceptor
class BasicAcceptor : public boost::enable_shared_from_this<BasicAcceptor> {
   private:
    static uint64_t sequence;

   public:
    CmdHPtr cmd;
    ConHPtr con;
    asio::io_service &ios;

   public:
    BasicAcceptor(asio::io_service &ios, CmdHPtr cmd, ConHPtr con);
    virtual ~BasicAcceptor();
    virtual void accepted(BasicSocketPtr s, const boost::system::error_code &err);
    virtual void accept() = 0;
    virtual void close() = 0;
    virtual BasicAcceptorPtr share();
};

// the template socke for tcp/udp
template <typename T>
class Socket : public BasicSocket {
   public:
    std::string localAddr;
    unsigned short localPort;
    asio::basic_stream_socket<T> sck;

   public:
    Socket(asio::io_service &ios, CmdHPtr cmd, ConHPtr con) : BasicSocket(ios, cmd, con), sck(ios) {}
    virtual void start() { read(); }
    virtual void read() {
        asio::async_read(sck, buf, bind(&BasicSocket::readed, this->share(), asio::placeholders::error));
    }
    virtual void close() { sck.close(); }
    virtual std::string address() {
        boost::system::error_code ec;
        asio::ip::basic_endpoint<T> ep = sck.remote_endpoint(ec);
        if (ec) {
            return ec.message();
        } else {
            return ep.address().to_string();
        }
    }
    virtual size_t write(const char *data, size_t len, boost::system::error_code &ec) {
        return boost::asio::write(sck, boost::asio::buffer(data, len), ec);
    }
    virtual size_t write(asio::streambuf &buf, boost::system::error_code &ec) {
        return boost::asio::write(sck, buf, ec);
    }
};

// the SocketPtr
template <typename T>
using SocketPtr = boost::shared_ptr<Socket<T> >;

// the template connector for tcp/udp
template <typename T>
class Connector : public Socket<T> {
   public:
    asio::ip::basic_endpoint<T> local;

   public:
    Connector(asio::io_service &ios, CmdHPtr cmd, ConHPtr con) : Socket<T>(ios, cmd, con) {}
    virtual void connect(const boost::asio::ip::address &addr, unsigned short port) {
        connect(boost::asio::ip::basic_endpoint<T>(addr, port));
    }
    virtual void connect(const char *addr, unsigned short port, boost::system::error_code &err) {
        connect(boost::asio::ip::address::from_string(addr, err), port);
    }
    virtual void connect(boost::asio::ip::basic_endpoint<T> remote) {
        this->sck.open(local.protocol());
        this->sck.set_option(tcp::no_delay(true));
        if (local.port()) {
            this->sck.set_option(asio::socket_base::reuse_address(true));
            this->sck.bind(local);
        }
        this->sck.async_connect(remote, boost::bind(&Connector::connected, this, boost::asio::placeholders::error));
    }
    virtual void connected(const boost::system::error_code &err) {
        if (err) {
            this->con->OnConn(this->share(), err);
            return;
        }
        boost::system::error_code ec;
        asio::ip::basic_endpoint<T> ep = this->sck.remote_endpoint(ec);
        if (ec) {
            this->con->OnConn(this->share(), ec);
            return;
        }
        //    this->localAddr=ep.address().to_string();
        //    this->localPort=ep.port;
        this->con->OnConn(this->share(), err);
        this->start();
    }
};

// the ConnectorPtr
template <typename T>
using ConnectorPtr = boost::shared_ptr<Connector<T> >;

// the template acceptor for tcp/udp
template <typename T>
class Acceptor : public BasicAcceptor {
   public:
    asio::basic_socket_acceptor<T> acceptor;
    basic_endpoint<T> endpoint;
    bool reused;

   public:
    Acceptor(asio::io_service &ios, const basic_endpoint<T> &endpoint, CmdHPtr cmd, ConHPtr con)
        : BasicAcceptor(ios, cmd, con), acceptor(ios) {
        this->endpoint = endpoint;
    }
    Acceptor(asio::io_service &ios, const boost::asio::ip::address &addr, unsigned short port, CmdHPtr cmd, ConHPtr con)
        : BasicAcceptor(ios, cmd, con), acceptor(ios) {
        this->endpoint = basic_endpoint<T>(addr, port);
    }
    Acceptor(asio::io_service &ios, const char *addr, unsigned short port, CmdHPtr cmd, ConHPtr con)
        : BasicAcceptor(ios, cmd, con), acceptor(ios) {
        this->endpoint = basic_endpoint<T>(asio::ip::address::from_string(addr), port);
    }

    virtual void start(boost::system::error_code &ec) {
        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::socket_base::reuse_address(true));
        acceptor.bind(endpoint, ec);
        if (ec) {
            return;
        }
        acceptor.listen();
        accept();
    }
    virtual void accept() {
        BasicSocketPtr basic(new Socket<T>(ios, cmd, con));
        Socket<T> *sck = (Socket<T> *)basic.get();
        acceptor.async_accept(sck->sck,
                              boost::bind(&Acceptor::accepted, this->share(), basic, asio::placeholders::error));
    }
    virtual void close() { acceptor.close(); }
};

// the AcceptorPtr
template <typename T>
using AcceptorPtr = boost::shared_ptr<Acceptor<T> >;

// the mod impl by 1 byte mod and 2 byte data length
class M1L2 : public ModH {
   private:
    char cbuf[5];
    uint16_t length = 0;

   public:
    uint8_t mod = 0x08;
    bool big = true;
    virtual int process(asio::streambuf &buf, CmdPtr cmd);
};
}
}
#endif /* socket_hpp */
