//
//  cached_file_test.hpp
//  boost-utils
//
//  Created by Centny on 1/10/17.
//
//

#ifndef cached_file_test_h
#define cached_file_test_h
#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>

using namespace boost;
using namespace butils::fs;

BOOST_AUTO_TEST_SUITE(CachedFile)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TestCachedFile) {
    boost::filesystem::path p("a.dat");
    printf("%s\n", "test endian put done...");
}

BOOST_AUTO_TEST_SUITE_END()
#endif /* cached_file_test_h */
