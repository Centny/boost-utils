//
//  endian_test.cpp
//  butil
//
//  Created by Centny on 1/1/17.
//
//
#include "endian.hpp"
#include <boost/test/included/unit_test.hpp>

using namespace boost;

BOOST_AUTO_TEST_SUITE(TestEndianBuf) // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TestPut) {
  const char *data;

  //
  //
  boost::utils::LittleEndianBuf little1;
  //  little1 = boost::utils::LittleEndianBuf();
  little1.put<uint8_t, 8>(10);
  data = little1.data();
  BOOST_CHECK(data[0] == 10);
  //
  //
  boost::utils::LittleEndianBuf little2;
  //  little2 = boost::utils::LittleEndianBuf();
  little2.put<uint16_t, 16>(10);
  data = little2.data();
  BOOST_CHECK(data[0] == 10);
  //
  //
  boost::utils::BigEndianBuf big1;
  //    big1 = boost::utils::BigEndianBuf();
  big1.put<uint16_t, 16>(10);
  data = big1.data();
  BOOST_CHECK(data[1] == 10);
  printf("%s\n","test endian put done...");
}

BOOST_AUTO_TEST_SUITE_END()
