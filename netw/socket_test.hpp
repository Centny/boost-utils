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
class B1 : public boost::enable_shared_from_this<B1<T> > {};
template <typename T>
class C1 : public A1 {};
template <typename T>
class C2 : public A, public boost::enable_shared_from_this<C2<T> > {};
BOOST_AUTO_TEST_CASE(TestSharedPtr) {
    boost::shared_ptr<A> a(new A);
    boost::shared_ptr<B<int> > b(new B<int>);
    boost::shared_ptr<C<int> > c(new C<int>);
    boost::shared_ptr<A> c2(new C<int>);
    boost::shared_ptr<A1> a1(new A1);
    a1->shared_from_this();
    boost::shared_ptr<B1<int> > b1(new B1<int>);
    b1->shared_from_this();
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

class TestConH : public ConH {
   public:
    virtual bool OnConn(BasicSocketPtr s, const boost::system::error_code &ec) {
        std::cout << "connected:" << ec.message() << std::endl;
        boost::system::error_code wec;
        s->write("abc", 3, wec);
        std::cout << "writed:" << wec.message() << std::endl;
    }
    // event on socket close.
    virtual void OnClose(BasicSocketPtr s, const boost::system::error_code &ec) {
        std::cout << ec.message() << std::endl;
    }
};
class TestCmdH : public CmdH {
   public:
    virtual int OnCmd(CmdPtr c) {
        printf("receive->%s", c->cdata());
        return 0;
    }
};

BOOST_AUTO_TEST_CASE(TestReadWrite) {
    //  return;
    ConHPtr con = ConHPtr(new TestConH());
    CmdHPtr cmd = CmdHPtr(new TestCmdH());
    //  boost::asio::tcp::acceptor x;
    asio::io_service ios;
    boost::thread thrd(boost::bind(&asio::io_service::run, &ios));
    boost::system::error_code ec;
    AcceptorPtr<boost::asio::ip::tcp> srv(new Acceptor<boost::asio::ip::tcp>(ios, "0.0.0.0", 14222, cmd, con));
    srv->start(ec);
    BOOST_CHECK(ec == 0);
    ConnectorPtr<boost::asio::ip::tcp> c1(new Connector<boost::asio::ip::tcp>(ios, cmd, con));
    c1->connect("127.0.0.1", 14222, ec);
    BOOST_CHECK(ec == 0);
    sleep(3);
    printf("%s\n", "test socket read write done...");
}

BOOST_AUTO_TEST_SUITE_END()
