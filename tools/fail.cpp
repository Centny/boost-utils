//
//  fail.cpp
//  boost-utils
//
//  Created by Centny on 1/23/17.
//
//

#include "fail.hpp"

#include <string>

Fail_::Fail_(int code) throw() : code(code), err(0) {}

Fail_::Fail_(size_t bsize, const char* fmt, ...) throw() : code(0) {
    err = new char[bsize];
    va_list args;
    va_start(args, fmt);
    vsprintf(err, fmt, args);
    va_end(args);
}

Fail_::Fail_(size_t bsize, int code, const char* fmt, ...) throw() : code(code) {
    err = new char[bsize];
    va_list args;
    va_start(args, fmt);
    vsprintf(err, fmt, args);
    va_end(args);
}

const char* Fail_::what() const throw() { return err; }

Fail_::~Fail_() throw() {
    delete err;
    err = 0;
}
