//
//  endian.hpp
//  butil
//
//  Created by Centny on 1/1/17.
//
//

#ifndef endian_hpp
#define endian_hpp
#include <stdio.h>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/buffers.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace butils {
namespace endian {
using namespace std;
using namespace boost;

/**/
template <BOOST_SCOPED_ENUM(boost::endian::order) Order>  // the EndianBuf template
class EndianBuf {
   public:
    template <typename T, std::size_t n_bits>  // the put template
    EndianBuf &put(T val) {
        const char *data = boost::endian::endian_buffer<Order, T, n_bits, boost::endian::align::yes>(val).data();
        buf_.sputn(data, n_bits);
        return *this;
    }
    const char *data() { return asio::buffer_cast<const char *>(buf_.data()); }
    asio::streambuf &buf() { return buf_; }

   protected:
    asio::streambuf buf_;
};

typedef EndianBuf<boost::endian::order::little> LittleEndianBuf;
typedef EndianBuf<boost::endian::order::big> BigEndianBuf;
}
}

#endif /* endian_hpp */
