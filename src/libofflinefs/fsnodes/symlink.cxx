#include "symlink.hxx"
#include "directory.hxx"

using std::auto_ptr;
using std::string;

Symlink::Symlink(FsDb& dbs,uint64_t id):Node(dbs,id) {
}


auto_ptr<Symlink> Symlink::create(FsDb& dbs){
   auto_ptr<Symlink> n(new Symlink(dbs,Node::create(dbs)->getid()));
   n->setattr<mode_t>("offlinefs.mode",S_IFLNK);
   n->setattrv("offlinefs.symlink",Buffer());
   return n;
}

std::auto_ptr<Symlink> Symlink::create(FsDb& dbs,const SContext& sctx,std::string path){
   return Node::create_<Symlink>(dbs,sctx,path);
}
