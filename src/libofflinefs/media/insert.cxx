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

Medium_insert::Medium_insert(libconfig::Setting& conf): Medium_directory(conf){
   if(!conf.lookupValue("checkcmd",checkcmd)){
      std::ostringstream os;
      os << "Medium_insert::Medium_insert: Error parsing config file after line " 
	 << conf.getSourceLine() << ": \"checkcmd\" parameter required.";
      throw std::runtime_error(os.str());
   }

   if(!conf.lookupValue("insertcmd",insertcmd)){
      std::ostringstream os;
      os << "Medium_insert::Medium_insert: Error parsing config file after line " 
	 << conf.getSourceLine() << ": \"insertcmd\" parameter required.";
      throw std::runtime_error(os.str());
   }
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
   if(unlink_files)
      insert();
   Medium_directory::delfile(f);
}

bool Medium_insert::check(){
   int err=system(checkcmd.c_str());
   if(err==-1)
      throw std::runtime_error("Medium_insert::check: error calling system.");
   return (WEXITSTATUS(err)==0);
}

void Medium_insert::insert(){
   if(!check()){
      int err=system(insertcmd.c_str());
      if(err==-1)
	 throw std::runtime_error("Medium_insert::insert: error calling system.");
      if(WEXITSTATUS(err))
	 throw std::runtime_error("Medium_insert::insert: error inserting the volume.");
      if(!check())
	 throw std::runtime_error("Medium_insert::insert: checkcmd failed.");
   }
}
