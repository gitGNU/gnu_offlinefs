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

#include "directory.hxx"
#include <stdlib.h>

using std::auto_ptr;
using std::string;
using std::list;

std::string Medium_directory::realpath(File& f){
   Buffer b=f.getattrv("offlinefs.phid");
   string phid(b.data,b.size);
   return directory + "/" + phid;   
}

Medium_directory::Medium_directory(libconfig::Setting& conf){
   if(!conf.lookupValue("label",label)){
      std::ostringstream os;
      os << "Medium_directory::Medium_directory: Error parsing config file after line " 
	 << conf.getSourceLine() << ": \"label\" parameter required.";
      throw std::runtime_error(os.str());
   }

   if(!conf.lookupValue("directory",directory)){
      std::ostringstream os;
      os << "Medium_directory::Medium_directory: Error parsing config file after line " 
	 << conf.getSourceLine() << ": \"directory\" parameter required.";
      throw std::runtime_error(os.str());
   }

   if(conf.exists("unlink_files")){
      if(!conf.lookupValue("unlink_files",unlink_files)){
	 std::ostringstream os;
	 os << "Medium_directory::Medium_directory: Error parsing config file after line " 
	    << conf.getSourceLine() << ": \"unlink_files\" requires a boolean value.";
	 throw std::runtime_error(os.str());
      }
   }
}

std::auto_ptr<Source> Medium_directory::getsource(File& f,int mode){
   return std::auto_ptr<Source>(new Source_file(f,realpath(f),mode));
}

static inline int truncate_(const char* path, off_t length){
   return truncate(path,length);
}

int Medium_directory::truncate(File& f,off_t length){
   return truncate_(realpath(f).c_str(),length);
}

Medium::Stats Medium_directory::getstats(){
   struct statvfs st;
   if(statvfs(directory.c_str(),&st))
      throw std::runtime_error("Medium_directory::getstats: error calling statvfs.");
   Stats st_;
   st_.blocks=st.f_blocks*st.f_frsize/4096;
   st_.freeblocks=st.f_bfree*st.f_frsize/4096;
   return st_;
}

void Medium_directory::addfile(File& f,string phid){
   f.setattrv("offlinefs.phid",Buffer(phid.c_str(),phid.size()));
   f.setattrv("offlinefs.mediumid",Buffer(label.c_str(),label.size()));
}

void Medium_directory::delfile(File& f){
   if(unlink_files)
      unlink(realpath(f).c_str());
}


