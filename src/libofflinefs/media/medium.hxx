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

#ifndef MEDIA_MEDIUM_HXX
#define MEDIA_MEDIUM_HXX

#include <libconfig.h++>

#include <common.hxx>
#include <fsnodes.hxx>
#include <fsdb.hxx>
#include <sources.hxx>

// Interface to an object implementing the file data operations
class Medium{
   public:
      // Filesystem statistics
      class Stats{
	 public:
	    Stats():blocks(0),freeblocks(0) {}
	    // Stored in multiples of 4096B
	    unsigned long blocks;
	    unsigned long freeblocks;
      };

      static std::auto_ptr<Medium> getmedium(libconfig::Setting& conf);
      virtual ~Medium() {}

      virtual std::auto_ptr<Source> getsource(File& f,int mode)=0;
      virtual int truncate(File& f,off_t length)=0;

      virtual Stats getstats()=0; 

      // Link file f with this medium, phid should be a string that will be used
      // to locate the file in the medium
      virtual void addfile(File& f,std::string phid)=0;
      virtual void delfile(File& f)=0;
};

#endif
