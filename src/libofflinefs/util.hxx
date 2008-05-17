#ifndef UTIL_HXX
#define UTIL_HXX

#include "common.hxx"
#include "fsdb.hxx"
#include "fsnodes.hxx"

#define MIN(a,b) (a<b?a:b)

bool ingroup(uid_t uid,gid_t gid);

inline void checkparentaccess(FsDb& dbs,std::string path,int mode){
   Directory::Path p(dbs,path);
   p.parent->access(fuse_get_context()->uid,fuse_get_context()->gid,mode);
   while(p.parent->getchild("..")->getid()!=p.parent->getid()){
      p.parent.reset((Directory*)p.parent->getchild("..").release());
      p.parent->access(fuse_get_context()->uid,fuse_get_context()->gid,X_OK);
   }
}

inline void checkaccess(FsDb& dbs, std::string path,int mode){
   checkparentaccess(dbs,path,X_OK);
   Node::getnode(dbs,path)->access(fuse_get_context()->uid,fuse_get_context()->gid,mode);
}



#endif
