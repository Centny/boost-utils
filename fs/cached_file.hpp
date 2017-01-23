//
//  cached_file.hpp
//  boost-utils
//
//  Created by Centny on 1/10/17.
//
//

#ifndef cached_file_hpp
#define cached_file_hpp
#include <boost/filesystem.hpp>
#include <fstream>
namespace butils {
namespace fs {
class CachedFile {
   protected:
    std::fstream tfs;
    std::fstream cfs;

   public:
    CachedFile(boost::filesystem::path& path, size_t size, const char* csuf = ".xdm", const char* tsuf = ".xcf");
    virtual void write(size_t offset, void* buf, size_t len);
    virtual void close();
};
}
}
#endif /* cached_file_hpp */
