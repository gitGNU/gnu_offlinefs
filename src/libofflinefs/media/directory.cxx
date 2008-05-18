#include "directory.hxx"
#include <stdlib.h>

using std::auto_ptr;
using std::string;
using std::list;

std::string Medium_directory::realpath(File& f){
   Buffer b=getattrv("directory");
   string basepath(b.data,b.size);
   b=f.getattrv("offlinefs.phid");
   string filepath(b.data,b.size);
   return (basepath+"/"+filepath);   
}

std::auto_ptr<Medium_directory> Medium_directory::create(FsDb& dbs){
   auto_ptr<Medium_directory> m(new Medium_directory(dbs,dbs.media.createregister()));
   string mediumtype("directory");
   m->setattrv("mediumtype",Buffer(mediumtype.c_str(),mediumtype.size()));
   m->setattr<uint32_t>("refcount",0);
   m->setattrv("directory",Buffer(NULL,0));
   string unlink_files("false");
   m->setattrv("unlink_files",Buffer(unlink_files.c_str(),unlink_files.size()));
   return m;
}

std::auto_ptr<Source> Medium_directory::getsource(File& f,int mode){
   return std::auto_ptr<Source>(new Source_file(f,realpath(f),mode));
}

inline int real_truncate(const char* path, off_t length){
   return truncate(path,length);
}

int Medium_directory::truncate(File& f,off_t length){
   return real_truncate(realpath(f).c_str(),length);
}

Medium::Stats Medium_directory::getstats(){
   Buffer b=getattrv("directory");
   struct statvfs st;
   if(statvfs(string(b.data,b.size).c_str(),&st))
      throw std::runtime_error("Medium_directory::getstats: error calling statvfs.");
   Stats st_;
   st_.blocks=st.f_blocks*st.f_frsize/4096;
   st_.freeblocks=st.f_bfree*st.f_frsize/4096;
   return st_;
}

void Medium_directory::addfile(File& f,string phid){
   f.setattrv("offlinefs.phid",Buffer(phid.c_str(),phid.size()));
   f.setattr<uint32_t>("offlinefs.mediumid",getid());
   Medium::addfile(f,phid);
}

void Medium_directory::delfile(File& f){
   Buffer b=getattrv("unlink_files");
   if(string(b.data,b.size)=="true")
      unlink(realpath(f).c_str());
   Medium::delfile(f);
}


