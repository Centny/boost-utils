//
//  log.hpp
//  boost-utils
//
//  Created by Centny on 1/2/17.
//
//

#ifndef log_hpp
#define log_hpp

#include "log.h"
#define V_LOG_D v_cwf_log_d
#define V_LOG_FREE v_cwf_log_d
#define V_LOG_I v_cwf_log_i
#define V_LOG_W v_cwf_log_w
#define V_LOG_E v_cwf_log_e

class Fail_ {
   public:
    char err[256];
    Fail_(const char* fmt, ...);
};

#define Fail(fmt, args...) Fail_(fmt, args)

#endif /* log_hpp */
