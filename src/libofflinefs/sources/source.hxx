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

#ifndef SOURCES_SOURCE_HXX
#define SOURCES_SOURCE_HXX

#include <common.hxx>
#include <nodes.hxx>
#include "types.hxx"

// Class implementing the highest level file IO
class Source{
   protected:
      FsDb& dbs;
      uint64_t fileid;
      int mode;
   public:
      Source(File& f,int mode);
      virtual ~Source() {}

      uint64_t getfileid() {return fileid;}

      // Instantiate a Source-derived object for a File
      static std::auto_ptr<Source> getsource(File& f,int mode);
      // Free the underlying storage for a File
      static void remove(File& f);

      // Analogous to the standard unix calls
      virtual int read(char* buf, size_t nbyte, off_t offset)=0;
      virtual int write(const char* buf, size_t nbyte, off_t offset)=0;
      virtual int flush()=0;
      virtual int fsync(int datasync)=0;
      virtual int ftruncate(off_t length)=0;
};

#endif
