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

using std::string;

Source_single::Source_single(File& f,int mode):Source(f,mode){
   chunk = dbs.mcache.getmedium(f.getattrv("offlinefs.medium"))->getchunk(f.getattrv("offlinefs.phid"),mode);
}

void Source_single::remove(File& f){
   f.txns.dbs.mcache.getmedium(f.getattrv("offlinefs.medium"))->delfile(f.getattrv("offlinefs.phid"));
}

int Source_single::read(char* buf, size_t nbyte, off_t offset){
   return chunk->read(buf,nbyte,offset);
}

int Source_single::write(const char* buf, size_t nbyte, off_t offset){
   if(((off_t)nbyte+offset)>size)
      size=nbyte+offset;
   return chunk->write(buf,nbyte,offset);
}

int Source_single::flush(){
   FsTxn txns(dbs);
   File f(txns,fileid);
   if(size > f.getattr<off_t>("offlinefs.size"))
      f.setattr<off_t>("offlinefs.size",size);
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
   f.setattr<off_t>("offlinefs.size",length);
   size=length;

   return chunk->ftruncate(length);
}
