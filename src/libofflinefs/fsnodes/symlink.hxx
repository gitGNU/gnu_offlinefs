#ifndef FSNODES_SYMLINK_HXX
#define FSNODES_SYMLINK_HXX

#include "node.hxx"

class Symlink:public Node{
   public:
      Symlink(FsDb& dbs,uint64_t id);

      static std::auto_ptr<Symlink> create(FsDb& dbs);
      static std::auto_ptr<Symlink> create(FsDb& dbs,const SContext& sctx,std::string path);
};

#endif
