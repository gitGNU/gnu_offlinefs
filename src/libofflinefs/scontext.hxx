#ifndef SCONTEXT_HXX
#define SCONTEXT_HXX

#include <common.hxx>
#include <set>

class SContext{
   public:
      SContext(uid_t uid,gid_t gid);
      uid_t uid;
      gid_t gid;
      std::set<gid_t> groups; // Unix groups the user belongs to
};

#endif
