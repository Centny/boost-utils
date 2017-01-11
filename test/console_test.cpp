//
//  console_test.cpp
//  boost-utils
//
//  Created by Centny on 1/3/17.
//
//

#include <errno.h>
#include <stdio.h>
#include "../fs/cached_file_test_c.hpp"
#include "../netw/socket_test_c.hpp"

int main(int argc, char **argv) {
    cached_file_c(argc, argv);
    // socket_c(argc, argv);
    printf("%s\n", "all done...");
    return 0;
}
