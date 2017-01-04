//
//  console_test.cpp
//  boost-utils
//
//  Created by Centny on 1/3/17.
//
//

#include <errno.h>
#include <stdio.h>
#include "../netw/socket.hpp"
using namespace butils::netw;

class TestConH : public ConH_ {
   public:
    int closed = 0;

   public:
    virtual bool OnConn(TCP s, const boost::system::error_code &ec) {
        if (ec) {
            //            std::error_code(ec.value());
            std::cout << "connected fail " << ec.value() << std::endl;
        }
        std::cout << "connected" << std::endl;
        boost::system::error_code wec;
        s->write("abc", 3, wec);
        std::cout << "writed:" << wec.value() << std::endl;
        return true;
    }
    // event on socket close.
    virtual void OnClose(TCP s, const boost::system::error_code &ec) {
        std::cout << "closed:" << ec.value() << std::endl;
        closed++;
    }
};
class TestCmdH : public CmdH_ {
   public:
    int received = 0;

   public:
    virtual int OnCmd(Cmd c) {
        printf("receive->%s\n", c->cdata());
        received++;
        return 0;
    }
};

int main(int argc, char **argv) {
    //  return;
    ConH con = ConH(new TestConH());
    CmdH cmd = CmdH(new TestCmdH());
    //    V_LOG_E("sfsdfksdfdsfkdsklfsdkf->%d", 1);
    //  boost::asio::tcp::acceptor x;
    asio::io_service ios;
    //  boost::thread thrd(boost::bind(&asio::io_service::run, &ios));
    boost::system::error_code ec;
    if (argc > 1 && argv[1][0] == 's') {
        Acceptor srv = BuildAcceptor(ios, "0.0.0.0", 14222, cmd, con);
        M1L2 *mod = (M1L2 *)srv->mod.get();
        mod->magic = 'A';
        srv->start(ec);
        std::cout << ec << std::endl;
        ios.run();
    } else {
        Connector c1 = BuildConnector(ios, cmd, con);
        M1L2 *mod = (M1L2 *)c1->mod.get();
        mod->magic = 'A';
        c1->connect("127.0.0.1", 14222, ec);
        std::cout << ec << std::endl;
        //        boost::shared_ptr<XX<int> > xx(new XX<int>);
        //        c1->sck.async_connect(boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>(boost::asio::ip::address::from_string("127.0.0.1"),14222),
        //        boost::bind(&XX<int>::connected,xx, boost::asio::placeholders::error));
        ios.run();
    }
    printf("%s-%s\n", argv[1], "all done...");
    return 0;
}
