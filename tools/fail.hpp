//
//  fail.hpp
//  boost-utils
//
//  Created by Centny on 1/23/17.
//
//

#include <exception>
#ifndef fail_hpp
#define fail_hpp

/*
 the common fail exception by code/message.
 */
class Fail_ : public std::exception {
   public:
    int code;
    char* err;

   public:
    Fail_(int code) _NOEXCEPT;
    Fail_(size_t bsize, const char* fmt, ...) _NOEXCEPT;
    Fail_(size_t bsize, int code, const char* fmt, ...) _NOEXCEPT;
    virtual const char* what() const throw();
    virtual ~Fail_() _NOEXCEPT;
};

#define Fail(fmt, args...) Fail_(256, fmt, args)
#define LFail(bsize, fmt, args...) Fail_(bsize + 1024, 0, fmt, args)
#define CFail(code) Fail_(code)
#define LCFail(bsize, code, fmt, args...) Fail_(bsize, code, fmt, args)

#endif /* fail_hpp */
