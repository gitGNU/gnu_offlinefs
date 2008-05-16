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
   else if(mediumtype=="mount")
      return std::auto_ptr<Medium>(new Medium_mount(dbs,id));
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

std::auto_ptr<Medium_directory> Medium_directory::create(FsDb& dbs,string path){
   auto_ptr<Medium_directory> m(new Medium_directory(dbs,dbs.media.createregister()));
   string mediumtype("directory");
   m->setattrv("mediumtype",Buffer(mediumtype.c_str(),mediumtype.size()));
   m->setattr<uint32_t>("refcount",0);
   m->setattrv("directory",Buffer(path.c_str(),path.size()));
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
   unlink(realpath(f).c_str());
   Medium::delfile(f);
}


std::auto_ptr<Medium_mount> Medium_mount::create(FsDb& dbs,string path,string label,string checkcmd){
   auto_ptr<Medium_directory> r=Medium_directory::create(dbs,path);
   string mediumtype("mount");
   r->setattrv("mediumtype",Buffer(mediumtype.c_str(),mediumtype.size()));
   r->setattrv("label",Buffer(label.c_str(),label.size()));
   r->setattrv("checkcmd",Buffer(checkcmd.c_str(),checkcmd.size()));
   return auto_ptr<Medium_mount>(new Medium_mount(dbs,r->getid()));
}

std::auto_ptr<Source> Medium_mount::getsource(File& f,int mode){
   insert();
   return Medium_directory::getsource(f,mode);
}

int Medium_mount::truncate(File& f,off_t length){
   insert();
   return Medium_directory::truncate(f,length);
}

Medium::Stats Medium_mount::getstats(){
   insert();
   return Medium_directory::getstats();
}

void Medium_mount::addfile(File& f,string phid){
   Medium_directory::addfile(f,phid);
}

void Medium_mount::delfile(File& f){
   insert();
   Medium_directory::delfile(f);
}

bool Medium_mount::check(){
   Buffer b=getattrv("checkcmd");
   string command(b.data,b.size);
   int err=system(command.c_str());
   if(err==-1)
      throw std::runtime_error("Medium_mount::check: error calling system.");
   return (WEXITSTATUS(err)==0);
}

bool Medium_mount::ask(){
   Buffer b=getattrv("label");
   string label(b.data,b.size);
//   string command="kdialog --warningcontinuecancel  \"Insert volume: \\\""+ label +"\\\"\" --title \"Insert volume\" --name offlinefs --caption offlinefs";
   string command="offlinefs_ask.sh \"" + label + "\"";
   int err=system(command.c_str());
   if(err==-1)
      throw std::runtime_error("Medium_mount::ask: error calling system.");
   return (WEXITSTATUS(err)==0);
}

void Medium_mount::mount(){
   Buffer b=getattrv("directory");
   string directory(b.data,b.size);
   string command="mount "+directory;
   int err=system(command.c_str());
   if(err==-1)
      throw std::runtime_error("Medium_mount::mount: error calling system.");
   if(WEXITSTATUS(err))
      throw std::runtime_error("Medium_mount::mount: error mounting the volume.");
}

void Medium_mount::insert(){
   if(!check()){
      if(!ask())
	 throw std::runtime_error("Medium_mount::insert: operation cancelled by the user.");
      mount();
      if(!check())
	 throw std::runtime_error("Medium_mount::insert: check failed.");
   }
}
