//
//  boost-utils.hpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#ifndef boost_utils_h
#define boost_utils_h
#include "endian/endian.hpp"
#include "netw/socket.hpp"
#include "tools/data.hpp"
#include "tools/sqlite.hpp"

#define SHARED_TO(T) boost::dynamic_pointer_cast<T>(shared_from_this())

#endif /* boost_utils_h */
