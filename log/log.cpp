//
//  log.cpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#include "log.hpp"
#include <string>

Fail_::Fail_(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(err, fmt, args);
    va_end(args);
}
