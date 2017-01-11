//
//  cached_file.cpp
//  boost-utils
//
//  Created by Centny on 1/10/17.
//
//

#include "cached_file.hpp"
namespace butils {
namespace fs {

CachedFile::CachedFile(boost::filesystem::path& path, size_t size, const char* csuf, const char* tsuf) {
    auto cp = path, tp = path;
    cp += csuf, tp += tsuf;
    tfs.open(tp.c_str(),std::ios::binary | std::ios::out);
    cfs.open(cp.c_str(),std::ios::binary | std::ios::out);
    tfs.seekp(size-1);
    tfs.write("", 1);
    cfs.write("", 1);
}

void CachedFile::write(size_t offset,void* buf,size_t len){
    
}

void CachedFile::close() {
    tfs.close();
    cfs.close();
}
}
}
