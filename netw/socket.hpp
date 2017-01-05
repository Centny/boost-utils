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
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include "../log/log.hpp"

namespace butils {
namespace netw {
using namespace boost::asio::ip;
using namespace boost;

class CmdH_;
class ConH_;
class ModH_;
typedef boost::shared_ptr<CmdH_> CmdH;
typedef boost::shared_ptr<ConH_> ConH;
typedef boost::shared_ptr<ModH_> ModH;
//
using ntcp = boost::asio::ip::tcp;
using nudp = boost::asio::ip::udp;
class Cmd_;
using Cmd = boost::shared_ptr<Cmd_>;
class Data_;
using Data = boost::shared_ptr<Data_>;
class Writer_;
using Writer = boost::shared_ptr<Writer_>;
class UDP_;
using UDP = boost::shared_ptr<UDP_>;
class Monitor_;
using Monitor = boost::shared_ptr<Monitor_>;
class TCP_;
using TCP = boost::shared_ptr<TCP_>;
class Acceptor_;
using Acceptor = boost::shared_ptr<Acceptor_>;
class Connector_;
using Connector = boost::shared_ptr<Connector_>;

// the mod handler for socket frame
class ModH_ {
   public:
    virtual void full(char *buf, size_t len) = 0;
    virtual size_t header() = 0;
    // 0:complete one.
    // 100:nead more data.
    //-1:error.
    virtual size_t parse(const char *buf) = 0;
};

// the cmd handler for execute frame.
class CmdH_ {
   public:
    virtual int OnCmd(Cmd c) = 0;
};

// the connect handler for socket event.
class ConH_ {
   public:
    // event on socket connected.
    virtual bool OnConn(TCP s, const boost::system::error_code &ec) = 0;
    // event on socket close.
    virtual void OnClose(TCP s, const boost::system::error_code &ec) = 0;
};

class Data_ : public boost::enable_shared_from_this<Data_> {
   public:
    char *data;
    size_t len;

   public:
    Data_(const char *buf, size_t len);
    ~Data_();
    Data share();
    char operator[](size_t i);
};
    Data BuildData(const char* buf, size_t len);

class Writer_ : public boost::enable_shared_from_this<Writer_> {
   protected:
    uint64_t Id_;

   public:
    Writer_();
    virtual uint64_t Id();
    virtual size_t write(Data data, boost::system::error_code &ec);
    virtual size_t write(const char *data, size_t len, boost::system::error_code &ec) = 0;
    virtual size_t write(asio::streambuf &buf, boost::system::error_code &ec) = 0;
    virtual void close() = 0;
    virtual std::string address() = 0;
    Writer share();
};

// the socket frame data.
class Cmd_ {
   public:
    Writer writer;
    size_t offset;
    size_t length;
    Data data;

   protected:
    Cmd_();

   public:
    Cmd_(Writer writer, const char *buf, size_t len);
    ~Cmd_();
    virtual const char *cdata();
    virtual size_t clength();
    virtual Cmd slice(size_t offset, size_t len = 0);
};

template <typename T>
std::string sck_address(asio::basic_stream_socket<T> &sck) {
    boost::system::error_code ec;
    asio::ip::basic_endpoint<T> ep = sck.remote_endpoint(ec);
    char buf[32];
    if (ec) {
        buf[sprintf(buf, "error(%d)", ec.value())] = 0;
    } else {
        buf[sprintf(buf, "%s:%d", ep.address().to_string().c_str(), ep.port())] = 0;
    }
    return std::string(buf);
}

template <typename T>
std::string ep_address(boost::asio::ip::basic_endpoint<T> &ep) {
    char buf[32];
    buf[sprintf(buf, "%s:%d", ep.address().to_string().c_str(), ep.port())] = 0;
    return std::string(buf);
}

class UDP_ : public Writer_ {
   public:
    asio::io_service &ios;
    Monitor M;
    basic_endpoint<nudp> remote;

   public:
    UDP_(asio::io_service &ios, Monitor m, basic_endpoint<nudp> &remote);
    virtual ~UDP_();
    virtual void close();
    virtual std::string address();
    virtual size_t write(const char *data, size_t len, boost::system::error_code &ec);
    virtual size_t write(asio::streambuf &buf, boost::system::error_code &ec);
    UDP share();
};

class Monitor_ : public boost::enable_shared_from_this<Monitor_> {
   protected:
    char cbuf[1024];
    basic_endpoint<nudp> remote;

   public:
    CmdH cmd;
    ModH mod;
    asio::io_service &ios;
    basic_endpoint<nudp> endpoint;
    asio::basic_datagram_socket<nudp> sck;

   protected:
    virtual void received(const boost::system::error_code &err, size_t transferred);
    virtual void receive();
    virtual void bind(basic_endpoint<nudp> ep, boost::system::error_code &ec);

   public:
    Monitor_(asio::io_service &ios, CmdH cmd);
    Monitor_(asio::io_service &ios, basic_endpoint<nudp> ep, CmdH cmd);
    Monitor_(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd);
    virtual ~Monitor_();
    virtual void close();
    virtual void start(boost::system::error_code &ec);
    virtual size_t write(basic_endpoint<nudp> remote, const char *data, size_t len, boost::system::error_code &ec);
    virtual size_t write(const char *addr, unsigned short port, const char *data, size_t len,
                         boost::system::error_code &ec);
    virtual size_t write(basic_endpoint<nudp> remote, asio::streambuf &buf, boost::system::error_code &ec);
    virtual size_t write(const char *addr, unsigned short port, asio::streambuf &buf, boost::system::error_code &ec);
    Monitor share();
};
Monitor BuildMonitor(asio::io_service &ios, CmdH cmd);
Monitor BuildMonitor(asio::io_service &ios, basic_endpoint<nudp> ep, CmdH cmd);
Monitor BuildMonitor(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd);

// the basic socket to process readed
class TCP_ : public Writer_ {
   private:
    size_t frame;

   public:
    CmdH cmd;
    ConH con;
    ModH mod;
    char cbuf[102400];
    asio::io_service &ios;
    asio::basic_stream_socket<ntcp> sck;

   protected:
    virtual int doCmd(Cmd m);
    virtual size_t writeHeader(size_t len, boost::system::error_code &ec);
    virtual void readed(const system::error_code &err, size_t transferred);
    virtual void read(size_t exp);

   public:
    virtual void start();
    TCP_(asio::io_service &ios, CmdH cmd, ConH con);
    virtual ~TCP_();
    virtual void close();
    virtual std::string address();
    virtual void bind(basic_endpoint<ntcp> ep, boost::system::error_code &ec);
    virtual size_t write(Data data, boost::system::error_code &ec);
    virtual size_t write(const char *data, size_t len, boost::system::error_code &ec);
    virtual size_t write(asio::streambuf &buf, boost::system::error_code &ec);
    TCP share();
};

// the template connector for tcp/udp
class Connector_ : public TCP_ {
   public:
    asio::ip::basic_endpoint<ntcp> local;

   protected:
    virtual void connected(const boost::system::error_code &err);

   public:
    Connector_(asio::io_service &ios, CmdH cmd, ConH con);
    virtual void connect(const asio::ip::address &addr, unsigned short port, boost::system::error_code &ec);
    virtual void connect(const char *addr, unsigned short port, boost::system::error_code &err);
    virtual void connect(asio::ip::basic_endpoint<ntcp> remote, boost::system::error_code &ec);
    Connector share();
};
Connector BuildConnector(asio::io_service &ios, CmdH cmd, ConH con);

// the template acceptor for tcp/udp
// template <typename T>
class Acceptor_ : public boost::enable_shared_from_this<Acceptor_> {
   public:
    bool reused;
    CmdH cmd;
    ConH con;
    ModH mod;
    asio::io_service &ios;
    basic_endpoint<ntcp> endpoint;
    asio::basic_socket_acceptor<ntcp> act;

   protected:
    virtual void accepted(TCP s, const boost::system::error_code &err);
    virtual void accept();
    virtual void bind(basic_endpoint<ntcp> &ep, boost::system::error_code &ec);
    virtual ModH smod(TCP s);

   public:
    Acceptor_(asio::io_service &ios, const basic_endpoint<asio::ip::tcp> &endpoint, CmdH cmd, ConH con);
    Acceptor_(asio::io_service &ios, const boost::asio::ip::address &addr, unsigned short port, CmdH cmd, ConH con);
    Acceptor_(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd, ConH con);
    Acceptor share();
    virtual void start(boost::system::error_code &ec);
    virtual void close();
};
Acceptor BuildAcceptor(asio::io_service &ios, const basic_endpoint<ntcp> &endpoint, CmdH cmd, ConH con);
Acceptor BuildAcceptor(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd, ConH con);

// the mod impl by 1 byte mod and 2 byte data length
template <typename T, std::size_t bits>
class M1LX : public ModH_ {
   public:
    uint8_t magic = 0x08;
    bool big = true;

   public:
    M1LX(uint8_t magic = 0x08, bool big = true) : magic(magic), big(big) {}
    ~M1LX() {}
    virtual void full(char *buf, size_t len) {
        buf[0] = magic;
        if (big) {
            boost::endian::detail::store_big_endian<T, bits>(buf + 1, (T)len);
        } else {
            boost::endian::detail::store_little_endian<T, bits>(buf + 1, (T)len);
        }
    }

    virtual size_t header() { return bits+1; }
    virtual size_t parse(const char *buf) {
        if (buf[0] != magic) {
            return 0;
        }
        // printf("%d,%d\n", buf[1], buf[2]);
        if (big) {
            return (size_t)boost::endian::detail::load_big_endian<T, bits>(buf + 1);
        } else {
            return (size_t)boost::endian::detail::load_little_endian<T, bits>(buf + 1);
        }
    }
};
typedef M1LX<uint16_t, 2> M1L2;
typedef M1LX<uint32_t, 4> M1L4;
}
}
#endif /* socket_hpp */
