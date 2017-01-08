//
//  socket_test.cpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include "socket.hpp"

using namespace boost;
using namespace butils::netw;

BOOST_AUTO_TEST_SUITE(TestSocket)  // name of the test suite is stringtest

class A {};

template <typename T>
class B {};
template <typename T>
class C : public A {};
class A1 : public boost::enable_shared_from_this<A1> {};

template <typename T>
class B1 : public boost::enable_shared_from_this<B1<T> > {
   public:
    boost::shared_ptr<B1<T> > xx() { return this->shared_from_this(); }
};
template <typename T>
class C1 : public A1 {};
template <typename T>
class C2 : public A, public boost::enable_shared_from_this<C2<T> > {};
BOOST_AUTO_TEST_CASE(TestShared) {
    boost::shared_ptr<A> a(new A);
    boost::shared_ptr<B<int> > b(new B<int>);
    boost::shared_ptr<C<int> > c(new C<int>);
    boost::shared_ptr<A> c2(new C<int>);
    boost::shared_ptr<A1> a1(new A1);
    a1->shared_from_this();
    boost::shared_ptr<B1<int> > b1(new B1<int>);
    b1->shared_from_this();
    b1->xx();
    boost::shared_ptr<C1<int> > c1(new C1<int>);
    c1->shared_from_this();
    boost::shared_ptr<A1> c12(new C1<int>);
    c12->shared_from_this();
    boost::shared_ptr<C2<int> > c13(new C2<int>);
    c13->shared_from_this();
    boost::shared_ptr<A> c14(new C2<int>);
    // c14->shared_from_this();
    //  boost::shared_ptr<A > c2(new C<int>);
    printf("%s\n", "test shared ptr done...");
}
typedef boost::shared_ptr<A> APtr;

BOOST_AUTO_TEST_CASE(TestShared2) {
    APtr a = APtr(new A);
    printf("%lu\n", a.use_count());
    APtr b = a;
    printf("%lu\n", b.use_count());
    printf("%s\n", "test shared ptr done...");
}

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
    bool res;

   public:
    virtual int OnCmd(Cmd c) {
        printf("receive->%s\n", c->cdata());
        received++;
        if (res && received < 2) {
            boost::system::error_code ec;
            c->writer->write("abc", 3, ec);
        }
        return 0;
    }
};
class TcpTest {
   public:
    asio::io_service ios;
    Acceptor srv;
    Connector c1;
    void run() {
        ConH con = ConH(new TestConH());
        CmdH cmd = CmdH(new TestCmdH());
        TestCmdH *ch = (TestCmdH *)cmd.get();
        //  boost::asio::tcp::acceptor x;
        boost::system::error_code ec;
        printf("%s\n", "->done...");
        srv = BuildAcceptor(ios, "0.0.0.0", 14222, cmd, con);
        srv->start(ec);
        BOOST_CHECK(ec == 0);
        c1 = BuildConnector(ios, cmd, con);
        c1->connect("127.0.0.1", 14222, ec);
        BOOST_CHECK(ec == 0);
        ios.run();
        BOOST_CHECK_EQUAL(ch->received, 2);
    }
    void close() {
        srv->close();
        c1->close();
    }
    void post() { ios.post(boost::bind(&TcpTest::close, this)); }
};
class UdpTest {
   public:
    asio::io_service ios;
    Monitor srv;
    Monitor c1;
    void run() {
        CmdH cmd = CmdH(new TestCmdH());
        TestCmdH *ch = (TestCmdH *)cmd.get();
        ch->res = true;
        //  boost::asio::tcp::acceptor x;
        boost::system::error_code ec;
        printf("%s\n", "->done...");
        srv = BuildMonitor(ios, "0.0.0.0", 14222, cmd);
        srv->mod->header();
        srv->start(ec);
        BOOST_CHECK(ec == 0);
        c1 = BuildMonitor(ios, cmd);
        c1->start(ec);
        BOOST_CHECK(ec == 0);
        c1->write("127.0.0.1", 14222, "abc", 3, ec);
        BOOST_CHECK(ec == 0);
        ios.run();
        BOOST_CHECK_EQUAL(ch->received, 2);
    }
    void close() {
        srv->close();
        c1->close();
    }
    void post() { ios.post(boost::bind(&UdpTest::close, this)); }
};

BOOST_AUTO_TEST_CASE(TestTcpRun) {
    //    return;
    TcpTest rt;
    boost::thread thrd(bind(&TcpTest::run, &rt));
    sleep(1);
    rt.post();
    thrd.join();
    printf("%s\n", "test tcp socket read write done...");
}

BOOST_AUTO_TEST_CASE(TestUdpRun) {
    //  return;
    UdpTest rt;
    boost::thread thrd(bind(&UdpTest::run, &rt));
    // rt.run();
    sleep(1);
    rt.post();
    thrd.join();
    printf("%s\n", "test udp socket read write done...");
}

BOOST_AUTO_TEST_SUITE_END()
