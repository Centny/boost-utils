//
//  endian_test.cpp
//  butil
//
//  Created by Centny on 1/1/17.
//
//
#include <boost/test/included/unit_test.hpp>
#include "endian.hpp"

using namespace boost;

BOOST_AUTO_TEST_SUITE(TestEndianBuf)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TestPut) {
    const char *data;

    //
    //
    butils::endian::LittleEndianBuf little1;
    //  little1 = boost::utils::LittleEndianBuf();
    little1.put<uint8_t, 8>(10);
    data = little1.data();
    BOOST_CHECK(data[0] == 10);
    //
    //
    butils::endian::LittleEndianBuf little2;
    //  little2 = boost::utils::LittleEndianBuf();
    little2.put<uint16_t, 16>(10);
    data = little2.data();
    BOOST_CHECK(data[0] == 10);
    //
    //
    butils::endian::BigEndianBuf big1;
    //    big1 = boost::utils::BigEndianBuf();
    big1.put<uint16_t, 16>(10);
    data = big1.data();
    BOOST_CHECK(data[1] == 10);
    printf("%s\n", "test endian put done...");
}

BOOST_AUTO_TEST_CASE(TestLoad) {
    char buf[2];
    endian::detail::store_big_endian<uint16_t, 2>(buf, 3);
    printf("%d,%d\n", buf[0], buf[1]);
    char buf1[10];
    auto val = endian::detail::load_big_endian<uint16_t, 2>(buf);
    BOOST_CHECK(val == 3);
    printf("%s\n", "test endian load done...");
}
BOOST_AUTO_TEST_SUITE_END()
