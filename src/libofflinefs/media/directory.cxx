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
#include "chunks/file.hxx"

using std::auto_ptr;
using std::string;
using std::list;

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

std::auto_ptr<Chunk> Medium_directory::getchunk(std::string phid,int mode){
   return std::auto_ptr<Chunk>(new Chunk_file(directory + "/" + phid,mode));
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

void Medium_directory::addfile(string phid){}

void Medium_directory::delfile(string phid){
   if(unlink_files)
      unlink((directory + "/" + phid).c_str());
}
