#include "medium.hxx"
#include <stdlib.h>

using std::auto_ptr;
using std::string;
using std::list;

Medium::EInUse::EInUse():runtime_error("Medium::EInUse") {}
Medium::ENotFound::ENotFound():runtime_error("Medium::ENotFound") {}

std::auto_ptr<Medium> Medium::defaultmedium(FsDb& dbs){
   return getmedium(dbs,0);
}

std::auto_ptr<Medium> Medium::getmedium(FsDb& dbs, uint32_t id){
   Register r(dbs.media,id);
   Buffer b=r.getattrv("mediumtype");
   string mediumtype(b.data,b.size);
   if(mediumtype=="directory")
      return std::auto_ptr<Medium>(new Medium_directory(dbs,id));
   else if(mediumtype=="insert")
      return std::auto_ptr<Medium>(new Medium_insert(dbs,id));
   throw ENotFound();
}
      
std::auto_ptr<Medium> Medium::create(FsDb& dbs, std::string type){
   if(type=="directory")
      return auto_ptr<Medium>(Medium_directory::create(dbs));
   else if(type=="insert")
      return auto_ptr<Medium>(Medium_insert::create(dbs));
   throw ENotFound();
}
      
void Medium::remove(){
   if(getattr<uint32_t>("refcount"))
      throw EInUse();
   Register::remove();
}

Medium::Stats Medium::collectstats(FsDb& dbs){
   Stats st;
   list<uint32_t> rs=dbs.media.listregisters();
   for(list<uint32_t>::iterator it=rs.begin();it!=rs.end();it++){
      Stats st_=getmedium(dbs,*it)->getstats();
      st.blocks+=st_.blocks;
      st.freeblocks+=st_.freeblocks;
   }
   return st;
}

void Medium::addfile(File& f,string phid){
   setattr<uint32_t>("refcount",getattr<uint32_t>("refcount")+1);
}

void Medium::delfile(File& f){
   setattr<uint32_t>("refcount",getattr<uint32_t>("refcount")-1);
}


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


std::auto_ptr<Medium_insert> Medium_insert::create(FsDb& dbs){
   auto_ptr<Medium_directory> r=Medium_directory::create(dbs);
   string mediumtype("insert");
   r->setattrv("mediumtype",Buffer(mediumtype.c_str(),mediumtype.size()));
   r->setattrv("label",Buffer(NULL,0));
   r->setattrv("checkcmd",Buffer(NULL,0));
   r->setattrv("insertscript",Buffer(NULL,0));
   return auto_ptr<Medium_insert>(new Medium_insert(dbs,r->getid()));
}

std::auto_ptr<Source> Medium_insert::getsource(File& f,int mode){
   insert();
   return Medium_directory::getsource(f,mode);
}

int Medium_insert::truncate(File& f,off_t length){
   insert();
   return Medium_directory::truncate(f,length);
}

Medium::Stats Medium_insert::getstats(){
   insert();
   return Medium_directory::getstats();
}

void Medium_insert::addfile(File& f,string phid){
   Medium_directory::addfile(f,phid);
}

void Medium_insert::delfile(File& f){
   Buffer b=getattrv("unlink_files");
   if(string(b.data,b.size)=="true")
      insert();
   Medium_directory::delfile(f);
}

bool Medium_insert::check(){
   Buffer b=getattrv("checkcmd");
   string command(b.data,b.size);
   int err=system(command.c_str());
   if(err==-1)
      throw std::runtime_error("Medium_insert::check: error calling system.");
   return (WEXITSTATUS(err)==0);
}

void Medium_insert::insert(){
   if(!check()){
      Buffer b=getattrv("label");
      string label(b.data,b.size);
      b=getattrv("directory");
      string directory(b.data,b.size);
      b=getattrv("insertscript");
      string insertscript(b.data,b.size);
      string insertcmd="sh "+insertscript+" \""+label+"\" \""+directory+"\"";
      int err=system(insertcmd.c_str());
      if(err==-1)
	 throw std::runtime_error("Medium_insert::insert: error calling system.");
      if(WEXITSTATUS(err))
	 throw std::runtime_error("Medium_insert::insert: error inserting the volume.");
      if(!check())
	 throw std::runtime_error("Medium_insert::insert: check failed.");
   }
}
