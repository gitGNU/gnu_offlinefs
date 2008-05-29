//     Copyright (C) 2008 Francisco Jerez
//     This file is part of offlinefs.

//     offlinefs is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     offlinefs is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.

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
