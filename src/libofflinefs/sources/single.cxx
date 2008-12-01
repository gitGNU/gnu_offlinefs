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

#include "single.hxx"
#include "media/medium.hxx"
#include "types.hxx"

using std::string;
namespace off = offlinefs;

Source_single::Source_single(File& f,int mode):Source(f,mode),atime(0),mtime(0){
   chunk = Medium::getmedium(f.txns,f.getattr<uint32_t>("offlinefs.mediumid"))->getchunk(f.getattrv("offlinefs.phid"),mode);
   size=f.getattr<off::off_t>("offlinefs.size");
}

void Source_single::remove(File& f){
   Medium::getmedium(f.txns, f.getattr<uint32_t>("offlinefs.mediumid"))->delfile(f.getattrv("offlinefs.phid"));
}

int Source_single::read(char* buf, size_t nbyte, off_t offset){
   atime=time(NULL);
   return chunk->read(buf,nbyte,offset);
}

int Source_single::write(const char* buf, size_t nbyte, off_t offset){
   mtime=time(NULL);

   if(((off::off_t)nbyte+offset)>size)
      size=nbyte+offset;

   return chunk->write(buf,nbyte,offset);
}

int Source_single::flush(){
   FsTxn txns(dbs);
   File f(txns,fileid);

   if(size > f.getattr<off::off_t>("offlinefs.size"))
      f.setattr<off::off_t>("offlinefs.size",size);

   if(atime > f.getattr<off::time_t>("offlinefs.atime"))
      f.setattr<off::off_t>("offlinefs.atime",atime);

   if(mtime > f.getattr<off::time_t>("offlinefs.mtime"))
      f.setattr<off::off_t>("offlinefs.mtime",mtime);

   return 0;
}

int Source_single::fsync(int datasync){
   if(!datasync)
      flush();

   return chunk->fsync(datasync);
}

int Source_single::ftruncate(off_t length){
   FsTxn txns(dbs);
   File f(txns,fileid);

   f.setattr<off::time_t>("offlinefs.mtime",mtime=time(NULL));
   f.setattr<off::off_t>("offlinefs.size",size=length);

   return chunk->ftruncate(length);
}
