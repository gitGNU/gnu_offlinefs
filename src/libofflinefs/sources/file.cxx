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

#include "file.hxx"

using std::string;

Source_file::Source_file(File& f,std::string path,int mode):Source(f,mode),fd(-1){
   if(mode&O_ACCMODE==O_RDONLY){
      fd=open(path.c_str(),mode);
   }else{
      fd=open(path.c_str(),mode|O_CREAT,0660);
      if(fd==-1 && errno==ENOENT){
	 //Try to create every parent directory
	 string::size_type pos=0;
	 string::size_type newpos=0;
	 struct stat st;
	 while((newpos=path.find("/",pos))!=string::npos){
	    if(pos!=newpos){
	       string dir=path.substr(0,newpos);
	       if(stat(dir.c_str(),&st)){
		  if(errno!=ENOENT || mkdir(dir.c_str(),0770))
		     throw std::runtime_error("Source_file::Source_file: error creating parent directories.");
	       }
	    }
	    pos=newpos+1;
	 }
	 fd=open(path.c_str(),mode|O_CREAT,0600);
      }
   }
   if(fd==-1)
      throw std::runtime_error("Source_file::Source_file: error opening file.");
}

Source_file::~Source_file(){
   if(fd!=-1)
      close(fd);
}

int Source_file::read(char* buf, size_t nbyte, off_t offset){
   return pread(fd,buf,nbyte,offset);
}

int Source_file::write(const char* buf, size_t nbyte, off_t offset){
   if(((off_t)nbyte+offset)>size)
      size=nbyte+offset;
   return pwrite(fd,buf,nbyte,offset);
}

int Source_file::flush(){
   f.setattr<off_t>("offlinefs.size",size);
   return 0;
}

inline int real_fsync(int fd){
   return fsync(fd);
}

int Source_file::fsync(int datasync){
   if(datasync)
      return fdatasync(fd);
   else{
      f.setattr<off_t>("offlinefs.size",size);
      return real_fsync(fd);
   }
}
