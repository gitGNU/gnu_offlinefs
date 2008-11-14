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

#include "insert.hxx"
#include <stdlib.h>

using std::auto_ptr;
using std::string;
using std::list;

auto_ptr<Medium_insert> Medium_insert::create(FsTxn& txns){
   auto_ptr<Medium_directory> m(Medium_directory::create(txns));

   m->setattrv("type",Buffer("insert"));
   m->setattrv("checkcmd",Buffer(""));
   m->setattrv("insertcmd",Buffer(""));

   return auto_ptr<Medium_insert>(new Medium_insert(txns,m->getid()));
}

std::auto_ptr<Chunk> Medium_insert::getchunk(std::string phid, int mode){
   insert();
   return Medium_directory::getchunk(phid,mode);
}

Medium::Stats Medium_insert::getstats(){
   insert();
   return Medium_directory::getstats();
}

void Medium_insert::addfile(string phid){
   Medium_directory::addfile(phid);
}

void Medium_insert::delfile(string phid){
   if(string(getattrv("unlink_files")) == "true")
      insert();
   Medium_directory::delfile(phid);
}

bool Medium_insert::check(){
   string checkcmd = getattrv("checkcmd");
   int err=system(checkcmd.c_str());
   if(err==-1)
      throw std::runtime_error("Medium_insert::check: error calling system.");
   return (WEXITSTATUS(err)==0);
}

void Medium_insert::insert(){
   if(!check()){
      string insertcmd = getattrv("insertcmd");
      int err=system(insertcmd.c_str());
      if(err==-1)
	 throw std::runtime_error("Medium_insert::insert: error calling system.");
      if(WEXITSTATUS(err))
	 throw std::runtime_error("Medium_insert::insert: error inserting the volume.");
      if(!check())
	 throw std::runtime_error("Medium_insert::insert: checkcmd failed.");
   }
}
