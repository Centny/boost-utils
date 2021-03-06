//
//  socket.cpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#include "socket.hpp"
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "../log/log.hpp"
namespace butils {
namespace netw {

// the basic acceptor sequence.
static uint64_t sequence = 0;
static boost::shared_mutex sequence_lck;

Writer_::Writer_() {
    sequence_lck.lock();
    Id_ = sequence++;
    sequence_lck.unlock();
}

uint64_t Writer_::Id() { return Id_; }

size_t Writer_::write(Data data, boost::system::error_code &ec) { return write(data->data, data->len, ec); }

Writer Writer_::share() { return shared_from_this(); }

Cmd_::Cmd_() {}

Cmd_::Cmd_(Writer writer, const char *buf, size_t len, Data header) : writer(writer), data(new Data_(buf, len)) {
    this->offset = 0;
    this->length = len;
    this->header = header;
}

Cmd_::~Cmd_() {}

const char *Cmd_::cdata() { return data->data + offset; }

size_t Cmd_::clength() { return length; }

Cmd Cmd_::slice(size_t offset, size_t len) {
    auto cmd = Cmd(new Cmd_);
    cmd->data = data;
    cmd->writer = writer;
    cmd->offset = offset;
    if (len < 1) {
        cmd->length = length - offset;
    } else {
        cmd->length = len;
    }
    return cmd;
}

char Cmd_::charAt(size_t idx) { return data->data[idx]; }

uint64_t Cmd_::Id() { return writer->Id(); }

TCP_::TCP_(asio::io_service &ios, CmdH cmd, ConH con) : ios(ios), sck(ios), cmd(cmd), con(con) {
    mod = ModH(new M1L2());
}

TCP_::~TCP_() { V_LOG_FREE("BasicSocket free client %d", this->Id()); }

void TCP_::close() { sck.close(); }

std::string TCP_::address() { return sck_address<ntcp>(sck); }

void TCP_::bind(boost::asio::ip::basic_endpoint<ntcp> ep, boost::system::error_code &ec) {
    sck.open(ep.protocol());
    sck.set_option(asio::socket_base::reuse_address(true));
    sck.bind(ep, ec);
}

size_t TCP_::write(Data data, boost::system::error_code &ec) { return Writer_::write(data, ec); }

void TCP_::read(size_t exp) {
    auto cback = boost::bind(&TCP_::readed, share(), asio::placeholders::error, asio::placeholders::bytes_transferred);
    asio::async_read(sck, boost::asio::buffer(cbuf, exp), cback);
}

void TCP_::readed(const system::error_code &err, size_t transferred) {
    auto bs = share();
    if (err) {
        V_LOG_E("BasicSocket(%llu,%x) readed error(%d), will close", this->Id(), tag, err.value());
        con->OnClose(bs, err);
        close();
        return;
    }
    if (frame < 1) {
        size_t hs = mod->header();
        if (transferred < hs) {
            V_LOG_E("BasicSocket readed invalid frame header length(%d), will close", transferred);
            con->OnClose(bs, err);
            close();
            return;
        }
        frame = mod->parse(cbuf);
        if (frame < 1) {
            V_LOG_E("%s", "BasicSocket readed invalid frame header mod, will close");
            con->OnClose(bs, err);
            close();
            return;
        }
        header = BuildData(cbuf, hs);
        read(frame);
        return;
    }
    if (frame != transferred) {
        V_LOG_E("BasicSocket readed invalid frame data length(%d),%d expected, will close", transferred, frame);
        con->OnClose(bs, err);
        close();
        return;
    }
    cmd->OnCmd(Cmd(new Cmd_(share(), cbuf, transferred, header)));
    frame = 0;
    read(mod->header());
}

size_t TCP_::writeHeader(size_t len, boost::system::error_code &ec) {
    size_t hlen = mod->header();
    char hbuf[hlen];
    mod->full(hbuf, len);
    return boost::asio::write(sck, boost::asio::buffer(hbuf, hlen), ec);
}

size_t TCP_::write(const char *data, size_t len, boost::system::error_code &ec) {
    writeHeader(len, ec);
    if (ec) {
        return 0;
    }
    return boost::asio::write(sck, boost::asio::buffer(data, len), ec);
}

size_t TCP_::write(asio::streambuf &buf, boost::system::error_code &ec) {
    writeHeader(buf.size(), ec);
    if (ec) {
        return 0;
    }
    return boost::asio::write(sck, buf, ec);
}

int TCP_::doCmd(Cmd m) { return cmd->OnCmd(m); }

void TCP_::start() { read(mod->header()); }

TCP TCP_::share() { return boost::dynamic_pointer_cast<TCP_>(Writer_::share()); }

UDP_::UDP_(asio::io_service &ios, Monitor m, basic_endpoint<nudp> remote) : ios(ios), M(m), remote(remote) {
    sequence_lck.lock();
    Id_ = sequence++;
    sequence_lck.unlock();
}

UDP_::~UDP_() {}

void UDP_::close() {}

std::string UDP_::address() { return ep_address(remote); }

size_t UDP_::write(const char *data, size_t len, boost::system::error_code &ec) {
    return M->write(remote, data, len, ec);
}

size_t UDP_::write(asio::streambuf &buf, boost::system::error_code &ec) { return M->write(remote, buf, ec); }

UDP UDP_::share() { return boost::dynamic_pointer_cast<UDP_>(Writer_::share()); }

uint64_t UDP_::Id() { return Id_; }

void Monitor_::received(const boost::system::error_code &err, size_t transferred) {
    if (err) {
        V_LOG_E("Monitor_ readed error(%d), will close", err.value());
        return;
    }
    size_t hlen = mod->header();
    size_t frame = mod->parse(cbuf);
    if (frame < 1) {
        V_LOG_E("%s", "Monitor_ readed invalid frame header, dropped");
        return;
    }
    if (frame < transferred - hlen) {
        V_LOG_E("Monitor_ readed invalid frame by transferred(%ld),expected(%ld), dropped", transferred, frame + hlen);
        return;
    }
    Data header = BuildData(cbuf, hlen);
    UDP uframe(new UDP_(ios, share(), remote));
    cmd->OnCmd(Cmd(new Cmd_(uframe, cbuf + hlen, frame, header)));
    receive();
}

void Monitor_::receive() {
    auto cback = boost::bind(&Monitor_::received, this, boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred);
    tremote = basic_endpoint<nudp>(udp::v4(), 0);
    sck.async_receive_from(boost::asio::buffer(cbuf, 1024), remote, cback);
}

void Monitor_::bind(basic_endpoint<nudp> ep, boost::system::error_code &ec) {
    sck.open(ep.protocol());
    if (reused) {
        sck.set_option(asio::socket_base::reuse_address(true));
    }
    sck.bind(ep, ec);
}

Monitor_::Monitor_(asio::io_service &ios, CmdH cmd)
    : ios(ios), sck(ios), cmd(cmd), UDP_(ios, Monitor(), basic_endpoint<nudp>(udp::v4(), 0)) {
    endpoint = udp::endpoint(udp::v4(), 0);
    mod = ModH(new M1L2);
}

Monitor_::Monitor_(asio::io_service &ios, basic_endpoint<nudp> ep, CmdH cmd)
    : ios(ios), sck(ios), endpoint(ep), cmd(cmd), UDP_(ios, Monitor(), basic_endpoint<nudp>(udp::v4(), 0)) {
    mod = ModH(new M1L2);
}

Monitor_::Monitor_(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd)
    : ios(ios),
      sck(ios),
      endpoint(basic_endpoint<nudp>(address::from_string(addr), port)),
      cmd(cmd),
      UDP_(ios, Monitor(), basic_endpoint<nudp>(udp::v4(), 0)) {
    mod = ModH(new M1L2);
}

Monitor_::~Monitor_() {}

void Monitor_::close() { sck.close(); }

void Monitor_::start(boost::system::error_code &ec) {
    if (endpoint.port()) {
        bind(endpoint, ec);
        if (ec) {
            return;
        }
    }
    receive();
}

size_t Monitor_::write(basic_endpoint<nudp> remote, const char *data, size_t len, boost::system::error_code &ec) {
    size_t hlen = mod->header();
    char tbuf[hlen + len];
    mod->full(tbuf, len);
    memcpy(tbuf + hlen, data, len);
    auto sended =
        sck.send_to(boost::asio::buffer(tbuf, len + hlen), remote, boost::asio::socket_base::message_peek, ec);
    if (endpoint.port() == 0) {
        endpoint = sck.local_endpoint();
        UDP_::M = share();
        UDP_::remote = tremote;
        receive();
    }
    return sended;
}

size_t Monitor_::write(const char *addr, unsigned short port, const char *data, size_t len,
                       boost::system::error_code &ec) {
    auto addr_ = address::from_string(addr, ec);
    if (ec) {
        return 0;
    }
    return write(basic_endpoint<nudp>(addr_, port), data, len, ec);
}

size_t Monitor_::write(basic_endpoint<nudp> remote, asio::streambuf &buf, boost::system::error_code &ec) {
    return write(remote, boost::asio::buffer_cast<const char *>(buf.data()), buf.size(), ec);
}

size_t Monitor_::write(const char *addr, unsigned short port, asio::streambuf &buf, boost::system::error_code &ec) {
    auto addr_ = address::from_string(addr, ec);
    if (ec) {
        return 0;
    }
    return write(basic_endpoint<nudp>(addr_, port), buf, ec);
}

Monitor Monitor_::share() { return boost::dynamic_pointer_cast<Monitor_>(Writer_::share()); }

Monitor BuildMonitor(asio::io_service &ios, CmdH cmd) { return Monitor(new Monitor_(ios, cmd)); }

Monitor BuildMonitor(asio::io_service &ios, basic_endpoint<nudp> ep, CmdH cmd) {
    return Monitor(new Monitor_(ios, ep, cmd));
}

Monitor BuildMonitor(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd) {
    return Monitor(new Monitor_(ios, addr, port, cmd));
}

Connector_::Connector_(asio::io_service &ios, CmdH cmd, ConH con) : TCP_(ios, cmd, con) {}

void Connector_::connect(const asio::ip::address &addr, unsigned short port, boost::system::error_code &ec) {
    connect(asio::ip::basic_endpoint<ntcp>(addr, port), ec);
}

void Connector_::connect(uint32_t addr, unsigned short port, boost::system::error_code &err) {
    connect(asio::ip::address_v4(addr), port, err);
}

void Connector_::connect(const char *addr, unsigned short port, boost::system::error_code &err) {
    connect(asio::ip::address::from_string(addr, err), port, err);
}

void Connector_::connect(asio::ip::basic_endpoint<ntcp> remote, boost::system::error_code &ec) {
    if (local.port()) {
        this->bind(local, ec);
        if (ec) {
            return;
        }
    } else {
        this->sck.open(local.protocol());
        this->sck.set_option(tcp::no_delay(true));
    }
    auto cback = boost::bind(&Connector_::connected, share(), asio::placeholders::error);
    this->sck.async_connect(remote, cback);
}

void Connector_::setlocal(uint32_t port) { local = asio::ip::basic_endpoint<ntcp>(asio::ip::address_v4::any(), port); }

void Connector_::connected(const boost::system::error_code &err) {
    if (err) {
        this->con->OnConn(this->share(), err);
        return;
    }
    boost::system::error_code ec;
    local = this->sck.local_endpoint(ec);
    if (ec) {
        this->con->OnConn(this->share(), ec);
        return;
    }
    this->con->OnConn(this->share(), err);
    this->start();
}

Connector Connector_::share() { return boost::dynamic_pointer_cast<Connector_>(Writer_::share()); }

std::string Connector_::address() { return "(" + ep_address(local) + "," + sck_address(sck) + ")"; }

Connector BuildConnector(asio::io_service &ios, CmdH cmd, ConH con) { return Connector(new Connector_(ios, cmd, con)); }

Acceptor_::Acceptor_(asio::io_service &ios, const basic_endpoint<asio::ip::tcp> &endpoint, CmdH cmd, ConH con)
    : ios(ios), cmd(cmd), con(con), act(ios) {
    this->endpoint = endpoint;
    mod = ModH(new M1L2());
    sequence_lck.lock();
    Id_ = sequence++;
    sequence_lck.unlock();
}

Acceptor_::Acceptor_(asio::io_service &ios, const boost::asio::ip::address &addr, unsigned short port, CmdH cmd,
                     ConH con)
    : ios(ios), cmd(cmd), con(con), act(ios) {
    this->endpoint = basic_endpoint<asio::ip::tcp>(addr, port);
    mod = ModH(new M1L2());
    sequence_lck.lock();
    Id_ = sequence++;
    sequence_lck.unlock();
}

Acceptor_::Acceptor_(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd, ConH con)
    : ios(ios), cmd(cmd), con(con), act(ios) {
    this->endpoint = basic_endpoint<asio::ip::tcp>(asio::ip::address::from_string(addr), port);
    mod = ModH(new M1L2());
    sequence_lck.lock();
    Id_ = sequence++;
    sequence_lck.unlock();
}

uint64_t Acceptor_::Id() { return Id_; }

void Acceptor_::accepted(TCP s, const boost::system::error_code &err) {
    if (err) {
        V_LOG_E("TcpServer accept fail with error(%d)", err.value());
        close();
        return;
    }
    s->mod = smod(s);
    if (con->OnConn(s, err)) {
        V_LOG_I("TcpServer accepted from %s", s->address().c_str());
        s->start();
    } else {
        V_LOG_W("TcpServer accept refuse from %s", s->address().c_str());
        s->close();
    }
    accept();
}

Acceptor Acceptor_::share() { return shared_from_this(); }

ModH Acceptor_::smod(TCP s) { return mod; }

void Acceptor_::bind(basic_endpoint<ntcp> &ep, boost::system::error_code &ec) {
    act.open(ep.protocol());
    if (reused) {
        act.set_option(asio::socket_base::reuse_address(true));
    }
    act.bind(ep, ec);
}

void Acceptor_::start(boost::system::error_code &ec) {
    bind(endpoint, ec);
    if (ec) {
        return;
    }
    act.listen();
    accept();
}

void Acceptor_::accept() {
    TCP basic(new TCP_(ios, cmd, con));
    auto cback = boost::bind(&Acceptor_::accepted, share(), basic, asio::placeholders::error);
    act.async_accept(basic->sck, cback);
}

void Acceptor_::close() { act.close(); }

Acceptor BuildAcceptor(asio::io_service &ios, const basic_endpoint<ntcp> &endpoint, CmdH cmd, ConH con) {
    return Acceptor(new Acceptor_(ios, endpoint, cmd, con));
}

Acceptor BuildAcceptor(asio::io_service &ios, const char *addr, unsigned short port, CmdH cmd, ConH con) {
    return Acceptor(new Acceptor_(ios, addr, port, cmd, con));
}

uint32_t cip2long(const char *ip) {
    auto addr = boost::asio::ip::address::from_string(ip);
    return addr.to_v4().to_ulong();
}
}
}
