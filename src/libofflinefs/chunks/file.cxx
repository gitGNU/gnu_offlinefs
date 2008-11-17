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

Chunk_file::Chunk_file(std::string path,int mode):fd(-1){
   if((mode&O_ACCMODE)==O_RDONLY){
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
		     throw std::runtime_error("Chunk_file::Chunk_file: Opening file \"" + path +
					      "\": error creating parent directories.");
	       }
	    }
	    pos=newpos+1;
	 }
	 fd=open(path.c_str(),mode|O_CREAT,0600);
      }
   }
   if(fd==-1)
      throw std::runtime_error(string("Chunk_file::Chunk_file: Error opening file \"")+ path +"\".");
}

Chunk_file::~Chunk_file(){
   if(fd!=-1)
      close(fd);
}

int Chunk_file::read(char* buf, size_t nbyte, off_t offset){
   return pread(fd,buf,nbyte,offset);
}

int Chunk_file::write(const char* buf, size_t nbyte, off_t offset){
   return pwrite(fd,buf,nbyte,offset);
}

int Chunk_file::flush(){
   return 0;
}

int Chunk_file::fsync(int datasync){
   if(datasync)
      return fdatasync(fd);
   else{
      return ::fsync(fd);
   }
}

int Chunk_file::ftruncate(off_t length){
   return ::ftruncate(fd,length);
}
