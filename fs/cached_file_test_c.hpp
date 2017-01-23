//
//  cached_file_test_c.hpp
//  boost-utils
//
//  Created by Centny on 1/10/17.
//
//

#ifndef cached_file_test_c_h
#define cached_file_test_c_h
#include "cached_file.hpp"
using namespace boost;
using namespace butils::fs;

int cached_file_c(int argc, char **argv) {
    boost::filesystem::path p("/tmp/a.dat");
    CachedFile cf(p, 1000000);
    cf.close();
    printf("%s\n", "all done...");
    return 0;
}

#endif /* cached_file_test_c_h */
