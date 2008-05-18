#include "medium.hxx"
#include "directory.hxx"
#include "insert.hxx"
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
